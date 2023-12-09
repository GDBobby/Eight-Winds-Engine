#pragma once

#include "SettingsJson.h"

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <map>
#include <array>
#include <unordered_map>

#define IRRK_LIB false //irrklang
#define MINI_LIB true //miniaudio
#define SOUND_ENGINE_LIB IRRK_LIB

#if MINI_LIB
#define MINIAUDIO_IMPLEMENTATION
#if defined(MA_WIN32) //miniaudio brings in <windows> (clenches fist)
#define NOMINMAX
#endif
#include "miniaudio/miniaudio.h"

namespace EWE {
	class SoundEngine {
	private:
		SoundEngine();
		//~MaterialHandler() = default;
		SoundEngine(const SoundEngine&) = delete;
		SoundEngine& operator=(const SoundEngine&) = delete;
	public:
		//need to make this a singleton, and pass in the default device on construction


		static std::shared_ptr<SoundEngine> getSoundEngineInstance() {
			static std::shared_ptr<SoundEngine> soundEngineInstance{ new SoundEngine };
			return soundEngineInstance;
		}

		~SoundEngine();
		void loadEffectsFromFile();


		void playMusic(uint8_t whichSong, bool repeat = false) {}
		void playEffect(uint8_t whichEffect);
		void playClickEffect();
		void playVoice(uint8_t whichVoice) {} //idk

		void playNextSong() {}
		void stopMusic() {}

		const float& getVolume(SoundVolume whichVolume) { return volume[whichVolume]; }
		const float& getVolume(int8_t whichVolume) { return volume[whichVolume]; }

		void switchDevices(int deviceIterator);
		void setVolume(int8_t whichVolume, uint8_t value) {}
		void initVolume() {
			setVolume(master_volume, SettingsJSON::settingsData.masterVolume);
			setVolume(effect_volume, SettingsJSON::settingsData.effectsVolume);
			setVolume(music_volume, SettingsJSON::settingsData.musicVolume);
			setVolume(voice_volume, SettingsJSON::settingsData.voiceVolume);
		}

		void loadEffects(std::unordered_map<uint16_t, std::string>& loadEffects);

		std::vector<std::string> deviceNames;
	private:
		std::unordered_map<uint16_t, ma_sound> effects; //file locations mapped to iterators; could use a vector as well
		ma_sound clickEffect;

		bool deviceInit = false;

		std::array<float, 4> volume{ 0.5f, 0.5f, 0.5f, 0.5f };
		
		ma_context context;
		ma_resource_manager_config resourceManagerConfig;
		ma_resource_manager resourceManager;
		//im being forced into C arrays rather than vectors
		//workaround, easily enough, is to just temp C arrays then copy to vectors
		//but, since this code will be self contianed, ill just deal with C arrays for the minor performance boost
		std::vector<ma_engine> engines{};
		ma_engine* selectedEngine{ nullptr };

		std::vector<ma_device> devices{ 0 };

		ma_device_info* pPlaybackDeviceInfos; 
		ma_uint32 playbackDeviceCount{ 0 };
		uint32_t availableDevice{ 0 };
		uint32_t chosenDevice{ 0 };
		//std::vector<ma_sound>sounds{};
		//ma_sound* selectedSound{ nullptr };

	
	};
}


#elif IRRK_LIB
#include <irrKlang/irrKlang.h>




enum SFX_Enum {
	//these need to be in order of the files, but it preserves sounds played outside this class if the order is disrupted, as long as its correct here
	//file order is alphabetical, i believe that wont change but idk
	sfx_BAH,
	//sfx_BAH1,
	//sfx_BAH2,
	sfx_boom,
	//sfx_bornReady,
	sfx_click,
	sfx_dash,
	//sfx_deathBell,
	//sfx_fireMagic,
	sfx_jump,
	sfx_klink,
	//sfx_levelup,
	//sfx_skeletal,
	//sfx_zap,
	sfx_Zhiteffect,
};
enum Music_Enum {
	Music_Bridge1,
	Music_BridgeM,
	Music_Devour,
	Music_howlingWind,
	Music_Menu,
	Music_Phase1,
	Music_Ritual,
	Music_Temple,
	Music_blink,

