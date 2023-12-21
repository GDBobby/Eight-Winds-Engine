#include "EWEngine/SoundEngine.h"

#include <filesystem>
#include <iostream>
#include <iterator>

#define EFFECTS_PATH "sounds/effects/"
#define MUSIC_PATH "sounds/music/"
#define VOICE_PATH "sounds/voice/"

#define WITH_NULL

void ma_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	(void)pInput;

	/*
	Since we're managing the underlying device ourselves, we need to read from the engine directly.
	To do this we need access to the ma_engine object which we passed in to the user data. One
	advantage of this is that you could do your own audio processing in addition to the engine's
	standard processing.
	*/
	ma_engine_read_pcm_frames((ma_engine*)pDevice->pUserData, pOutput, frameCount, NULL);
}

namespace EWE {
#if MINI_LIB
	SoundEngine::SoundEngine() {
		ma_result result;

		resourceManagerConfig = ma_resource_manager_config_init();
		resourceManagerConfig.decodedFormat     = ma_format_f32;
		resourceManagerConfig.decodedChannels = 0; // Setting the channel count to 0 will cause sounds to use their native channel count
		resourceManagerConfig.decodedSampleRate = 48000;// Using a consistent sample rate is useful for avoiding expensive resampling in the audio thread. This will result in resampling being performed by the loading thread(s).

		result = ma_resource_manager_init(&resourceManagerConfig, &resourceManager);
		if(result != MA_SUCCESS) {
			printf("Failed to initialize resource manager.\n");
			throw std::exception("failed to init miniaudio");
		}
		result = ma_context_init(NULL, 0, NULL, &context);
		if (result != MA_SUCCESS) {
			printf("Failed to initialize context.\n");
			throw std::exception("failed to init miniaudio");
		}
		result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, NULL, NULL);
		if (result != MA_SUCCESS) {
			printf("failed to get devices \n");
			throw std::exception("failed to init miniaudio");
		}


		engines.resize(playbackDeviceCount + 1);
		devices.resize(engines.size());
		deviceNames.reserve(engines.size());
		effects.resize(engines.size());
		music.resize(engines.size());
		voices.resize(engines.size());

		initEngines(pPlaybackDeviceInfos, playbackDeviceCount);

		//ma_sound_group_init(&engines.at(selectedEngine), 0, NULL, &effectGroup);
		//ma_sound_group_init(&engines.at(selectedEngine), 0, NULL, &musicGroup);
		//ma_sound_group_init(&engines.at(selectedEngine), 0, NULL, &voiceGroup);

		bool foundMatchingDevice = false;

		loadHowlingWind();

		initVolume();
		printf("end of soundengine constructor \n");
		//playMusic(0, false);

