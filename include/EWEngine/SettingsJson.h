#pragma once

#include <include/rapidjson/document.h>
#include <include/rapidjson/prettywriter.h>// for stringify JSON
#include <include/rapidjson/error/en.h>


#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>


#define CURRENT_VERSION 10
#define SETTINGS_LOCATION "settings.json"


enum class SoundVolume {
	master,
	effect,
	music,
	voice,

	//overflow,
};

namespace SettingsInfo {
	enum ScreenDimension_Enum {
		//wide screen dimensions
		SD_1920W,
		SD_1536W,
		SD_1366W,
		SD_1280W,
		SD_800W,
		SD_640W,

		//narrwo? 4:3 dimensions
		SD_1280,
		SD_800,
		SD_640,


		SD_size //default to this,
	};
	std::pair<uint32_t, uint32_t> getScreenDimensions(ScreenDimension_Enum SD_E);
	std::string getScreenDimensionString(ScreenDimension_Enum SD_E);
	std::vector<std::string> getScreenDimensionStringVector();

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
		int versionKey = CURRENT_VERSION; //i dont think i can use periods, maybe somethin else like X

		SettingsInfo::WindowMode_Enum windowMode{ SettingsInfo::WT_borderless };
		SettingsInfo::ScreenDimension_Enum screenDimensions = SettingsInfo::SD_size; //keyd to a set of dimensions, that ill enum above

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

		[[nodiscard]] std::pair<uint32_t, uint32_t> getDimensions() {
			return getScreenDimensions(screenDimensions);
		}
		SettingsInfo::ScreenDimension_Enum setDimensions(int width, int height);
		std::string getDimensionsString() {
			return SettingsInfo::getScreenDimensionString(screenDimensions);
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
	static bool readFromJsonFile(rapidjson::Document& document);
	static void saveToJsonFile();
};