	Music_invalid,
};

#pragma comment(lib, "irrKlang.lib")
//#include <iostream>
class StopEventReceiver;

namespace EWE {
	class SoundEngine {
	private:
		SoundEngine();
		//~MaterialHandler() = default;
		SoundEngine(const SoundEngine&) = delete;
		SoundEngine& operator=(const SoundEngine&) = delete;
	public:
		//need to make this a singleton, and pass in the default device on construction


        static std::shared_ptr<SoundEngine> getSoundEngineInstance() {
            static std::shared_ptr<SoundEngine> soundEngineInstance{ new SoundEngine };
            return soundEngineInstance;
        }

		~SoundEngine();
		void loadEffectsFromFile();


		void playMusic(uint8_t whichSong, bool repeat = false);
		void playEffect(SFX_Enum whichEffect);
		void playVoice(uint8_t whichVoice);

		void playNextSong();
		void stopMusic();

		const float& getVolume(SoundVolume whichVolume) { return volume[whichVolume]; }
		const float& getVolume(int8_t whichVolume) { return volume[whichVolume]; }

		void switchDevices(int deviceIterator);
		void setVolume(int8_t whichVolume, uint8_t value);
		void initVolume() {
			setVolume(master_volume, SettingsJSON::settingsData.masterVolume);
			setVolume(effect_volume, SettingsJSON::settingsData.effectsVolume);
			setVolume(music_volume, SettingsJSON::settingsData.musicVolume);
			setVolume(voice_volume, SettingsJSON::settingsData.voiceVolume);
		}

		//const uint32_t& getDefaultDevice() { return defaultDevice; }

		std::vector<std::string> deviceNames;

	private:
		irrklang::ISoundEngine* engine;
		StopEventReceiver* stopEventReceiver;

		irrklang::ISoundDeviceList* deviceList;
		
		irrklang::IAudioStreamLoader* audioStreamLoader;

		std::vector<irrklang::ISoundSource*> effects;
		std::vector<irrklang::IAudioStream*> musicAudioStream;
		std::map<Music_Enum, irrklang::ISoundSource*> musicMap;

		bool songIsPlaying = false;

		bool deviceWasSwitched = false;
		//uint8_t lastSong = 0;
		uint8_t currentSong = 0;
		irrklang::ISound* currentSongSound = nullptr;

		std::vector<irrklang::ISoundSource*> voice;
		std::vector<irrklang::ISound*> musicSounds;

		//master, effects, music, voice    ~ really just saving some trouble on if else statements in setVolume(), or splitting setVolume into SetEffectsVolume, setMasterVolume etc 
		std::vector<float> volume{0.1f, 0.5f, 0.5f, 0.5f};

		
		void mapMusicFile(const std::filesystem::directory_entry& musicPath);

	};
}

class StopEventReceiver : public irrklang::ISoundStopEventReceiver {
public:
	StopEventReceiver(EWE::SoundEngine* soundEngine) {
		sndEng = soundEngine;
	}
	~StopEventReceiver() {

	}
	EWE::SoundEngine* sndEng;
	void OnSoundStopped(irrklang::ISound* sound, irrklang::E_STOP_EVENT_CAUSE reason, void* userData) {
		switch (reason) {
		case irrklang::ESEC_SOUND_FINISHED_PLAYING: {
			sndEng->playNextSong();
			break;
		}
		case irrklang::ESEC_SOUND_STOPPED_BY_USER: {
			// dont do anything for now, fix this up later if it becomes an issue
			break;
		}
		default: {
			printf("bug catcher, sound ended unacceptably, reason : %d \n", reason);
			break;
		}
		}

	}
};
#endif