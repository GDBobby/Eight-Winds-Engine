#include "EWEngine/SettingsJson.h"

#include <include/rapidjson/document.h>
#include <include/rapidjson/prettywriter.h>// for stringify JSON
#include <include/rapidjson/error/en.h>

SettingsJSON::SettingsData SettingsJSON::settingsData;
SettingsJSON::SettingsData SettingsJSON::tempSettings;

namespace SettingsInfo {
	std::vector<ScreenDimensions> ScreenDimensions::commonDimensions{
		{7680, 4320},
		{5120, 2160},
		{5120, 1440},
		{3840, 2160},
		{3840, 1080},
		{3440, 1440},
		{3440, 1440},
		{2560, 1440},
		{2560, 1080},
		{1920, 1080},
		{1600, 900},
		{1536, 864},
		{1440, 900},
		{1366, 768},
		{1280, 1024},
		{1280, 960},
		{1280, 720},
		{1024, 768},
		{800, 600},
		{800, 450},
		{640, 480},
		{640, 360}
	};
	void ScreenDimensions::FixCommonDimensionsToScreenSize(int width, int height) {
		for (uint16_t i = 0; i < commonDimensions.size(); i++) {
			if (width < commonDimensions[i].width || height < commonDimensions[i].height) {
				commonDimensions.erase(commonDimensions.begin() + i);
				i--;
			}
		}
	}
	ScreenDimensionsFloat ScreenDimensions::ConvertToFloat() {
		ScreenDimensionsFloat ret{};
		ret.width = static_cast<float>(width);
		ret.height = static_cast<float>(height);
		return ret;
	}

	std::string ScreenDimensions::GetString() const {

		std::string ret{ std::to_string(width) };
		ret += "x";
		ret += std::to_string(height);
		return ret;
	}

	std::vector<std::string> getWindowModeStringVector() {
		return{
			{"windowed"},
			{"borderless"},
			//{"fullscreen"},
		};
	}
	std::string getWindowModeString(WindowMode_Enum WT_E) {
		switch (WT_E) {
		case WT_windowed: {
			return { "windowed" };
		}
		case WT_borderless: {

			return { "borderless" };
		}
						  /*
						  case WT_fullscreen: {

							  return { "fullscreen" };
						  }
						  */
		default: {
			return "wtf WM";
		}
		}
	}

	std::vector<std::string> getFPSStringVector() {
		return {
			{"60"},
			{"75"},
			{"120"},
			{"144"},
			{"150"},
			{"240"},
			{"250"},
			//{"Custom"},
		};
	}
	uint16_t getFPSInt(FPS_Enum FPS_E) {
		switch (FPS_E) {
		case FPS_60: {
			return 60;
		}
		case FPS_75: {
			return 75;
		}
		case FPS_120: {
			return 120;
		}
		case FPS_144: {
			return 144;
		}
		case FPS_150: {
			return 150;
		}
		case FPS_240: {
			return 240;
		}
		case FPS_250: {
			return 250;
		}
		default: {
			return 0;
		}
		}
	}
	std::string getFPSString(uint16_t FPS) {
		switch (FPS) {
		case 60: {
			return "60";
		}
		case 75: {
			return "75";
		}
		case 120: {
			return "120";
		}
		case 144: {
			return "144";
		}
		case 150: {
			return "150";
		}
		case 240: {
			return "240";
		}
		case 250: {
			return "250";
		}
		default: {
			return "Custom";
		}
		}
	}
	FPS_Enum getFPSEnum(uint16_t FPS) {
		switch (FPS) {
		case 60: {
			return FPS_60;
		}
		case 75: {
			return FPS_75;
		}
		case 120: {
			return FPS_120;
		}
		case 144: {
			return FPS_144;
		}
		case 150: {
			return FPS_150;
		}
		case 240: {
			return FPS_240;
		}
		case 250: {
			return FPS_250;
		}
		default: {
			return FPS_Custom;
		}
		}
	}
} //namespace SettingsInfo