		//loadHowlingWind();
	}
	SoundEngine::~SoundEngine() {
		for (auto& effectEngine : effects) {
			for (auto iter2 = effectEngine.begin(); iter2 != effectEngine.end(); iter2++) {
				ma_sound_uninit(&iter2->second);
			}
		}

		for (auto& engine : engines) {
			ma_engine_uninit(&engine);
		}
		for (auto& device : devices) {
			ma_device_uninit(&device);
		}
		ma_context_uninit(&context);

		ma_resource_manager_uninit(&resourceManager);
	}

	void SoundEngine::switchDevices(uint16_t deviceIterator) {
		if ((deviceIterator < 0) || (deviceIterator > devices.size())) {
#ifdef _DEBUG
			printf("failed to switch devices - %d:%lld \n", deviceIterator, devices.size());
#endif
			return;
		}
		if (deviceIterator == selectedEngine) {
			printf("trying to switch sound device to the currently active \n");
			return;
		}



		printf("switching device - from:to - %d:%d \n", selectedEngine, deviceIterator);
		ma_result result = ma_engine_start(&engines[deviceIterator]);
		if (result != MA_SUCCESS) {
			printf("failed to start engine on switch : %d \n", deviceIterator);
		}
		else {
			ma_engine_stop(&engines[selectedEngine]);
			printf("before swapping sources \n");
			for (auto iter = effects.at(selectedEngine).begin(); iter != effects.at(selectedEngine).end(); iter++) {
				ma_sound_stop(&iter->second);
				ma_sound_uninit(&iter->second);
			}
			for (auto iter = music.at(selectedEngine).begin(); iter != music.at(selectedEngine).end(); iter++) {
				if (iter->first == currentSong) {
					ma_sound_get_cursor_in_pcm_frames(&iter->second, &currentPCMFrames);
					printf("pcm cursor : %llu \n", currentPCMFrames);
				}
				ma_sound_stop(&iter->second);
				ma_sound_uninit(&iter->second);
			}
			effects.at(selectedEngine).clear();
			music.at(selectedEngine).clear();
			selectedEngine = deviceIterator;
			printf("before reloading soudns \n");
			reloadSounds();
			printf("after reloading sounds \n");
		}
			
	}

	void SoundEngine::playMusic(uint16_t whichSong, bool repeat = false) {
		printf("starting music \n");

		currentSong = whichSong;
		ma_result result = ma_sound_start(&music.at(selectedEngine).at(whichSong));
		if (whichSong == 0) {
			ma_sound_set_looping(&music.at(selectedEngine).at(whichSong), true);
			printf("soudn looping : %d \n", ma_sound_is_looping(&music.at(selectedEngine).at(whichSong)));
		}

		if (result != MA_SUCCESS) {
			printf("WARNING: Failed to [load];ay sound \"%d\"", whichSong);
			return;
		}
	}
	void SoundEngine::playEffect(uint16_t whichEffect) {
		printf("starting sound \n");
		
		if(selectedEngine > effects.size()){
			printf("selected engine is out of range \n");
			return;
		}
		if(whichEffect > effects.at(selectedEngine).size()){
			printf("selected effect is out of range \n");
			return;
		}


		ma_result result = ma_sound_start(&effects.at(selectedEngine).at(whichEffect));
		if (result != MA_SUCCESS) {
			printf("WARNING: Failed to [load];ay sound \"%d\"", whichEffect);
			return;
		}
	}

	void SoundEngine::loadHowlingWind() {
		bin2cpp::File const* hWind = &bin2cpp::getHowlingWindFile();
		effects.at(selectedEngine).emplace(0, ma_sound{});
		
		ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, 2, 48000);
		decoderConfig.pCustomBackendUserData = NULL;
		decoderConfig.ppCustomBackendVTables = NULL;
		decoderConfig.customBackendCount = 0;
		decoderConfig.encodingFormat = ma_encoding_format_mp3;
		printf("finna init from memory \n");
		//ma_decoder_init();

		ma_result result = ma_decoder_init_memory(hWind->getBuffer(), hWind->getSize(), NULL, &hwDecoder);
		
		if (result != MA_SUCCESS) {
			printf("decoder init from memroy failed \n");
		}
		music.at(selectedEngine).emplace(1, ma_sound{});
		printf("finna init sound from data source \n");
		result = ma_sound_init_from_data_source(&engines[selectedEngine], &hwDecoder, MA_SOUND_FLAG_STREAM, NULL, &hwSound);

		if (result != MA_SUCCESS) {
			printf("init from data source failed : HOWLING WIND \n");

		}
		ma_sound_start(&hwSound);
	}
	void SoundEngine::initEngines(ma_device_info* deviceInfos, uint32_t deviceCount) {
		ma_engine_config engineConfig;
		engineConfig = ma_engine_config_init();
		engineConfig.pResourceManager = &resourceManager;
		engineConfig.noAutoStart = MA_TRUE;    /* Don't start the engine by default - we'll do that manually below. */

		bool foundDesiredDevice = false;
		for (uint32_t i = 0; i < deviceCount + 1; i++) {
			ma_device_config deviceConfig; //are these disposable or do i need these for the same lifetime as the device/engine?

			deviceConfig = ma_device_config_init(ma_device_type_playback);
			if (i == 0) {
				deviceConfig.playback.pDeviceID = NULL;
			}
			else {
				deviceConfig.playback.pDeviceID = &pPlaybackDeviceInfos[i - 1].id;
			}
			deviceConfig.playback.format = resourceManager.config.decodedFormat;
			deviceConfig.playback.channels = 0;
			deviceConfig.sampleRate = resourceManager.config.decodedSampleRate;
			deviceConfig.dataCallback = ma_data_callback;
			deviceConfig.pUserData = &engines[i];

			ma_result result = ma_device_init(&context, &deviceConfig, &devices[i]);
			if (result != MA_SUCCESS) {
				if (i == 0) {
					printf("failed to initialize the default device \n");
					throw std::exception("failed to init miniaudio device");
				}
				printf("Failed to initialize device for %s.\n", pPlaybackDeviceInfos[i - 1].name);
				throw std::exception("failed to init miniaudio device");
			}

			// Now that we have the device we can initialize the engine. The device is passed into the engine's config
			ma_engine_config engineConfig;
			engineConfig = ma_engine_config_init();
			engineConfig.pDevice = &devices[i];
			engineConfig.pResourceManager = &resourceManager;
			engineConfig.noAutoStart = MA_TRUE;    /* Don't start the engine by default - we'll do that manually below. */

			printf("device name - %d: %s\n", i, pPlaybackDeviceInfos[i].name);
			if (i == 0) {
				auto& deviceName = deviceNames.emplace_back("default");
				result = ma_engine_init(&engineConfig, &engines[0]);
				if (result != MA_SUCCESS) {
					printf("Failed to init engine for %s\n", deviceName.c_str());
					ma_device_uninit(&devices[0]);
					//throw std exception or just cancel the swap?
				}
				else if (SettingsJSON::settingsData.selectedDevice == deviceName) {
					foundDesiredDevice = true;
					result = ma_engine_start(&engines[0]);
					if (result != MA_SUCCESS) {
						printf("Failed to start engine for DEFAULT \n");
						ma_engine_uninit(&engines[0]);
						ma_device_uninit(&devices[0]);
						//throw std exception or just cancel the swap?
					}
					else {
						selectedEngine = 0;
					}
				}
			}
			else {

				auto& deviceName = deviceNames.emplace_back(pPlaybackDeviceInfos[i - 1].name);
				result = ma_engine_init(&engineConfig, &engines[i]);
				if (result != MA_SUCCESS) {
					printf("Failed to init engine for %s\n", deviceName.c_str());
					ma_device_uninit(&devices[0]);
					//throw std exception or just cancel the swap?
				}
				else if (deviceName.find(SettingsJSON::settingsData.selectedDevice) != deviceName.npos) {
					foundDesiredDevice = true;
					result = ma_engine_start(&engines[i]);
					if (result != MA_SUCCESS) {
						printf("Failed to start engine for %s\n", deviceName.c_str());
						ma_engine_uninit(&engines[i]);
						ma_device_uninit(&devices[i]);
						//throw std exception or just cancel the swap?
					}
					else {
						selectedEngine = i;
					}
				}
			}
		}
		if (!foundDesiredDevice) {
			if (engines.size() > 0) {
				ma_result result = ma_engine_init(&engineConfig, &engines[0]);
				if (result != MA_SUCCESS) {
					printf("Failed to init engine for DEFAULT \n");
					ma_device_uninit(&devices[0]);
					//throw std exception or just cancel the swap?
				}
				else {
					result = ma_engine_start(&engines[0]);
					if (result != MA_SUCCESS) {
						printf("Failed to start engine for DEFAULT \n");
						ma_engine_uninit(&engines[0]);
						ma_device_uninit(&devices[0]);
						//throw std exception or just cancel the swap?
					}
					else {
						selectedEngine = 0;
					}
				}
			}
		}

		printf("after init engines, selected device : %d \n", selectedEngine);
	}

	void SoundEngine::loadSoundMap(std::unordered_map<uint16_t, std::string>& loadSounds, SoundType soundType) {
		if (selectedEngine > engines.size()) {
			printf("selected engine isinvalid when loading effects \n");
			return;
		}

		std::unordered_map<uint16_t, std::string>* locations{nullptr};
		std::unordered_map<uint16_t, ma_sound>* sounds{nullptr};
		switch (soundType) {
		case SoundType::Effect: {
			locations = &effectLocations;
			sounds = &effects.at(selectedEngine);
			break;
		}
		case SoundType::Music: {
			locations = &musicLocations;
			sounds = &music.at(selectedEngine);
			break;
		}
		case SoundType::Voice: {
			locations = &voiceLocations;
			sounds = &voices.at(selectedEngine);
			break;
		}
		default: {
			printf("loading an unsupported soudn type? : %d \n", soundType);
			return;
			break;
		}
		}


		ma_result result;
		for (auto& sound : loadSounds) {
			if (locations->find(sound.first) == locations->end()) {
				locations->emplace(sound);
			}
			else {
				//updates the vlaue?? should probably just be ignored
				printf("overwriting a saved effect location : %d \n", sound.first);
				printf("overwritign wont take effect until the device is swapped. i could support this but not doing it currently \n");
				locations->at(sound.first) = sound.second;
			}
		}

		for (auto& soundPath : *locations) {
			if (sounds->find(soundPath.first) == sounds->end()) {
				sounds->emplace(soundPath.first, ma_sound{});
				switch (soundType) {
					case SoundType::Effect: {
						if (std::filesystem::exists(soundPath.second)) {
							result = ma_sound_init_from_file(&engines[selectedEngine], soundPath.second.c_str(),
								MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_STREAM,
								NULL, NULL, &sounds->at(soundPath.first));
							ma_sound_set_volume(&sounds->at(soundPath.first), static_cast<float>(volume[(uint8_t)SoundVolume::effect]) / 100.f);

							if (result != MA_SUCCESS) {
								printf("WARNING: Failed to load effect \"%s\"", soundPath.second.c_str());
								throw std::exception("failed to load sound");
							}
						}
						else {
							printf("effect path doesnt exist \n");
						}
						break;
					}
					case SoundType::Voice:
					case SoundType::Music: {
						result = ma_sound_init_from_file(&engines[selectedEngine], soundPath.second.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &sounds->at(soundPath.first));

						ma_sound_set_volume(&sounds->at(soundPath.first), static_cast<float>(volume[(uint8_t)SoundVolume::music]) / 100.f);
						if (result != MA_SUCCESS) {
							printf("WARNING: Failed to load music or voice \"%s\"", soundPath.second.c_str());
							throw std::exception("failed to load sound");
						}
					}
				}
			}
		}

	}
	void SoundEngine::reloadSounds() {
		ma_result result;
		//effects
		printf("before reloading effects \n");
		for (auto& effectPath : effectLocations) {
			effects.at(selectedEngine).emplace(effectPath.first, ma_sound{});
			result = ma_sound_init_from_file(&engines[selectedEngine], effectPath.second.c_str(),
				MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_STREAM,
				NULL, NULL, &effects.at(selectedEngine).at(effectPath.first));

			if (result != MA_SUCCESS) {
				printf("WARNING: Failed to load effect \"%s\"", effectPath.second.c_str());
				throw std::exception("failed to load sound");
			}
		}

		printf("before reloading music \n");
		for (auto& musicPath : musicLocations) {
			music.at(selectedEngine).emplace(musicPath.first, ma_sound{});
			printf("imemdiately befroe : %s \n", musicPath.second.c_str());
			result = ma_sound_init_from_file(&engines[selectedEngine], musicPath.second.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &music.at(selectedEngine).at(musicPath.first));
			if (currentSong == musicPath.first) {
				ma_sound_seek_to_pcm_frame(&music.at(selectedEngine).at(musicPath.first), currentPCMFrames);
				if (!ma_sound_at_end(&music.at(selectedEngine).at(musicPath.first))) {
					ma_sound_set_volume(&music.at(selectedEngine).at(musicPath.first), volume[(uint8_t)SoundVolume::music]);
					ma_sound_start(&music.at(selectedEngine).at(musicPath.first));
				}
			}

			if (result != MA_SUCCESS) {
				printf("WARNING: Failed to load music \"%s\"", musicPath.second.c_str());
				throw std::exception("failed to load sound");
			}
		}
		initVolume();
		printf("after reloading music \n");
	}
	void SoundEngine::setVolume(SoundVolume whichVolume, uint8_t value) {
		float volume = static_cast<float>(value) / 100.f;

		switch (whichVolume) {
		case SoundVolume::master: {
			for (auto& engine : engines) {
				ma_engine_set_volume(&engine, volume);
			}
			break;
		}
		case SoundVolume::effect: {
			for (auto& sound : effects.at(selectedEngine)) {
				ma_sound_set_volume(&sound.second, volume);
			}
			break;
		}
		case SoundVolume::music: {
			for (auto& sound : music.at(selectedEngine)) {
				ma_sound_set_volume(&sound.second, volume);
			}
			ma_sound_set_volume(&hwSound, volume);
			break;
		}
		case SoundVolume::voice: {
			for (auto& sound : voices.at(selectedEngine)) {
				ma_sound_set_volume(&sound.second, volume);
			}
			break;
		}
		}
	}


