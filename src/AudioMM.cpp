#include "EWEngine/GUI/AudioMM.h"

namespace EWE {
	AudioMM* AudioMM::audioPtr{ nullptr };

	void AudioMM::AudioPosCallback(GLFWwindow* window, double xpos, double ypos) {
		//controlPtr->
		if (glfwGetMouseButton(audioPtr->windowPtr, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
			glfwSetCursorPosCallback(audioPtr->windowPtr, nullptr);
			return; //works fine
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
		audioPtr->sliders[audioPtr->grabbedSlider].SetSliderPosition(static_cast<float>(nextVolume) / 100.f);
	}

	AudioMM::AudioMM(GLFWwindow* windowPtr) : soundEngine{ SoundEngine::GetSoundEngineInstance() }, windowPtr{ windowPtr }{
		assert(audioPtr == nullptr && "audiomm can only be creatged once");
		audioPtr = this;

		//float widthRescaling = screenWidth / DEFAULT_WIDTH;
		const float heightRescaling = VK::Object->screenHeight / DEFAULT_HEIGHT;

		labels.emplace_back("Audio Settings", VK::Object->screenWidth / 2, 40.f * heightRescaling, TA_center, 4.f);
		labels.emplace_back("Audio Devices", VK::Object->screenWidth * 0.75f, 365.f * heightRescaling, TA_left, 2.f);
		labels.emplace_back("Master Volume", VK::Object->screenWidth / 2, 260.f * heightRescaling, TA_center, 2.f);
		labels.emplace_back("Effects Volume", VK::Object->screenWidth / 2, 390.f * heightRescaling, TA_center, 2.f);
		labels.emplace_back("Msuci Volume", VK::Object->screenWidth / 2, 520.f * heightRescaling, TA_center, 2.f);

		clickText.emplace_back(TextStruct{ "Discard Return", VK::Object->screenWidth * .3f, 700.f * heightRescaling, TA_center, 2.f });
		clickText.emplace_back(TextStruct{ "Save Return", VK::Object->screenWidth / 2.f, 700.f * heightRescaling, TA_center, 2.f });

		comboBoxes.emplace_back(TextStruct{ "default", VK::Object->screenWidth * 0.75f, 400.f * heightRescaling, TA_left, 1.f });

		initSoundSettings();
	}

	void AudioMM::processClick(double xpos, double ypos) {
		std::pair<UIComponentTypes, int16_t> returnValues = MenuModule::CheckClick(xpos, ypos);
		if (returnValues.first > 0) {
			soundEngine->PlayEffect(0);
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
			if (selectedComboBox == 0 && (returnValues.second >= 0)) { //only works if wsound devices are in combobox[0]
				//printf("which device switching to? %s \n",)
				soundEngine->SwitchDevices(returnValues.second);
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

				//clickReturns.push(MCR_DiscardReturn);
				callbacks[0]();
				//return MCR_DiscardReturn;
			}
			else if (returnValues.second == 1) { //save return
				printf("save return \n");
				SettingsJSON::settingsData = SettingsJSON::tempSettings;
				SettingsJSON::saveToJsonFile();
				soundEngine->initVolume();
				
				//clickReturns.push(MCR_SaveReturn);
				callbacks[1]();
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
				comboBoxes[0].PushOption(soundEngine->deviceNames[i].substr(0, strLength));
			}
			else {
				//comboMenuStructs[0].push_back({ soundEngine->deviceNames[i], screenWidth / 2, 80, TA_left, 2.f });
				comboBoxes[0].PushOption(soundEngine->deviceNames[i]);
			}
			//printf("device name : i ~ %s : i \n", comboMenuStructs[0].back().string.c_str(), i);
			if (i == soundEngine->GetSelectedDevice()) {
				foundSavedDevice = true;
				//soundEngine->switchDevices(i);
				//printf("yo wtf is music volume here? %.2f \n", soundEngine->getVolume(SoundVolume::music));
				//menuStructs[menu_audio_settings].second[0].string = comboMenuStructs[0].back().string;

				comboBoxes[0].SetSelection(static_cast<int8_t>(comboBoxes[0].comboOptions.size() - 1));
				//menuMap[menu_audio_settings].comboBoxes.emplace_back(TextStruct{ soundEngine->deviceNames[i].substr(0, strLength), })
			}
			//printf("after : % s \n", comboMenuStructs[0].back().string.c_str());
			//printf("compare? : %d \n", SettingsJSON::settingsData.selectedDevice.compare(comboMenuStructs[0].back().string));
		}
		if (foundSavedDevice) {
			soundEngine->SetVolume(SoundVolume::master, SettingsJSON::settingsData.masterVolume);
			soundEngine->SetVolume(SoundVolume::music, SettingsJSON::settingsData.musicVolume);
			soundEngine->SetVolume(SoundVolume::effect, SettingsJSON::settingsData.effectsVolume);
		}

		initVolumes(soundEngine->GetVolume(SoundVolume::master), soundEngine->GetVolume(SoundVolume::music), soundEngine->GetVolume(SoundVolume::effect));
	}

	void AudioMM::resetSounds(float master, float music, float sfx) {
		sliders[0].SetSliderPosition(master);
		sliders[1].SetSliderPosition(sfx);
		sliders[2].SetSliderPosition(music);
	}
}