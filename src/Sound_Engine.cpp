#include "EWEngine/Sound_Engine.h"

#include <filesystem>

#define EFFECTS_PATH "sounds/effects/"
#define MUSIC_PATH "sounds/music/"
#define VOICE_PATH "sounds/voice/"

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
	SoundEngine::SoundEngine() {
		ma_result result;

		resourceManagerConfig = ma_resource_manager_config_init();
		resourceManagerConfig.decodedFormat     = ma_format_f32;
		resourceManagerConfig.decodedChannels = 0; // Setting the channel count to 0 will cause sounds to use their native channel count
		resourceManagerConfig.decodedSampleRate = 48000;// Using a consistent sample rate is useful for avoiding expensive resampling in the audio thread. This will result in resampling being performed by the loading thread(s).

		result = ma_resource_manager_init(&resourceManagerConfig, &resourceManager);
		assert(result == MA_SUCCESS && "failed to initialize resource manager");
		result = ma_context_init(NULL, 0, NULL, &context);
		assert(result == MA_SUCCESS && "failed to initialize context");
		result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, NULL, NULL);
		assert(result == MA_SUCCESS && "failed to get devices");



		engines.resize(playbackDeviceCount + 1);
		devices.resize(engines.size());
		deviceNames.reserve(engines.size());

		//deviceNames.emplace_back("default");
		//for(uint32_t i = 0; i < deviceCount; i++){
		//	printf("device name[%d] : %s\n", i, deviceNames.emplace_back(pPlaybackDeviceInfos[i].name).c_str());
		//}

		effects.resize(engines.size());
		music.resize(engines.size());
		voices.resize(engines.size());

		InitEngines(pPlaybackDeviceInfos, playbackDeviceCount);

		//ma_sound_group_init(&engines.at(selectedEngine), 0, NULL, &effectGroup);
		//ma_sound_group_init(&engines.at(selectedEngine), 0, NULL, &musicGroup);
		//ma_sound_group_init(&engines.at(selectedEngine), 0, NULL, &voiceGroup);

		LoadHowlingWind();
		initVolume();
		if ((volumes[(uint8_t)SoundVolume::master] > 0.f) && (volumes[(uint8_t)SoundVolume::music] > 0.f)) {
			currentSong = 65534;
			ma_sound_start(&hwSound);
		}

		//printf("end of soundengine constructor \n");
		//playMusic(0, false);
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

	void SoundEngine::SwitchDevices(uint16_t deviceIterator) {
		if ((deviceIterator < 0) || (deviceIterator > devices.size())) {
#if EWE_DEBUG
			printf("failed to switch devices - %d:%zu \n", deviceIterator, devices.size());
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
			ReloadSounds();
		}
			
	}

	void SoundEngine::PlayMusic(uint16_t whichSong, bool repeat) {
		//printf("starting music \n");

		currentSong = whichSong;
		//ma_result result = ma_sound_start(&music.at(selectedEngine).at(whichSong));
		ma_sound_set_looping(&music.at(selectedEngine).at(whichSong), repeat);
		//printf("soudn looping : %d \n", ma_sound_is_looping(&music.at(selectedEngine).at(whichSong)));

		ma_result result = ma_sound_start(&music.at(selectedEngine).at(whichSong));

		if (result != MA_SUCCESS) {
			printf("WARNING: Failed to play music %d", whichSong);
			return;
		}
	}
	void SoundEngine::StopMusic() {
		printf("stop the music pls \n");
		if (currentSong == 65534) {
			ma_sound_stop(&hwSound);
			return;
		}


		if (music.at(selectedEngine).find(currentSong) != music.at(selectedEngine).end()) {
			ma_sound_stop(&music.at(selectedEngine).at(currentSong));
		}
		else {
			printf("attempting to stop music, failed to find it \n");
		}
	}
	void SoundEngine::PlayEffect(uint16_t whichEffect, bool looping) {
#if EWE_DEBUG
		//printf("starting sound \n");
#endif

 		if(selectedEngine > effects.size()){
			printf("selected engine is out of range \n");
			return;
		}
		if(whichEffect > effects.at(selectedEngine).size()){
			printf("selected effect is out of range \n");
			return;
		}

		if ((volumes[(uint8_t)SoundVolume::master] <= 0.f) && (volumes[(uint8_t)SoundVolume::effect] <= 0.f)) {
			printf("volume is 0\n");
			return;
		}

#if EWE_DEBUG
		//printf("selectedEngine : %d:%.2f - volume of sound : %.2f \n", selectedEngine, ma_engine_get_volume(&engines.at(selectedEngine)), ma_sound_get_volume(&effects.at(selectedEngine).at(whichEffect)));
#endif
		//ma_sound
		ma_result result = ma_sound_start(&effects.at(selectedEngine).at(whichEffect));
		ma_sound_set_looping(&effects.at(selectedEngine).at(whichEffect), looping);
		if (result != MA_SUCCESS) {
			printf("WARNING: Failed to start sound \"%d\"", whichEffect);
			return;
		}
	}
	void SoundEngine::StopEffect(uint16_t whichEffect) {
		printf("stop effect : %d\n", ma_sound_stop(&effects.at(selectedEngine).at(whichEffect)));
		ma_sound_seek_to_pcm_frame(&effects.at(selectedEngine).at(whichEffect), 0);
	}

	void SoundEngine::RestartEffect(uint16_t whichEffect, bool looping)
	{
		ma_sound* soundAddr = &effects.at(selectedEngine).at(whichEffect);
		ma_sound_stop(soundAddr);
		ma_sound_seek_to_pcm_frame(soundAddr, 0);
		ma_result result = ma_sound_start(soundAddr);
		ma_sound_set_looping(soundAddr, looping);
		if (result != MA_SUCCESS) {
			printf("WARNING: Failed to start sound \"%d\"", whichEffect);
			return;
		}
	}

	void SoundEngine::LoadHowlingWind() {
		bin2cpp::File const* hWind = &bin2cpp::getHowlingWindFile();
		
		ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, 2, 48000);
		decoderConfig.pCustomBackendUserData = NULL;
		decoderConfig.ppCustomBackendVTables = NULL;
		decoderConfig.customBackendCount = 0;
		decoderConfig.encodingFormat = ma_encoding_format_mp3;
		//printf("finna init from memory \n");
		//ma_decoder_init();

		ma_result result = ma_decoder_init_memory(hWind->getBuffer(), hWind->getSize(), NULL, &hwDecoder);
		
		if (result != MA_SUCCESS) {
			printf("decoder init from memroy failed \n");
		}
		//printf("finna init sound from data source \n");
		result = ma_sound_init_from_data_source(&engines[selectedEngine], &hwDecoder, MA_SOUND_FLAG_STREAM, NULL, &hwSound);

		if (result != MA_SUCCESS) {
			printf("init from data source failed : HOWLING WIND \n");

		}
	}
	void SoundEngine::InitEngines(ma_device_info* deviceInfos, uint32_t deviceCount) {

		//printf("before ma_engine_config_init\n");
		ma_engine_config engineConfig = ma_engine_config_init();
		//printf("after ma_engine_config_init\n");
		engineConfig.pResourceManager = &resourceManager;
		engineConfig.noAutoStart = MA_TRUE;    /* Don't start the engine by default - we'll do that manually below. */

		bool foundDesiredDevice = false;
		deviceNames.emplace_back("default");
		for(uint32_t i = 0; i < deviceCount; i++){
#if EWE_DEBUG
			printf("device name[%d] : %s\n", i, pPlaybackDeviceInfos[i].name);
#endif
			deviceNames.emplace_back(pPlaybackDeviceInfos[i].name);
		}

		for (uint32_t i = 0; i < deviceCount + 1; i++) {
			ma_device_config deviceConfig;

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
					//throw std::runtime_error("failed to init miniaudio device");
					return;

				}
				else{
					printf("Failed to initialize device for %s.\n", pPlaybackDeviceInfos[i - 1].name);
					throw std::runtime_error("failed to init miniaudio device");
				}
			}

			auto& deviceName = deviceNames[i];
			if (i == 0) {
				// Now that we have the device we can initialize the engine. The device is passed into the engine's config
				ma_engine_config engineConfig;
				engineConfig = ma_engine_config_init();
				engineConfig.pDevice = &devices[i];
				engineConfig.pResourceManager = &resourceManager;
				engineConfig.noAutoStart = MA_TRUE;    /* Don't start the engine by default - we'll do that manually below. */
			
				result = ma_engine_init(&engineConfig, &engines[0]);
				if (result != MA_SUCCESS) {
					printf("Failed to init engine for %s\n", deviceName.c_str());
					ma_device_uninit(&devices[0]);
					//throw std exception or just cancel the swap?
				}
				else if (SettingsJSON::settingsData.selectedDevice == deviceName) {
					foundDesiredDevice = true;
#if EWE_DEBUG
					printf("starting default device, matched with settings \n");
#endif
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
				// Now that we have the device we can initialize the engine. The device is passed into the engine's config
				ma_engine_config engineConfig;
				engineConfig = ma_engine_config_init();
				engineConfig.pDevice = &devices[i];
				engineConfig.pResourceManager = &resourceManager;
				engineConfig.noAutoStart = MA_TRUE;    /* Don't start the engine by default - we'll do that manually below. */

				result = ma_engine_init(&engineConfig, &engines[i]);
				if (result != MA_SUCCESS) {
					printf("Failed to init engine for %s\n", deviceName.c_str());
					ma_device_uninit(&devices[0]);
					//throw std exception or just cancel the swap?
				}
				else if (deviceName.find(SettingsJSON::settingsData.selectedDevice) != deviceName.npos) {
					foundDesiredDevice = true;
					result = ma_engine_start(&engines[i]);
					printf("starting device from settings : %s \n", deviceName.c_str());
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
#if EWE_DEBUG
			printf("device name - %d: %s\n", i, deviceNames[i].c_str());
#endif
		}
		if (!foundDesiredDevice) {
			printf("failed to find desired device in settings, starting default \n");
			if (engines.size() > 0) {
				ma_result result = ma_engine_init(&engineConfig, &engines[0]);
				if (result != MA_SUCCESS) {
					printf("Failed to init engine for DEFAULT \n");
					ma_device_uninit(&devices[0]);
					//throw std exception or just cancel the swap?
				}
				else {
					printf("starting default engine \n");
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
#if EWE_DEBUG
		printf("after init engines, selected device : %d \n", selectedEngine);
#endif
	}

	void SoundEngine::LoadSoundMap(std::unordered_map<uint16_t, std::string>& loadSounds, SoundType soundType) {
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
			printf("loading an unsupported soudn type? : %d \n", (int)soundType);
			return;
			break;
		}
		}


		ma_result result;
		for (auto& sound : loadSounds) {
			if (locations->find(sound.first) == locations->end()) {
				if (!std::filesystem::exists(sound.second))
				{
					printf("failed to find sound file location : %s\n", sound.second.c_str());
				}
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
							ma_sound_set_volume(&sounds->at(soundPath.first), volumes[(uint8_t)SoundVolume::effect]);

							if (result != MA_SUCCESS) {
								printf("WARNING: Failed to load effect \"%s\"", soundPath.second.c_str());
								throw std::runtime_error("failed to load sound");
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

						ma_sound_set_volume(&sounds->at(soundPath.first), volumes[(uint8_t)SoundVolume::music]);
						if (result != MA_SUCCESS) {
							printf("WARNING: Failed to load music or voice \"%s\"", soundPath.second.c_str());
							throw std::runtime_error("failed to load sound");
						}
						break;
					}
				}
			}
			else {
				printf("trying to emplace a sound into a map key that already has a sounnd. this is being ignored - %d:%d \n", soundPath.first, (int)soundType);
			}
		}

	}

	int16_t SoundEngine::AddMusicToBack(std::string const& musicLocation)
	{
		for (auto& loc : musicLocations)
		{
			if (loc.second == musicLocation)
			{
				return loc.first;
			}
		}
		int16_t ret = musicLocations.size();
		musicLocations.try_emplace(ret, musicLocation);

		ma_result result;
		music[selectedEngine].emplace(ret, ma_sound{});
		result = ma_sound_init_from_file(&engines[selectedEngine], musicLocation.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &music[selectedEngine].at(ret));

		ma_sound_set_volume(&music[selectedEngine].at(ret), volumes[(uint8_t)SoundVolume::music]);
		if (result != MA_SUCCESS) {
			printf("WARNING: Failed to load music or voice \"%s\"", musicLocation.c_str());
			throw std::runtime_error("failed to load sound");
			return -1;
		}
		printf("successfully added music to back\n");
		return ret;
	}


	void SoundEngine::ReloadSounds() {
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
				throw std::runtime_error("failed to load sound");
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
					ma_sound_set_volume(&music.at(selectedEngine).at(musicPath.first), volumes[(uint8_t)SoundVolume::music]);
					ma_sound_start(&music.at(selectedEngine).at(musicPath.first));
				}
			}

			if (result != MA_SUCCESS) {
				printf("WARNING: Failed to load music \"%s\"", musicPath.second.c_str());
				throw std::runtime_error("failed to load sound");
			}
		}
		initVolume();
		printf("after reloading music \n");
	}
	void SoundEngine::SetVolume(SoundVolume whichVolume, uint8_t value) {
		auto& volume = this->volumes[(uint8_t)whichVolume];
		volume = static_cast<float>(value) / 100.f;

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
}