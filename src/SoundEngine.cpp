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
		engines.resize(playbackDeviceCount);
		devices.resize(playbackDeviceCount);
		deviceNames.reserve(playbackDeviceCount);

		for (uint16_t i = 0; i < playbackDeviceCount; i++) { //64 is arbitrary max device count
			printf("device name - %d: %s\n", i, pPlaybackDeviceInfos[i].name);
			deviceNames.emplace_back(pPlaybackDeviceInfos[i].name);
		}

		//result = ma_engine_init(NULL, &engine);
		//if (result != MA_SUCCESS) {
		//	printf("failed to init miniaudio engine \n");
		//	throw std::exception("failed to init miniaudio");
		//}
		//ma_engine_play_sound(&engine, "C:\\Projects\\EightWinds\\sounds\\effects\\boom.mp3", NULL);
	}
	SoundEngine::~SoundEngine() {
		for (auto iter = effects.begin(); iter != effects.end(); iter++) {
			ma_sound_uninit(&iter->second);
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

	void SoundEngine::switchDevices(int deviceIterator) {
		if (!deviceInit) {
			ma_device_config deviceConfig; //are these disposable or do i need these for the same lifetime as the device/engine?

			deviceConfig = ma_device_config_init(ma_device_type_playback);
			deviceConfig.playback.pDeviceID = &pPlaybackDeviceInfos[deviceIterator].id;
			deviceConfig.playback.format = resourceManager.config.decodedFormat;
			deviceConfig.playback.channels = 0;
			deviceConfig.sampleRate = resourceManager.config.decodedSampleRate;
			deviceConfig.dataCallback = ma_data_callback;
			deviceConfig.pUserData = &engines[deviceIterator];// &engine[engineCount]; //engineCount is 0 at the point in time its used in the example

			ma_result result = ma_device_init(&context, &deviceConfig, &devices[deviceIterator]);
			if (result != MA_SUCCESS) {
				printf("Failed to initialize device for %s.\n", pPlaybackDeviceInfos[deviceIterator].name);
				throw std::exception("failed to init miniaudio device");
			}

			// Now that we have the device we can initialize the engine. The device is passed into the engine's config
			ma_engine_config engineConfig;
			engineConfig = ma_engine_config_init();
			engineConfig.pDevice = &devices[deviceIterator];
			engineConfig.pResourceManager = &resourceManager;
			engineConfig.noAutoStart = MA_TRUE;    /* Don't start the engine by default - we'll do that manually below. */

			result = ma_engine_init(&engineConfig, &engines[deviceIterator]);
			if (result != MA_SUCCESS) {
				printf("Failed to init engine for %s\n", pPlaybackDeviceInfos[deviceIterator].name);
				ma_device_uninit(&devices[deviceIterator]);
				//throw std exception or just cancel the swap?
			}
			else {
				if (selectedEngine != nullptr) {
					ma_engine_stop(selectedEngine);
				}
				selectedEngine = &engines[deviceIterator];
				ma_engine_start(selectedEngine);
				
			}
			
		}
	}
	void SoundEngine::loadEffects(std::unordered_map<uint16_t, std::string>& loadEffects) {
		//this needs to be done after an engine is loaded? i guess

		ma_result result;
		if (selectedEngine == nullptr) {
			throw std::exception("loading effects without an engine");
		}

		for (auto iter = loadEffects.begin(); iter != loadEffects.end(); iter++) {
			result = ma_sound_init_from_file(selectedEngine, iter->second.c_str(), MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_STREAM, NULL, NULL, &effects[iter->first]);
			if (result != MA_SUCCESS) {
				printf("WARNING: Failed to load sound \"%s\"", iter->second.c_str());
				throw std::exception("failed to load sound");
			}
		}
		result = ma_sound_init_from_file(selectedEngine, "sounds\\effects\\click.mp3", MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_STREAM, NULL, NULL, &clickEffect);

		if (result != MA_SUCCESS) {
			printf("probably change the click file location \n");
			throw std::exception("failed to load sound");
		}

	}
	void SoundEngine::playEffect(uint8_t whichEffect) {
		if (selectedEngine == nullptr) {
			return;
		}
		ma_result result = ma_sound_start(&effects.at(whichEffect));

		if (result != MA_SUCCESS) {
			printf("WARNING: Failed to [load];ay sound \"%d\"", whichEffect);
			return;
		}
	}
	void SoundEngine::playClickEffect() {
		if (selectedEngine == nullptr) {
			return;
		}
		ma_result result = ma_sound_start(&clickEffect);
		if (result != MA_SUCCESS) {
			printf("failed to play click effect \n");
			return;
		}
	}


#elif IRRK_LIB
	SoundEngine::SoundEngine() {
		
		engine = irrklang::createIrrKlangDevice();
		engine->setSoundVolume(volume[master_volume]);

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
				//printf("music volume ? : %.2f \n", volume[music_volume]);
				music.back()->setDefaultVolume(volume[music_volume]);
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
				voice.back()->setDefaultVolume(volume[voice_volume]);
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
			effects.back()->setDefaultVolume(volume[effect_volume]);
			if (effects.back() == 0) {
				printf("failed to load effect file : %s \n", entry.path().string().c_str());
				effects.pop_back();
			}
		}
		//printf("effect size %d \n", effects.size());
	}


	void SoundEngine::playMusic(uint8_t whichSong, bool repeat) {
		
		if (((volume[master_volume] == 0) || (volume[music_volume] == 0))) { return; }
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
		
		if (((volume[master_volume] == 0) || (volume[effect_volume] == 0)) || (whichEffect >= effects.size())) { printf("sound effect failure \n"); return; }

		//printf("play effect %d \n", whichEffect);
		engine->play2D(effects[whichEffect]);
	}

	void SoundEngine::playVoice(uint8_t whichVoice) {
		//if ((currentDevice == 69420) || ((volume[master_volume] == 0) || (volume[voice_volume] == 0)) || (whichVoice > voice.size())) { return; }

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
		//ma_device_set_master_volume(&device, volume[whichVolume]);
		//printf("setting volume %d : %.2f \n", whichVolume, value);
		float thisVolume = static_cast<float>(value) / 100.f;
		volume[whichVolume] = thisVolume;
		if (whichVolume == master_volume) {
			engine->setSoundVolume(thisVolume);
		}
		else if (whichVolume == effect_volume) {
			for (int i = 0; i < effects.size(); i++) {
				effects[i]->setDefaultVolume(thisVolume);
			}
		}
		else if (whichVolume == music_volume) {
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
		else if (whichVolume == voice_volume) {
			for (int i = 0; i < voice.size(); i++) {
				voice[i]->setDefaultVolume(thisVolume);
			}
		}
	}
	void SoundEngine::playNextSong(){
		if (((volume[master_volume] == 0) || (volume[music_volume] == 0)) || (musicMap.size() == 0) || (currentSongSound == nullptr)) { return; }
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
			//printf("music volume ? : %.2f \n", volume[music_volume]);
			musicMap[musicEnum]->setDefaultVolume(volume[music_volume]);
		}
	}
#endif
}