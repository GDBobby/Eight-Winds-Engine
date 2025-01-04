#pragma once


#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <type_traits>


#define CURRENT_SETTINGS_VERSION 11
#define SETTINGS_LOCATION "settings.json"


enum class SoundVolume {
	master,
	effect,
	music,
	voice,

	//overflow,
};

namespace SettingsInfo {
	struct ScreenDimensionsFloat {
		float width;
		float height;

		bool operator==(ScreenDimensionsFloat const other) const {
			return (width == other.width) && (height == other.height);
		}
		bool operator!=(ScreenDimensionsFloat const other) const {
			return !this->operator==(other);
		}
	};

	struct ScreenDimensions {
		uint32_t width;
		uint32_t height;

		static std::vector<ScreenDimensions> commonDimensions;
		static void FixCommonDimensionsToScreenSize(int width, int height);

		ScreenDimensionsFloat ConvertToFloat();
		bool operator==(ScreenDimensions const other) const {
			return (width == other.width) && (height == other.height);
		}
		bool operator!=(ScreenDimensions const other) const {
			return !this->operator==(other);
		}

		std::string GetString() const;
	};

	enum WindowMode_Enum {
		WT_windowed,
		WT_borderless,
		//WT_fullscreen,


		WT_size,
	};
	std::vector<std::string> getWindowModeStringVector();
	std::string getWindowModeString(WindowMode_Enum WT_E);

	enum FPS_Enum {
		FPS_60,
		FPS_75,
		FPS_120,
		FPS_144,
		FPS_150,
		FPS_240,
		FPS_250,
		FPS_Custom,
	};
	std::vector<std::string> getFPSStringVector();
	uint16_t getFPSInt(FPS_Enum FPS_E);
	std::string getFPSString(uint16_t FPS);
	FPS_Enum getFPSEnum(uint16_t FPS);
}

class SettingsJSON {
public:
	struct SettingsData {
		int versionKey = CURRENT_SETTINGS_VERSION; //i dont think i can use periods, maybe somethin else like X

		SettingsInfo::WindowMode_Enum windowMode{ SettingsInfo::WT_borderless };
		SettingsInfo::ScreenDimensions screenDimensions{1920, 1080};

		//swapping to uint8_t, with no 0 - 100
		uint8_t masterVolume{ 50 }; //volume is 0 ~ 1, 1 being 100%
		uint8_t effectsVolume{ 50 };
		uint8_t musicVolume{ 50 };
		uint8_t voiceVolume{ 50 };
		std::string selectedDevice{ "default" };
		uint16_t FPS = 144;
		bool pointLights = false;
		bool renderInfo = false;

		void setVolume(int8_t whichVolume, uint8_t value);
		const uint8_t& getVolume(int8_t whichVolume);


		std::string GetDimensionsString() {
			return screenDimensions.GetString();
		}
		std::string getWindowModeString() {
			return SettingsInfo::getWindowModeString(windowMode);
		}
		std::string getFPSString() {
			return std::to_string(FPS);
		}
		SettingsInfo::FPS_Enum getFPSEnum() {
			return SettingsInfo::getFPSEnum(FPS);
		}

		void setFPS(SettingsInfo::FPS_Enum fpsEnum) {
			if (fpsEnum) { return; } //what is this???
			else {
				FPS = SettingsInfo::getFPSInt(fpsEnum);
			}
		}
	};
	static SettingsData settingsData;
	static SettingsData tempSettings;

	static void initializeSettings();

	static void generateDefaultFile();
	static void saveToJsonFile();
};