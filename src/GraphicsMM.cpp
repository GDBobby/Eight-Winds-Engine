#include "gui/GraphicsMM.h"

namespace EWE {
	GraphicsMM::GraphicsMM(float screenWidth, float screenHeight) {

		float widthRescaling = screenWidth / DEFAULT_WIDTH;
		float heightRescaling = screenHeight / DEFAULT_HEIGHT;

		labels.emplace_back("Graphics Settings", screenWidth / 2, 40.f, TA_center, 4.f);

		labels.emplace_back("Screen Dimensions", 100.f * widthRescaling, 200.f * heightRescaling, TA_left, 2.f);
		{ //screen dimensions, local scope
			printf("before dimensions \n");
			comboBoxes.emplace_back(TextStruct{ SettingsJSON::settingsData.getDimensionsString(), 100.f * widthRescaling, 240.f * heightRescaling, TA_left, 1.5f }, screenWidth, screenHeight);
			printf("after dimensions string \n");

			std::vector<std::string> SDStrings = SettingsInfo::getScreenDimensionStringVector();
			printf("after screendimensionvector \n");
			for (int i = 0; i < SDStrings.size(); i++) {
				comboBoxes.back().pushOption(SDStrings[i], screenWidth, screenHeight);
				if (strcmp(SDStrings[i].c_str(), comboBoxes.back().activeOption.textStruct.string.c_str()) == 0) {
					comboBoxes.back().currentlySelected = i;
				}
			}
			printf("after combo options \n");
		}
		printf("before window mode \n");
		labels.emplace_back("Window Mode", 500.f * widthRescaling, 200.f * heightRescaling, TA_left, 2.f);
		{ //window mode, local scope
			printf("window mode string \n");
			comboBoxes.emplace_back(TextStruct{ getWindowModeString(SettingsJSON::settingsData.windowMode), 500.f * widthRescaling, 240.f * heightRescaling, TA_left, 1.5f }, screenWidth, screenHeight);
			printf("window mode string vector \n");
			std::vector<std::string> WTStrings = SettingsInfo::getWindowModeStringVector();
			printf("after window mode string \n");
			for (int i = 0; i < WTStrings.size(); i++) {
				comboBoxes.back().pushOption(WTStrings[i], screenWidth, screenHeight);
				if (strcmp(WTStrings[i].c_str(), comboBoxes.back().activeOption.textStruct.string.c_str()) == 0) {
					comboBoxes.back().currentlySelected = i;
				}
			}
			printf("after combo options \n");
		}
		printf("before fps \n");
		labels.emplace_back("FPS", 800.f * widthRescaling, 200.f * heightRescaling, TA_left, 2.f);
		{
			comboBoxes.emplace_back(TextStruct{ std::to_string(SettingsJSON::settingsData.FPS), 800.f * widthRescaling, 240.f * heightRescaling, TA_left, 1.5f }, screenWidth, screenHeight);
			std::vector<std::string> fpsStrings = SettingsInfo::getFPSStringVector();
			comboBoxes.back().currentlySelected = -1;
			for (int i = 0; i < fpsStrings.size(); i++) {
				comboBoxes.back().pushOption(fpsStrings[i], screenWidth, screenHeight);
				if (strcmp(fpsStrings[i].c_str(), SettingsJSON::settingsData.getFPSString().c_str()) == 0) {
					comboBoxes.back().currentlySelected = i;
				}
			}
			if (comboBoxes.back().currentlySelected == -1) {
				comboBoxes.back().pushOption(SettingsJSON::settingsData.getFPSString(), screenWidth, screenHeight);
				comboBoxes.back().currentlySelected = static_cast<int8_t>(comboBoxes.back().comboOptions.size()) - 1;
			}
		}

		glm::vec2 translation;
		glm::ivec2 screenCoords = { 800 * widthRescaling, 200 * heightRescaling };
		/*
		UIComp::convertScreenTo2D(screenCoords, translation, screenWidth, screenHeight);
		printf("after converting chekcbox coords: %.2f:%.2f \n", translation.x, translation.y);
		checkBoxes.emplace_back("Resizeable ", translation, Checkbox::DO_left, screenWidth, screenHeight);
		*/

		printf("before checkboxes \n");

		screenCoords = { 1000 * widthRescaling, 340 * heightRescaling };
		UIComp::convertScreenTo2D(screenCoords, translation, screenWidth, screenHeight);
		checkBoxes.emplace_back("Point Lights ", translation, Checkbox::DO_left, screenWidth, screenHeight);

		screenCoords = { 1000 * widthRescaling, 380 * heightRescaling };
		UIComp::convertScreenTo2D(screenCoords, translation, screenWidth, screenHeight);
		checkBoxes.emplace_back("Render Info ", translation, Checkbox::DO_left, screenWidth, screenHeight);

		printf("after checkboxes \n");

		clickText.emplace_back(TextStruct{ "Discard Return", screenWidth * .3f, 700.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
		clickText.emplace_back(TextStruct{ "Save Return", screenWidth / 2.f, 700.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
		printf("end audio constructor \n");


		checkBoxes[0].isChecked = SettingsJSON::settingsData.pointLights;
		checkBoxes[1].isChecked = SettingsJSON::settingsData.renderInfo;
	}


	void GraphicsMM::processClick(double xpos, double ypos) {
		//UIComponentTypes returnValues.first, int8_t returnValues.second
		std::pair<UIComponentTypes, int8_t> returnValues = MenuModule::checkClick(xpos, ypos);

		//callbacks[returnValues.first].call(returnValues.second);

		if (returnValues.first == UIT_Checkbox) {
			switch (returnValues.second) {
			case 0: {
				SettingsJSON::tempSettings.pointLights = checkBoxes[1].isChecked;
				break;
			}
			case 1: {
				SettingsJSON::tempSettings.renderInfo = checkBoxes[2].isChecked;
			}
			}
			//return MCR_none;
		}
		if (returnValues.first == UIT_Combobox) {
			if (selectedComboBox == 0) { //screen dimensions
				printf("setting screen dimensions : %s \n", SettingsInfo::getScreenDimensionString((SettingsInfo::ScreenDimension_Enum)returnValues.second).c_str());
				SettingsJSON::tempSettings.screenDimensions = (SettingsInfo::ScreenDimension_Enum)returnValues.second;
			}
			else if (selectedComboBox == 1) { //window mode
				SettingsJSON::tempSettings.windowMode = (SettingsInfo::WindowMode_Enum)returnValues.second;
			}
			else if (selectedComboBox == 2) {
				SettingsJSON::tempSettings.FPS = SettingsInfo::getFPSInt((SettingsInfo::FPS_Enum)returnValues.second);
			}
		}

		if (returnValues.first == UIT_ClickTextBox) {
			if (returnValues.second == 0) { //discard return
				printf("discard return \n");
				SettingsJSON::tempSettings = SettingsJSON::settingsData;

				printf("screen Dim:winowMode - %d:%d \n", SettingsJSON::settingsData.screenDimensions, SettingsJSON::settingsData.windowMode);
				comboBoxes[0].setSelection(SettingsJSON::settingsData.screenDimensions);
				comboBoxes[1].setSelection(SettingsJSON::settingsData.windowMode);
				comboBoxes[2].setSelection(SettingsJSON::settingsData.getFPSEnum());
				checkBoxes[0].isChecked = SettingsJSON::settingsData.pointLights;
				checkBoxes[1].isChecked = SettingsJSON::settingsData.renderInfo;

				clickReturns.push(MCR_DiscardReturn);
				//return MCR_DiscardReturn;
			}
			else if (returnValues.second == 1) { //save return
				printf("save return \n");

				clickReturns.push(MCR_SaveReturn);
				//return MCR_SaveReturn;
			}
			else {
				printf("Default click text in graphics???? \n");
			}
		}

		//return MCR_none;

	}
}