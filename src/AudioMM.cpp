#include "EWEngine/gui/AudioMM.h"

namespace EWE {
	AudioMM* AudioMM::audioPtr{ nullptr };

	void AudioMM::AudioPosCallback(GLFWwindow* window, double xpos, double ypos) {
		//controlPtr->
		if (glfwGetMouseButton(audioPtr->windowPtr, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
			glfwSetCursorPosCallback(audioPtr->windowPtr, nullptr);
			return;
		}
		int16_t slider_movement = (static_cast<int16_t>(xpos - audioPtr->mousePosX));
		audioPtr->mousePosX = xpos;
		int16_t nextVolume = static_cast<int16_t>(SettingsJSON::tempSettings.getVolume(audioPtr->grabbedSlider));
		if (nextVolume + slider_movement <= 0) {
			nextVolume = 0;
			SettingsJSON::tempSettings.setVolume(audioPtr->grabbedSlider, 0);
		}
		else if (nextVolume + slider_movement >= 100) {
			nextVolume = 100;
			SettingsJSON::tempSettings.setVolume(audioPtr->grabbedSlider, 100);
		}
		else {
			nextVolume += slider_movement;
			SettingsJSON::tempSettings.setVolume(audioPtr->grabbedSlider, static_cast<uint8_t>(nextVolume));
		}
		audioPtr->sliders[audioPtr->grabbedSlider].setSliderPosition(static_cast<float>(nextVolume) / 100.f);
	}

	AudioMM::AudioMM(float screenWidth, float screenHeight, GLFWwindow* windowPtr) : soundEngine{ SoundEngine::getSoundEngineInstance() }, screenWidth{ screenWidth }, screenHeight{ screenHeight }, windowPtr{ windowPtr } {
		if (audioPtr == nullptr) {
			audioPtr = this;
		}
		else {
			throw std::exception("audio mm can only be created once \n");
		}

		float widthRescaling = screenWidth / DEFAULT_WIDTH;
		float heightRescaling = screenHeight / DEFAULT_HEIGHT;

		labels.emplace_back("Audio Settings", screenWidth / 2, 40.f * heightRescaling, TA_center, 4.f);
		labels.emplace_back("Audio Devices", screenWidth * 0.75f, 365.f * heightRescaling, TA_left, 2.f);
		labels.emplace_back("Master Volume", screenWidth / 2, 260.f * heightRescaling, TA_center, 2.f);
		labels.emplace_back("Effects Volume", screenWidth / 2, 390.f * heightRescaling, TA_center, 2.f);
		labels.emplace_back("Msuci Volume", screenWidth / 2, 520.f * heightRescaling, TA_center, 2.f);

		//clickText.emplace_back(TextStruct{ "No Device", screenWidth * 0.51f, 460.f * heightRescaling, TA_left, 1.f }, screenWidth, screenHeight);
		clickText.emplace_back(TextStruct{ "Discard Return", screenWidth * .3f, 700.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
		clickText.emplace_back(TextStruct{ "Save Return", screenWidth / 2.f, 700.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);


		//sliders.emplace_back();
		//sliders[0].Init(glm::vec2{ 0.f, -.4f }, screenHeight, screenWidth, 0.5f);
		//sliders.emplace_back();
		//sliders[1].Init(glm::vec2{ 0.f, -.4f + .25f }, screenHeight, screenWidth, 0.5f);
		//sliders.emplace_back();
		//sliders[2].Init(glm::vec2{ 0.f, -.4f + (.25f * 2.f) }, screenHeight, screenWidth, 0.5f);

		comboBoxes.emplace_back(TextStruct{ "default", screenWidth * 0.75f, 400.f * heightRescaling, TA_left, 1.f }, screenWidth, screenHeight);

		initSoundSettings();
	}

	void AudioMM::processClick(double xpos, double ypos) {
		std::pair<UIComponentTypes, int8_t> returnValues = MenuModule::checkClick(xpos, ypos);
		if (returnValues.first > 0) {
			soundEngine->playClickEffect();
		}
		if (returnValues.first == UIT_Slider) {
			if (returnValues.second % 3 == 1) {
				sliderMousePos = xpos;
				callbackSlider = (returnValues.second - 1) / 3;
				grabbedSlider = callbackSlider;
				mousePosX = xpos;
				mousePosY = ypos;
				printf("grabbed slider: %d \n", grabbedSlider);
				audioSelected();
			}
			else if (returnValues.second % 3 == 0) {
				int tempIterator = (returnValues.second - (returnValues.second % 3)) / 3;
				SettingsJSON::tempSettings.setVolume(tempIterator, static_cast<uint8_t>(sliders[tempIterator].slidePosition * 100.f));
			}
		}
		else if (returnValues.first == UIT_Combobox) {
			printf("returnValues.first, UIT_Combobox, returnValues.second = %d \n", returnValues.second);
			if (selectedComboBox == 0) { //only works if wsound devices are in combobox[0]
				//printf("which device switching to? %s \n",)
				soundEngine->switchDevices(returnValues.second);
				//menuStructs[menu_audio_settings].second[0].string = comboMenuStructs[0][returnValues.second].string;
				SettingsJSON::tempSettings.selectedDevice = comboBoxes[0].activeOption.textStruct.string;
			}
		}
		else if (returnValues.first == UIT_ClickTextBox) {
			if (returnValues.second == 0) { //discard return
				//printf("discard return \n");
				SettingsJSON::tempSettings = SettingsJSON::settingsData;
				//printf("after settings \n");
				soundEngine->initVolume();
				//printf("after init volume \n");
				resetSounds(static_cast<float>(SettingsJSON::settingsData.masterVolume) / 100.f, static_cast<float>(SettingsJSON::settingsData.musicVolume) / 100.f, static_cast<float>(SettingsJSON::settingsData.effectsVolume) / 100.f);
				//printf("after resetting sounds \n");

				clickReturns.push(MCR_DiscardReturn);
				//return MCR_DiscardReturn;
			}
			else if (returnValues.second == 1) { //save return
				printf("save return \n");
				SettingsJSON::settingsData = SettingsJSON::tempSettings;
				SettingsJSON::saveToJsonFile();
				soundEngine->initVolume();
				
				clickReturns.push(MCR_SaveReturn);
				//return MCR_SaveReturn;
			}
		}
		//return MCR_none;
		
	}

	void AudioMM::initSoundSettings() {
		bool foundSavedDevice = false;
		//comboMenuStructs.push_back({});
		//menuMap[menu_audio_settings].comboBoxes.emplace_back(screenWidth / 2.f, 80.f);
		for (int i = 0; (i < soundEngine->deviceNames.size()); i++) {
			int strLength = 0;
			int parenPos = 0;
			int spacePos = 0;
			for (int j = 0; j < soundEngine->deviceNames[i].length(); j++) {
				if (soundEngine->deviceNames[i][j] == '(') { parenPos = j - 1; break; }
			}
			for (int j = 0; j < soundEngine->deviceNames[i].length(); j++) {
				//FIX THIS LATER
				if (soundEngine->deviceNames[i][j] == ' ') { spacePos = j; break; }
			}
			if (parenPos > 0 && spacePos > 0) {
				strLength = glm::min(parenPos, spacePos);
			}
			else {
				//one is 0, or both are 0. so i want the one that is longer, or if both are 0, this will return 0
				strLength = glm::max(parenPos, spacePos);
			}
			//printf("before : %s \n", soundEngine->deviceNames[i].c_str());
			if (strLength != 0) {
				//comboMenuStructs[0].push_back({ soundEngine->deviceNames[i].substr(0, strLength), screenWidth / 2, 80, TA_left, 2.f });
				//printf("pushing back soudn device option, screenDim %.2f:%.2f \n", screenWidth, screenHeight);
				comboBoxes[0].pushOption(soundEngine->deviceNames[i].substr(0, strLength), screenWidth, screenHeight);
			}
			else {
				//comboMenuStructs[0].push_back({ soundEngine->deviceNames[i], screenWidth / 2, 80, TA_left, 2.f });
				comboBoxes[0].pushOption(soundEngine->deviceNames[i], screenWidth, screenHeight);
			}
			//printf("device name : i ~ %s : i \n", comboMenuStructs[0].back().string.c_str(), i);
			if (SettingsJSON::settingsData.selectedDevice.compare(comboBoxes[0].comboOptions.back().textStruct.string) == 0) {
				foundSavedDevice = true;
				soundEngine->switchDevices(i);
				//printf("yo wtf is music volume here? %.2f \n", soundEngine->getVolume(music_volume));
				//menuStructs[menu_audio_settings].second[0].string = comboMenuStructs[0].back().string;

				comboBoxes[0].setSelection(static_cast<int8_t>(comboBoxes[0].comboOptions.size() - 1));
				//menuMap[menu_audio_settings].comboBoxes.emplace_back(TextStruct{ soundEngine->deviceNames[i].substr(0, strLength), })
			}
			//printf("after : % s \n", comboMenuStructs[0].back().string.c_str());
			//printf("compare? : %d \n", SettingsJSON::settingsData.selectedDevice.compare(comboMenuStructs[0].back().string));
		}
		if (!foundSavedDevice) {
			//menuStructs[menu_audio_settings].second[0].string = soundEngine->deviceNames[0];
			comboBoxes[0].setSelection(0);
		}
		soundEngine->setVolume(master_volume, SettingsJSON::settingsData.masterVolume);
		soundEngine->setVolume(music_volume, SettingsJSON::settingsData.musicVolume);
		soundEngine->setVolume(effect_volume, SettingsJSON::settingsData.effectsVolume);

		initVolumes(soundEngine->getVolume(master_volume), soundEngine->getVolume(music_volume), soundEngine->getVolume(effect_volume), screenWidth, screenHeight);
	}

	void AudioMM::resetSounds(float master, float music, float sfx) {
		sliders[0].setSliderPosition(master);
		sliders[1].setSliderPosition(sfx);
		sliders[2].setSliderPosition(music);
	}
}