#pragma once

#include "EWEngine/Graphics/Preprocessor.h"
#include "SettingsJson.h"
#include "EWEngine/resources/howlingWind.h"


#define MINIAUDIO_IMPLEMENTATION
#if defined(MA_WIN32) //miniaudio brings in <windows> (clenches fist)
#define NOMINMAX
#endif

// https://miniaud.io/docs/examples/data_source_chaining.html
//for making the end of a song play the next, seems straightforward
//i would imagine this is less than 15min to implement, i just dont have multiple sources to loop currently
#include "miniaudio/miniaudio.h"

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <map>
#include <array>
#include <unordered_map>

namespace EWE {
	class SoundEngine {
	private:
		SoundEngine();
		//~RigidRenderingSystem() = default;
		SoundEngine(const SoundEngine&) = delete;
		SoundEngine& operator=(const SoundEngine&) = delete;
	public:
		enum class SoundType : uint16_t {
			Effect,
			Music,
			Voice,
		};

		//need to make this a singleton, and pass in the default device on construction


		static std::shared_ptr<SoundEngine> GetSoundEngineInstance() {
			static std::shared_ptr<SoundEngine> soundEngineInstance{ new SoundEngine() };
			return soundEngineInstance;
		}

		~SoundEngine();


		void PlayMusic(uint16_t whichSong, bool repeat = false);
		void PlayEffect(uint16_t whichEffect, bool looping = false);
		void StopEfect(uint16_t whichEffect);
		void PlayVoice(uint16_t whichVoice) {} //idk

		void PlayNextSong() {}
		void StopMusic() {
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

		float GetVolume(SoundVolume whichVolume) { return volumes[(uint8_t)whichVolume]; }
		float GetVolume(int8_t whichVolume) { return volumes[whichVolume]; }

		//0 is the default device
		void SwitchDevices(uint16_t deviceIterator);
		void SetVolume(SoundVolume whichVolume, uint8_t value);
		//void loadEffects(std::unordered_map<uint16_t, std::string>& loadEffects);
		void LoadSoundMap(std::unordered_map<uint16_t, std::string>& loadSounds, SoundType soundType);

		std::vector<std::string> deviceNames;
		uint16_t GetSelectedDevice() {
			return selectedEngine;
		}

		void initVolume() {
			SetVolume(SoundVolume::master, SettingsJSON::settingsData.masterVolume);
			SetVolume(SoundVolume::effect, SettingsJSON::settingsData.effectsVolume);
			SetVolume(SoundVolume::music, SettingsJSON::settingsData.musicVolume);
			SetVolume(SoundVolume::voice, SettingsJSON::settingsData.voiceVolume);
		}

	private:
		//ma_sound_group effectGroup;
		//ma_sound_group musicGroup;
		//ma_sound_group voiceGroup;
		std::vector<std::unordered_map<uint16_t, ma_sound>> effects;
		std::vector<std::unordered_map<uint16_t, ma_sound>> music;
		std::vector<std::unordered_map<uint16_t, ma_sound>> voices;
		std::unordered_map<uint16_t, std::string> effectLocations;
		std::unordered_map<uint16_t, std::string> musicLocations;
		std::unordered_map<uint16_t, std::string> voiceLocations;

		ma_decoder hwDecoder;
		ma_sound hwSound;

		std::array<float, 4> volumes{ 0.5f, 0.5f, 0.5f, 0.5f };
		
		ma_context context;
		ma_resource_manager_config resourceManagerConfig;
		ma_resource_manager resourceManager;
		std::vector<ma_engine> engines{};
		uint16_t selectedEngine{65535};

		std::vector<ma_device> devices{ 0 };

		ma_device_info* pPlaybackDeviceInfos; 
		ma_uint32 playbackDeviceCount{ 0 };
		//std::vector<ma_sound>sounds{};
		//ma_sound* selectedSound{ nullptr };
		void LoadHowlingWind();
		void InitEngines(ma_device_info* deviceInfos, uint32_t deviceCount);
		void ReloadSounds();

		uint16_t currentSong = UINT16_MAX;
		ma_uint64 currentPCMFrames = 0;

	
	};
}