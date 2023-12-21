#include "EWEngine/SettingsJson.h"


SettingsJSON::SettingsData SettingsJSON::settingsData;
SettingsJSON::SettingsData SettingsJSON::tempSettings;

namespace SettingsInfo {
	std::pair<uint32_t, uint32_t> getScreenDimensions(ScreenDimension_Enum SD_E) {
		switch (SD_E) {

			//wide scren
		case SD_1920W: {
			return { 1920, 1080 };
		}
		case SD_1536W: {
			return { 1536, 864 };
		}
		case SD_1366W: {
			return { 1366, 768 };
		}
		case SD_1280W: {
			return { 1280, 720 };
		}
		case SD_800W: {
			return { 800, 450 };
		}
		case SD_640W: {
			return { 640, 360 };
		}
					//4:3
		case SD_1280: {
			return { 1280, 960 };
		}
		case SD_800: {
			return { 800, 600 };
		}
		case SD_640: {
			return { 640, 480 };
		}
		default: {
			printf("why default sreen dimension? \n");
			return { 1280,720 };
		}
		}
	};
	std::string getScreenDimensionString(ScreenDimension_Enum SD_E) {
		switch (SD_E) {
		case SD_1920W: {
			return "1920x1080";
		}
		case SD_1536W: {
			return "1536x864";
		}
		case SD_1366W: {
			return "1366x768";
		}
		case SD_1280W: {
			return "1280x720";
		}
		case SD_800W: {
			return "800x450";
		}
		case SD_640W: {
			return "640x360";
		}
					//4:3
		case SD_1280: {
			return "1280x960";
		}
		case SD_800: {
			return "800x600";
		}
		case SD_640: {
			return "640x480";
		}
		default: {
			printf("why default sreen dimension? \n");
			return "please report this";
		}
		}
	}

	std::vector<std::string> getScreenDimensionStringVector() {
		return{
			{"1920x1080"},
			{"1536x864"},
			{"1366x768"},
			{"1280x720"},
			{"800x450"},
			{"640x360"},
			//4x3
			{"1280x960"},
			{"800x600"},
			{"640x480"},
		};
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
	throw std::exception("invalid volume type");
	return masterVolume;
}
SettingsInfo::ScreenDimension_Enum SettingsJSON::SettingsData::setDimensions(int width, int height) {
	printf("setting dimensions - %d:%d \n", width, height);
	for (int i = 0; i < SettingsInfo::SD_size; i++) {
		if (SettingsInfo::getScreenDimensions((SettingsInfo::ScreenDimension_Enum)i).first == width && SettingsInfo::getScreenDimensions((SettingsInfo::ScreenDimension_Enum)i).second == height) {
			screenDimensions = (SettingsInfo::ScreenDimension_Enum)i;
			printf("found screen dimensions : %s \n", SettingsInfo::getScreenDimensionString(screenDimensions).c_str());
			return screenDimensions;
		}
	}
	printf("couldnt find screen dimensions????? \n");
	screenDimensions = SettingsInfo::SD_1280;
	return SettingsInfo::SD_1280;
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
			throw std::exception("failed to parse");
			generateDefaultFile();
		}
		else {
			if (!readFromJsonFile(document)) {
				//failed to parse correctly
				printf("failed to read settings correctly \n");
				throw std::exception("failed t o read settings correctly");
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
	SettingsData settingsDefault;
	settingsData = settingsDefault;
	tempSettings = settingsDefault;
	printf("generating default file \n");
	saveToJsonFile();
}

bool SettingsJSON::readFromJsonFile(rapidjson::Document& document) {
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
	if (!document["screenDimensions"].IsInt()) {
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



	settingsData.versionKey = CURRENT_VERSION;

	int valueBuffer = document["windowMode"].GetInt();
	if (valueBuffer != 0 && valueBuffer != 1) {
		settingsData.windowMode = SettingsInfo::WT_borderless;
	}
	else {
		settingsData.windowMode = (SettingsInfo::WindowMode_Enum)valueBuffer;
	}

	valueBuffer = document["screenDimensions"].GetInt();
	if (valueBuffer >= 0 && valueBuffer < SettingsInfo::SD_size) {
		settingsData.screenDimensions = (SettingsInfo::ScreenDimension_Enum)valueBuffer;
	}
	settingsData.masterVolume = document["masterVolume"].GetUint();

	settingsData.effectsVolume = document["effectsVolume"].GetUint();
	settingsData.musicVolume = document["musicVolume"].GetUint();
	settingsData.voiceVolume = document["voiceVolume"].GetUint();

	//holma
	if (settingsData.masterVolume < 0 || settingsData.masterVolume > 100) {
		settingsData.masterVolume = 50;
	}
	if (settingsData.effectsVolume < 0 || settingsData.effectsVolume > 100) {
		settingsData.effectsVolume = 50;
	}
	if (settingsData.musicVolume < 0 || settingsData.musicVolume > 100) {
		settingsData.musicVolume = 50;
	}
	if (settingsData.voiceVolume < 0 || settingsData.voiceVolume > 100) {
		settingsData.voiceVolume = 50;
	}

	settingsData.selectedDevice = document["selectedDevice"].GetString();
	settingsData.FPS = document["FPS"].GetInt();
	if (settingsData.FPS < 0) {
		settingsData.FPS = 0;
	}

	settingsData.pointLights = document["pointLights"].GetBool();
	settingsData.renderInfo = document["renderInfo"].GetBool();

	tempSettings = settingsData;

	return true;
}

void SettingsJSON::saveToJsonFile() {
	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	writer.StartObject();
	writer.Key("version");
	writer.Int(CURRENT_VERSION);
	writer.Key("windowMode");
	writer.Int(settingsData.windowMode);
	writer.Key("screenDimensions");
	writer.Int(settingsData.screenDimensions);
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