#elif IRRK_LIB
	SoundEngine::SoundEngine() {
		
		engine = irrklang::createIrrKlangDevice();
		engine->setSoundVolume(volume[SoundVolume::master]);

		deviceList = irrklang::createSoundDeviceList();
		deviceNames.resize(deviceList->getDeviceCount());

		stopEventReceiver = new StopEventReceiver(this);

		//fstream.

		for (int i = 0; i < deviceNames.size(); i++) {
			deviceNames[i] = deviceList->getDeviceDescription(i);
			//printf("device name at %d : %s \n", i, deviceNames[i].c_str());
		}
		loadEffectsFromFile();
		for (const auto& entry : std::filesystem::directory_iterator(MUSIC_PATH)) {
			
			mapMusicFile(entry);


			//musicMap[]
			/*
			music.push_back(engine->addSoundSourceFromFile(entry.path().string().c_str()));
			if (music.back() == 0) {
				printf("failed to load music file : %s \n", entry.path().string().c_str());
				music.pop_back();
			}
			else {
				//printf("music volume ? : %.2f \n", volume[SoundVolume::music]);
				music.back()->setDefaultVolume(volume[SoundVolume::music]);
			}
			*/
		}

		//voice is not currently utilized, feels like a waste? last tiem i added voice i just used sound effects
		for (const auto& entry : std::filesystem::directory_iterator(VOICE_PATH)) {
			voice.push_back(engine->addSoundSourceFromFile(entry.path().string().c_str()));
			if (voice.back() == 0) {
				printf("failed to load music file : %s \n", entry.path().string().c_str());
				voice.pop_back();
			}
			else {
				voice.back()->setDefaultVolume(volume[SoundVolume::voice]);
			}
		}
		//engine->play2D(effects[0], true);
		//printf("where this mfer at \n");

		initVolume();
		playMusic(Music_howlingWind, false);
	}

	SoundEngine::~SoundEngine() {
		engine->drop();
		delete stopEventReceiver;
	}

	void SoundEngine::loadEffectsFromFile() {
		for (const auto& entry : std::filesystem::directory_iterator(EFFECTS_PATH)) {
			effects.push_back(engine->addSoundSourceFromFile(entry.path().string().c_str()));
			effects.back()->setDefaultVolume(volume[SoundVolume::effect]);
			if (effects.back() == 0) {
				printf("failed to load effect file : %s \n", entry.path().string().c_str());
				effects.pop_back();
			}
		}
		//printf("effect size %d \n", effects.size());
	}


	void SoundEngine::playMusic(uint8_t whichSong, bool repeat) {
		
		if (((volume[SoundVolume::master] == 0) || (volume[SoundVolume::music] == 0))) { return; }
		if (musicMap.find((Music_Enum)whichSong) == musicMap.end()) {
			return;
		}
		printf("playing song? \n");
		for (auto iter = musicMap.begin(); iter != musicMap.end(); iter++) {
			//engine->stopAllSoundsOfSoundSource(iter->second);
			if (currentSongSound != nullptr) {
				currentSongSound->stop();
				currentSongSound->drop();
				currentSongSound = nullptr;
			}
		}

		songIsPlaying = true;
		currentSong = whichSong;
		//printf("play song %d \n", whichSong);
		printf("finna play song \n");
		currentSongSound = engine->play2D(musicMap[(Music_Enum)whichSong], repeat, true); //it needs to start paused, dog ass irrklang. REPLACE IRRKLANG ASAP
		currentSongSound->setIsPaused(false);
		// i need to do some irrklang shit and the documentation is god awful
		if (currentSongSound == nullptr) {
			printf("why is current song sound nullptr immediately aftr playing a song? \n");
		}
		else {
			currentSongSound->grab();
			//this is broken
			//printf("current song NOT nullptr \n");
			if (!repeat) {
				currentSongSound->setSoundStopEventReceiver(stopEventReceiver);
			}
		}
		
		currentSong = whichSong;
		printf("after play music \n");
	}
	void SoundEngine::stopMusic() {
		engine->stopAllSounds();
	}
	void SoundEngine::playEffect(SFX_Enum whichEffect) {
		
		if (((volume[SoundVolume::master] == 0) || (volume[SoundVolume::effect] == 0)) || (whichEffect >= effects.size())) { printf("sound effect failure \n"); return; }

		//printf("play effect %d \n", whichEffect);
		engine->play2D(effects[whichEffect]);
	}

	void SoundEngine::playVoice(uint8_t whichVoice) {
		//if ((currentDevice == 69420) || ((volume[SoundVolume::master] == 0) || (volume[SoundVolume::voice] == 0)) || (whichVoice > voice.size())) { return; }

	}

	void SoundEngine::switchDevices(int deviceIterator) {
		//printf("device switch\n");
		if (!currentSongSound) {
			//printf("cuyrrent song sound is nullptr \n");
		}
		if (currentSongSound && songIsPlaying) {
			//printf("song was playing \n");
			currentSongSound->setIsPaused();
			irrklang::ik_u32 songPosition = currentSongSound->getPlayPosition();
			engine->stopAllSounds();
			//engine->drop();
			engine = irrklang::createIrrKlangDevice(irrklang::ESOD_AUTO_DETECT, irrklang::ESEO_DEFAULT_OPTIONS, deviceList->getDeviceID(deviceIterator));

			currentSongSound = engine->play2D(musicMap[(Music_Enum)currentSong], false, true);
			currentSongSound->setPlayPosition(songPosition);
			currentSongSound->setIsPaused(false);
		}
		else {
			engine->stopAllSounds();
			//engine->drop();
			engine = irrklang::createIrrKlangDevice(irrklang::ESOD_AUTO_DETECT, irrklang::ESEO_DEFAULT_OPTIONS, deviceList->getDeviceID(deviceIterator));
		}
		
		for (int i = 0; i < effects.size(); i++) {
			effects[i]->drop();
			/*
			//this doesn't work, going to have to reload from file i guess
			void* data = effects[i]->getSampleData();
			irrklang::ik_s32 tempSize = effects[i]->getAudioFormat().getSampleDataSize();
			effects[i] = engine->addSoundSourceFromMemory(data, tempSize, "");
			*/
		}
		effects.clear();
		loadEffectsFromFile();
		deviceWasSwitched = true;
	}
	void SoundEngine::setVolume(int8_t whichVolume, uint8_t value) {
		//ma_device_set_SoundVolume::master(&device, volume[whichVolume]);
		//printf("setting volume %d : %.2f \n", whichVolume, value);
		float thisVolume = static_cast<float>(value) / 100.f;
		volume[whichVolume] = thisVolume;
		if (whichVolume == SoundVolume::master) {
			engine->setSoundVolume(thisVolume);
		}
		else if (whichVolume == SoundVolume::effect) {
			for (int i = 0; i < effects.size(); i++) {
				effects[i]->setDefaultVolume(thisVolume);
			}
		}
		else if (whichVolume == SoundVolume::music) {
			for (auto iter = musicMap.begin(); iter != musicMap.end(); iter++) {
				iter->second->setDefaultVolume(thisVolume);
				if (currentSongSound != nullptr) {
					currentSongSound->setVolume(thisVolume);
				}
				else {
					playMusic(Music_Menu, true);
				}
			}
		}
		else if (whichVolume == SoundVolume::voice) {
			for (int i = 0; i < voice.size(); i++) {
				voice[i]->setDefaultVolume(thisVolume);
			}
		}
	}
	void SoundEngine::playNextSong(){
		if (((volume[SoundVolume::master] == 0) || (volume[SoundVolume::music] == 0)) || (musicMap.size() == 0) || (currentSongSound == nullptr)) { return; }
		printf("playing next song \n");
		currentSongSound->setIsPaused(true);
		currentSongSound->setPlayPosition(0);
		//currentSongSound->stop();
		//currentSongSound->drop();
		switch (currentSong) {
			case Music_Devour: {
				playMusic(Music_Phase1);
				break;
			}
			case Music_Phase1: {
				playMusic(Music_Ritual);
				break;
			}
			case Music_Ritual: {
				playMusic(Music_Devour);
				break;
			}
			case Music_Menu: {
				playMusic(Music_BridgeM);
				break;
			}
			case Music_BridgeM: {
				playMusic(Music_Menu);
				break;
			}
			default: {
				printf("why default? \n");
				break;
			}
		}
		
		//currentSong = (currentSong + 1) % Music_invalid;
		//while (musicMap.find((Music_Enum)currentSong) == musicMap.end()) {
		//	currentSong = (currentSong + 1) % Music_invalid;
		//}
		
		printf("play next song %d \n", currentSong);
		
		//currentSongSound = engine->play2D(musicMap[(Music_Enum)currentSong], false, true);
		//currentSongSound->setIsPaused(false);
	
	}



	void SoundEngine::mapMusicFile(const std::filesystem::directory_entry& entry) {
		//printf("music string : %s \n", entry.path().string().c_str());
		std::string musicString = entry.path().string();
		musicString = musicString.substr(musicString.find_last_of('/') + 1, musicString.find_first_of('.') - musicString.find_last_of('/') - 1);
		//printf("musicString : %s \n", musicString.c_str());
		
		Music_Enum musicEnum = Music_invalid;
		
		if (musicString == "Bridge_1") {
			musicEnum = Music_Bridge1;
			
		}
		else if (musicString == "Bridge_1_Muffled") {
			musicEnum = Music_BridgeM;
		}
		else if (musicString == "Devour_Title") {
			musicEnum = Music_Devour;
		}
		
		else if (musicString == "howlingWind") {
			musicEnum = Music_howlingWind;
		}
		
		else if (musicString == "Menu_Title_1") {
			musicEnum = Music_Menu;
		}
		else if (musicString == "Phase_1_Sequence") {
			musicEnum = Music_Phase1;
		}
		else if (musicString == "Ritual_Sequence_3") {
			musicEnum = Music_Ritual;
		}
		else if (musicString == "Temple_Sequence_2") {
			musicEnum = Music_Temple;
		}
		else if (musicString == "BlinkIfYouDare") {
			musicEnum = Music_blink;
		}
		

		if (musicEnum == Music_invalid) {
			printf("invalid music file? : %s \n", musicString.c_str());
			return;
		}
		musicMap[musicEnum] = engine->addSoundSourceFromFile(entry.path().string().c_str());
		if (musicMap[musicEnum] == 0) {
			printf("failed to load music file : %s \n", entry.path().string().c_str());
			musicMap.erase(musicEnum);
		}
		else {
			//printf("music volume ? : %.2f \n", volume[SoundVolume::music]);
			musicMap[musicEnum]->setDefaultVolume(volume[SoundVolume::music]);
		}
	}
#endif


}