bool readFromJsonFile(rapidjson::Document& document) {
	if (!document.HasMember("version")) {
		return false;
	}
	if (!document["version"].IsInt()) {
		printf("version not int \n");
		return false;
	}
	else if (document["version"].GetInt() != CURRENT_VERSION) {
		printf("icnorrect version \n");
		return false;
	}

	if (!document["windowMode"].IsInt()) {
		printf("wm not int \n");
		return false;
	}
	if (!document["screenDimensionsX"].IsInt()) {
		printf("SD not int \n");
		return false;
	}
	if (!document["screenDimensionsY"].IsInt()) {
		printf("SD not int \n");
		return false;
	}
	if (!document["masterVolume"].IsInt()) {
		printf("MaV not int \n");
		return false;
	}
	if (!document["effectsVolume"].IsInt()) {
		printf("EV not int \n");
		return false;
	}
	if (!document["musicVolume"].IsInt()) {
		printf("MuV not int \n");
		return false;
	}
	if (!document["voiceVolume"].IsInt()) {
		printf("VV not int \n");
		return false;
	}
	if (!document["selectedDevice"].IsString()) {
		printf("device not string \n");
		return false;
	}
	if (!document["FPS"].IsInt()) {
		printf("FPS not int \n");
		return false;
	}
	if (!document["pointLights"].IsBool()) {
		printf("PL not bool \n");
		return false;
	}
	if (!document["renderInfo"].IsBool()) {
		printf("render not bool \n");
		return false;
	}



	SettingsJSON::settingsData.versionKey = CURRENT_VERSION;

	int valueBuffer = document["windowMode"].GetInt();
	if (valueBuffer != 0 && valueBuffer != 1) {
		SettingsJSON::settingsData.windowMode = SettingsInfo::WT_borderless;
	}
	else {
		SettingsJSON::settingsData.windowMode = (SettingsInfo::WindowMode_Enum)valueBuffer;
	}

	valueBuffer = document["screenDimensionsX"].GetInt();
	SettingsJSON::settingsData.screenDimensions.width = valueBuffer;
	valueBuffer = document["screenDimensionsY"].GetInt();
	SettingsJSON::settingsData.screenDimensions.height = valueBuffer;
	
	SettingsJSON::settingsData.masterVolume = document["masterVolume"].GetUint();

	SettingsJSON::settingsData.effectsVolume = document["effectsVolume"].GetUint();
	SettingsJSON::settingsData.musicVolume = document["musicVolume"].GetUint();
	SettingsJSON::settingsData.voiceVolume = document["voiceVolume"].GetUint();

	//holma
	if (SettingsJSON::settingsData.masterVolume < 0 || SettingsJSON::settingsData.masterVolume > 100) {
		SettingsJSON::settingsData.masterVolume = 50;
	}
	if (SettingsJSON::settingsData.effectsVolume < 0 || SettingsJSON::settingsData.effectsVolume > 100) {
		SettingsJSON::settingsData.effectsVolume = 50;
	}
	if (SettingsJSON::settingsData.musicVolume < 0 || SettingsJSON::settingsData.musicVolume > 100) {
		SettingsJSON::settingsData.musicVolume = 50;
	}
	if (SettingsJSON::settingsData.voiceVolume < 0 || SettingsJSON::settingsData.voiceVolume > 100) {
		SettingsJSON::settingsData.voiceVolume = 50;
	}

	SettingsJSON::settingsData.selectedDevice = document["selectedDevice"].GetString();
	SettingsJSON::settingsData.FPS = document["FPS"].GetInt();
	if (SettingsJSON::settingsData.FPS < 0) {
		SettingsJSON::settingsData.FPS = 0;
	}

	SettingsJSON::settingsData.pointLights = document["pointLights"].GetBool();
	SettingsJSON::settingsData.renderInfo = document["renderInfo"].GetBool();

	SettingsJSON::tempSettings = SettingsJSON::settingsData;

	return true;
}


void SettingsJSON::SettingsData::setVolume(int8_t whichVolume, uint8_t value) {
	//ma_device_set_SoundVolume::master(&device, volume[whichVolume]);
	//printf("setting volume %d : %.2f \n", whichVolume, value);

	if (whichVolume == (uint8_t)SoundVolume::master) {
		masterVolume = value;
	}
	else if (whichVolume == (uint8_t)SoundVolume::effect) {
		effectsVolume = value;
	}
	else if (whichVolume == (uint8_t)SoundVolume::music) {
		musicVolume = value;
	}
	else if (whichVolume == (uint8_t)SoundVolume::voice) {
		voiceVolume = value;
	}
}
const uint8_t& SettingsJSON::SettingsData::getVolume(int8_t whichVolume) {
	if (whichVolume == (uint8_t)SoundVolume::master) {
		return masterVolume;
	}
	else if (whichVolume == (uint8_t)SoundVolume::effect) {
		return effectsVolume;
	}
	else if (whichVolume == (uint8_t)SoundVolume::music) {
		return musicVolume;
	}
	else if (whichVolume == (uint8_t)SoundVolume::voice) {
		return voiceVolume;
	}
	std::cout << "invalid volumne type " << std::endl;
	throw std::runtime_error("invalid volume type");
	return masterVolume;
}




void SettingsJSON::initializeSettings() {
	rapidjson::Document document;

	if (!std::filesystem::exists(SETTINGS_LOCATION)) {
		//no file exist
		std::ofstream tempFile{ SETTINGS_LOCATION };
		generateDefaultFile();
		tempFile.close();
	}
	else {
		std::ifstream inFile;
		inFile.open(SETTINGS_LOCATION, std::ios::binary);

		// get length of file:
		inFile.seekg(0, std::ios::end);
		size_t length = inFile.tellg();
		inFile.seekg(0, std::ios::beg);

		// allocate memory:
		char* buffer = new char[length + 1];

		// read data as a block:
		inFile.read(buffer, length);
		buffer[length] = '\0';

		document.Parse(buffer);
		if (document.HasParseError() || !document.IsObject()) {
			printf("error parsing settings at : %s \n", SETTINGS_LOCATION);
			printf("error at %d : %s \n", static_cast<int32_t>(document.GetErrorOffset()), rapidjson::GetParseError_En(document.GetParseError()));
			//assert(false);
			generateDefaultFile();
		}
		else {
			if (!readFromJsonFile(document)) {
				//failed to parse correctly
				printf("failed to read settings correctly \n");
				//assert(false);
				generateDefaultFile();
			}
		}
		// delete temporary buffer
		delete[] buffer;

		// close filestream
		inFile.close();
	}
}

void SettingsJSON::generateDefaultFile() {
	SettingsData settingsDefault{};
	settingsData = settingsDefault;
	tempSettings = settingsDefault;
	printf("generating default file \n");
	saveToJsonFile();
}


void SettingsJSON::saveToJsonFile() {
	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	writer.StartObject();
	writer.Key("version");
	writer.Int(CURRENT_VERSION);
	writer.Key("windowMode");
	writer.Int(settingsData.windowMode);
	writer.Key("screenDimensionsX");
	writer.Int(settingsData.screenDimensions.width);
	writer.Key("screenDimensionsY");
	writer.Int(settingsData.screenDimensions.height);
	writer.Key("masterVolume");
	writer.Uint(settingsData.masterVolume);
	writer.Key("effectsVolume");
	writer.Uint(settingsData.effectsVolume);
	writer.Key("musicVolume");
	writer.Uint(settingsData.musicVolume);
	writer.Key("voiceVolume");
	writer.Uint(settingsData.voiceVolume);
	writer.Key("selectedDevice");
	writer.String(settingsData.selectedDevice.c_str());
	writer.Key("FPS");
	writer.Int(settingsData.FPS);
	writer.Key("pointLights");
	writer.Bool(settingsData.pointLights);
	writer.Key("renderInfo");
	writer.Bool(settingsData.renderInfo);
	writer.EndObject();
	std::ofstream file;
	file.open(SETTINGS_LOCATION);
	file << sb.GetString();
	file.close();

}
