#include "EWEngine/GUI/MenuModule.h"

namespace EWE {


	std::map<MenuTextureEnum, uint16_t> MenuModule::textureIDs;
	std::unique_ptr<EWEModel> MenuModule::model2D;
	std::unique_ptr<EWEModel> MenuModule::nineUIModel;

	std::queue<MenuClickReturn> MenuModule::clickReturns;

	void (*MenuModule::changeMenuStateFromMM)(uint8_t, unsigned char);

	void MenuModule::initTextures(EWEDevice& eweDevice) {
		//these textures are deleted in EWETexture, when the program is cleaning itself up on close

		textureIDs.emplace(MT_NineUI, EWETexture::addUITexture(eweDevice, "UI/NineUI.png"));
		textureIDs.emplace(MT_NineFade, EWETexture::addUITexture(eweDevice, "UI/NineFade.png"));
		textureIDs.emplace(MT_Slider, EWETexture::addUITexture(eweDevice, "UI/clickyBox.png"));
		textureIDs.emplace(MT_BracketButton, EWETexture::addUITexture(eweDevice, "UI/bracketButton.png"));
		textureIDs.emplace(MT_Bracket, EWETexture::addUITexture(eweDevice, "UI/bracketSlide.png"));
		textureIDs.emplace(MT_Unchecked, EWETexture::addUITexture(eweDevice, "UI/unchecked.png"));
		textureIDs.emplace(MT_Checked, EWETexture::addUITexture(eweDevice, "UI/checked.png"));
		//textureIDs.emplace(MT_background, EWETexture::addUITexture(eweDevice, "UI/mainPageBG.png"));
		textureIDs.emplace(MT_Button, EWETexture::addUITexture(eweDevice, "UI/ButtonUp.png"));
		{
			textureIDs.emplace(MT_Base, EWETexture::addUITexture(eweDevice, "UI/menuBase.png"));
		}

		model2D = EWEModel::generate2DQuad(eweDevice);
		nineUIModel = EWEModel::generateNineUIQuad(eweDevice);
	}
	/*
	MenuModule::MenuModule(MenuStates menuState, float screenWidth, float screenHeight) {
		//printf("constructing \n");
		float widthRescaling = screenWidth / DEFAULT_WIDTH;
		float heightRescaling = screenHeight / DEFAULT_HEIGHT;

		switch (menuState) {
		case menu_main: {

			labels.emplace_back("Eight Winds", 0.f, 140.f, TA_center, 6.f);
			labels.emplace_back("GAME_VERSION", screenWidth, screenHeight * 0.1f, TA_right, 3.f);
			//main menu clickable

			clickText.emplace_back("Play", 0.f, 200.f * heightRescaling, TA_left, 3.f, screenWidth, screenHeight);
			clickText.emplace_back("Controls", 0.f, 280.f * heightRescaling, TA_left, 3.f, screenWidth, screenHeight);
			clickText.emplace_back("Graphics Settings", 0.f, 360.f * heightRescaling, TA_left, 3.f, screenWidth, screenHeight);
			clickText.emplace_back("Audio Settings", 0.f, 440.f * heightRescaling, TA_left, 3.f, screenWidth, screenHeight);
			clickText.emplace_back("Level Builder", 0.f, 520.f * heightRescaling, TA_left, 3.f, screenWidth, screenHeight);

			clickText.emplace_back("Exit", 0.f, 620.f * heightRescaling, TA_left, 3.f, screenWidth, screenHeight);
			// EXAMPLE
			//dropBoxes.push_back({});
			//dropBoxes[0].dropper.textStruct.string = "Tester";
			//dropBoxes[0].dropper.textStruct.x = screenWidth * 2.f / 3.f;
			//dropBoxes[0].dropper.textStruct.y = 400.f * heightRescaling;
			//dropBoxes[0].dropper.textStruct.scale = 1.f;
			//dropBoxes[0].dropper.textStruct.align = TA_left;
			//dropBoxes[0].align = TA_left;
			//dropBoxes[0].pushOption("test1");
			//dropBoxes[0].pushOption("test1121Testeteststest12");
			//dropBoxes[0].pushOption("test3");
			//dropBoxes[0].init(screenWidth, screenHeight);

			//typeBoxes.emplace_back(TextStruct{ "testBox", screenWidth / 2.f, screenHeight / 2.f, TA_left, 1.f }, screenWidth, screenHeight);
			//
			break;
		}
		case menu_controls: {
			labels.emplace_back("Forward", screenWidth / 3.f, 140.f * heightRescaling, TA_left, 2.f);
			labels.emplace_back("Left", screenWidth / 3.f, 180.f * heightRescaling, TA_left, 2.f);
			labels.emplace_back("Back", screenWidth / 3.f, 220.f * heightRescaling, TA_left, 2.f);
			labels.emplace_back("Right", screenWidth / 3.f, 260.f * heightRescaling, TA_left, 2.f);
			labels.emplace_back("Attack", screenWidth / 3.f, 300.f * heightRescaling, TA_left, 2.f);
			labels.emplace_back("Block", screenWidth / 3.f, 340.f * heightRescaling, TA_left, 2.f);
			labels.emplace_back("Jump", screenWidth / 3.f, 380.f * heightRescaling, TA_left, 2.f);
			labels.emplace_back("Special", screenWidth / 3.f, 420.f * heightRescaling, TA_left, 2.f);
			labels.emplace_back("Sensitivity", screenWidth * .63f, 40.f * heightRescaling, TA_center, 2.f);


			clickText.emplace_back("ForwardKey", screenWidth * 2 / 3, 140.f * heightRescaling, TA_right, 2.f, screenWidth, screenHeight); //12
			clickText.emplace_back("LeftKey", screenWidth * 2 / 3, 180.f * heightRescaling, TA_right, 2.f, screenWidth, screenHeight);
			clickText.emplace_back("BackKey", screenWidth * 2 / 3, 220.f * heightRescaling, TA_right, 2.f, screenWidth, screenHeight);
			clickText.emplace_back("RightKey", screenWidth * 2 / 3, 260.f * heightRescaling, TA_right, 2.f, screenWidth, screenHeight);
			clickText.emplace_back("AttackKey", screenWidth * 2 / 3, 300.f * heightRescaling, TA_right, 2.f, screenWidth, screenHeight);
			clickText.emplace_back("BlockKey", screenWidth * 2 / 3, 340.f * heightRescaling, TA_right, 2.f, screenWidth, screenHeight);
			clickText.emplace_back("JumpKey", screenWidth * 2 / 3, 380.f * heightRescaling, TA_right, 2.f, screenWidth, screenHeight);
			clickText.emplace_back("SpecialKey", screenWidth * 2 / 3, 420.f * heightRescaling, TA_right, 2.f, screenWidth, screenHeight);


			clickText.emplace_back("Discard Return", screenWidth * .45f, 500.f * heightRescaling, TA_right, 2.f, screenWidth, screenHeight);
			clickText.emplace_back("Save Return", screenWidth * .55f, 500.f * heightRescaling, TA_left, 2.f, screenWidth, screenHeight);
			break;
		}
		case menu_graphics_settings: {
			labels.emplace_back("Graphics Settings", screenWidth / 2, 140, TA_center, 4.f);

			labels.emplace_back("Screen Dimensions", 100.f * widthRescaling, 175.f * heightRescaling, TA_left, 2.f);
			{ //screen dimensions, local scope
				comboBoxes.emplace_back(TextStruct{ SettingsJSON::settingsData.getDimensionsString(), 100.f * widthRescaling, 200.f * heightRescaling, TA_left, 1.5f }, screenWidth, screenHeight);

				std::vector<std::string> SDStrings = SettingsInfo::getScreenDimensionStringVector();
				for (int i = 0; i < SDStrings.size(); i++) {
					comboBoxes.back().pushOption(SDStrings[i], screenWidth, screenHeight);
					if (strcmp(SDStrings[i].c_str(), comboBoxes.back().activeOption.textStruct.string.c_str()) == 0) {
						comboBoxes.back().currentlySelected = i;
					}
				}
			}
			labels.emplace_back("Window Mode", 400.f * widthRescaling, 175.f * heightRescaling, TA_left, 2.f);
			{ //window mode, local scope
				comboBoxes.emplace_back(TextStruct{ getWindowModeString(SettingsJSON::settingsData.windowMode), 400.f * widthRescaling, 200.f * heightRescaling, TA_left, 1.5f }, screenWidth, screenHeight);
				std::vector<std::string> WTStrings = SettingsInfo::getWindowModeStringVector();
				for (int i = 0; i < WTStrings.size(); i++) {
					comboBoxes.back().pushOption(WTStrings[i], screenWidth, screenHeight);
					if (strcmp(WTStrings[i].c_str(), comboBoxes.back().activeOption.textStruct.string.c_str()) == 0) {
						comboBoxes.back().currentlySelected = i;
					}
				}
			}
			labels.emplace_back("FPS", 600.f * widthRescaling, 175.f * heightRescaling, TA_left, 2.f);
			{
				comboBoxes.emplace_back(TextStruct{ std::to_string(SettingsJSON::settingsData.FPS), 600.f * widthRescaling, 200.f * heightRescaling, TA_left, 1.5f }, screenWidth, screenHeight);
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
					comboBoxes.back().currentlySelected = comboBoxes.back().comboOptions.size() - 1;
				}
			}

			glm::vec2 translation;
			glm::ivec2 screenCoords = { 800 * widthRescaling, 200 * heightRescaling };
			screenCoords = { 800 * widthRescaling, 200 * heightRescaling };
			UIComp::convertScreenTo2D(screenCoords, translation, screenWidth, screenHeight);
			checkBoxes.emplace_back("Grass Enabled ", translation, Checkbox::DO_left, screenWidth, screenHeight);

			screenCoords = { 800 * widthRescaling, 240 * heightRescaling };
			UIComp::convertScreenTo2D(screenCoords, translation, screenWidth, screenHeight);
			checkBoxes.emplace_back("Point Lights ", translation, Checkbox::DO_left, screenWidth, screenHeight);

			screenCoords = { 800 * widthRescaling, 280 * heightRescaling };
			UIComp::convertScreenTo2D(screenCoords, translation, screenWidth, screenHeight);
			checkBoxes.emplace_back("Render Info ", translation, Checkbox::DO_left, screenWidth, screenHeight);


			clickText.emplace_back(TextStruct{ "Discard Return", screenWidth * .3f, 500.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Save Return", screenWidth / 2.f, 500.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			break;
		}
		case menu_audio_settings: {
			labels.emplace_back("Audio Settings", screenWidth / 2, 40.f * heightRescaling, TA_center, 3.f);
			labels.emplace_back("Audio Devices", screenWidth * 0.49f, 460.f * heightRescaling, TA_right, 1.f);
			labels.emplace_back("Master Volume", screenWidth / 2, 170.f * heightRescaling, TA_center, 1.f);
			labels.emplace_back("Effects Volume", screenWidth / 2, 265.f * heightRescaling, TA_center, 1.f);
			labels.emplace_back("Msuci Volume", screenWidth / 2, 355.f * heightRescaling, TA_center, 1.f);

			//clickText.emplace_back(TextStruct{ "No Device", screenWidth * 0.51f, 460.f * heightRescaling, TA_left, 1.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Discard Return", screenWidth * .3f, 500.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Save Return", screenWidth / 2.f, 500.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);

			comboBoxes.emplace_back(TextStruct{ "default", screenWidth * 0.51f, 460.f * heightRescaling, TA_left, 1.f }, screenWidth, screenHeight);

			break;
		}
		case menu_main_play_selection: {
			labels.emplace_back("Single Player", screenWidth * .25f, 180.f * heightRescaling, TA_center, 2.1f);
			labels.emplace_back("Multiplayer", screenWidth * .75f, 180.f * heightRescaling, TA_center, 2.1f);
			labels.emplace_back("Playfab Not Connected", screenWidth, screenHeight - 30, TA_right, 1.f);

			clickText.emplace_back(TextStruct{ "Practice", screenWidth * .25f, 220.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Return", screenWidth / 2.f, 500.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Target Course", screenWidth * .25f, 300.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Host Lobby", screenWidth * .75f, 220.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Show Lobby List", screenWidth * .75f, 300.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Join Ladder", screenWidth * .75f, 380.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			break;
		}
		case menu_play: {
			labels.emplace_back("Eight Winds", screenWidth / 2, 100 * heightRescaling, TA_center, 6.f);


			clickText.emplace_back(TextStruct{ "Resume", screenWidth / 2, 240.f * heightRescaling, TA_center, 3.f }, screenWidth, screenHeight);
			//clickText.emplace_back("Rollback 30 frames", screenWidth / 2, 310, TA_center, 3.f);
			clickText.emplace_back(TextStruct{ "Controls", screenWidth / 2, 340.f * heightRescaling, TA_center, 3.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Graphics Settings", screenWidth / 2, 410.f * heightRescaling, TA_center, 3.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Audio Settings", screenWidth / 2, 480.f * heightRescaling, TA_center, 3.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Return To Main Menu", screenWidth / 2, 570.f * heightRescaling, TA_center, 3.f }, screenWidth, screenHeight);
			//clickText.emplace_back(TextStruct{ "Exit", screenWidth / 2, 640.f * heightRescaling, TA_center, 3.f}, screenWidth, screenHeight);
			break;
		}
		case menu_lobby: {
			labels.emplace_back("Host's Lobby", screenWidth / 2, 100 * heightRescaling, TA_center, 4.f);


			clickText.emplace_back(TextStruct{ "Start Game", screenWidth * .45f, 520.f * heightRescaling, TA_right, 2.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Leave Lobby", screenWidth * .55f, 520.f * heightRescaling, TA_left, 2.f }, screenWidth, screenHeight);
			//clickText.emplace_back(TextStruct{ "Invite through Steam", screenWidth / 2, 480.f * heightRescaling, TA_center, 2.f}, screenWidth, screenHeight);
			break;
		}
		case menu_lobby_list: {
			labels.emplace_back("Lobby List", screenWidth / 2, 100 * heightRescaling, TA_center, 2.f);

			clickText.emplace_back(TextStruct{ "Refresh", screenWidth * 0.25f, 500.f * heightRescaling, TA_left, 2.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Return", screenWidth / 2.f, 500.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			break;
		}
		case menu_ladder: {
			labels.emplace_back("Searching...", screenWidth * .4f, screenHeight / 2, TA_left, 2.f);

			clickText.emplace_back(TextStruct{ "Cancel", screenWidth / 2.f, 500.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			break;
		}
		case menu_target_selection: {
			labels.emplace_back("Target Course Selection", screenWidth / 2.f, screenHeight * 0.2f, TA_center, 3.f);

			clickText.emplace_back(TextStruct{ "Mountain", screenWidth * 0.25f, screenHeight * 0.3f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			clickText.emplace_back(TextStruct{ "Straight Away", screenWidth * 0.25f, (screenHeight * 0.3f + 80.f) * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);

			clickText.emplace_back(TextStruct{ "Leaderboard", screenWidth * 0.75f, screenHeight * 0.3f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);

			clickText.emplace_back(TextStruct{ "Return", screenWidth / 2.f, 500.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			break;
		}
		case menu_target_completion: {
			//labels.emplace_back("completion time", screenWidth / 2.f, screenHeight * 0.2f, TA_center, 3.f);

			clickText.emplace_back(TextStruct{ "Reset", screenWidth * .3f, 500.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);

			clickText.emplace_back(TextStruct{ "Return To Main Menu", screenWidth / 2.f, 500.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);
			break;
		}
		case menu_target_leaderboard: {
			labels.emplace_back("Target Leaderboard", screenWidth / 2.f, screenHeight * 0.2f, TA_center, 3.f);
			labels.emplace_back("Mountain", screenWidth * .2f, 200.f * heightRescaling, TA_center, 2.f);

			clickText.emplace_back(TextStruct{ "Return", screenWidth / 2.f, 500.f * heightRescaling, TA_center, 2.f }, screenWidth, screenHeight);

			hasBackground = true;
			backgroundTransform.translation = { 0.f,0.f };
			backgroundTransform.scale = { 1.75f,1.f };
			break;
		}
		case menu_level_builder: {
			MenuBar& backRef = menuBars.emplace_back(0.f, 0.f, screenWidth, 30.f * heightRescaling, screenWidth, screenHeight);

			std::vector<std::string> options = {
				"New",
				"Save",
				"Save As",
				"Load",
				"Return To Menu",
				"Exit"
			};
			backRef.pushDropper("File", options, screenWidth, screenHeight);

			options.clear();
			options = {
				"Undo",
				"Redo",
				"Copy",
				"Paste",
				"Settings"
			};
			backRef.pushDropper("Edit", options, screenWidth, screenHeight);

			options.clear();
			options = {
				"Add Quad",
				"Object Control",
				"Light Settings",
				"Reset Camera Pos"
			};
			backRef.pushDropper("View", options, screenWidth, screenHeight);
			backRef.init(screenWidth, screenHeight);

			printf("bug before control box construction? \n");
			float steps[] = { 0.01f, .1f, 1.f };
			//TypeVariableBox(void* data, size_t variableCount, VariableType vType, float x, float y, float width, float screenWidth, float screenHeight, void* stepData, NamingConvention nameConv = NC_numeric)

			//i need to give this the variable after construction, then call an init function within the VariableTypeBoxes class
			controlBoxes.emplace_back(mainWindow.getGLFWwindow(), "Player", 600.f, 300.f, 100.f, screenWidth, screenHeight);

			std::string stringBuffer = "Translation";
			controlBoxes[0].emplaceVariableControl(stringBuffer, PlayerObject::poVector[0].playerInput.liveState->translation, UIComp::VT_float, 3, (void*)steps);
			stringBuffer = "Rotation";
			controlBoxes[0].emplaceVariableControl(stringBuffer, &PlayerObject::poVector[0].playerInput.liveState->cameraRotation[1], UIComp::VT_float, 1, (void*)steps);
			controlBoxes[0].giveGLFWCallbacks(uiHandler.staticMouseCallback, uiHandler.staticKeyCallback);

			
			printf("bug after control box destruciton \n");

			break;
		}


		}
	}
	*/

	std::pair<UIComponentTypes, int8_t> MenuModule::checkClick(double xpos, double ypos) {
		std::pair<UIComponentTypes, int8_t> returnVal = { UIT_none, -1 };
		if (selectedComboBox >= 0) {
			printf("checking click for selected combobox : %d \n", selectedComboBox);
			//one combo box is unraveled
			for (int i = 0; i < comboBoxes[selectedComboBox].comboOptions.size(); i++) {
				if (comboBoxes[selectedComboBox].comboOptions[i].Clicked(xpos, ypos)) {
					comboBoxes[selectedComboBox].setSelection(i);
					//UIComp::TextToTransform(comboBoxes[selectedComboBox].activeOption.transform, comboBoxes[selectedComboBox].activeOption.textStruct, comboBoxes[selectedComboBox].activeOption.clickBox, screenWidth, screenHeight);
					return { UIT_Combobox, i };
				}
			}
			comboBoxes[selectedComboBox].currentlyDropped = false;
			selectedComboBox = -1;
			return returnVal;
		}
		for (int i = 0; i < comboBoxes.size(); i++) {
			//int16_t comboClick = comboBoxes[i]; //not finished yet
			//return comboClick + sliders.size() * 3;
			if (comboBoxes[i].Clicked(xpos, ypos)) {
				printf("clicked a combo box? :%d - xpos:ypos %.1f:%.1f \n", i, xpos, ypos);

				selectedComboBox = i;
				return { UIT_Combobox, -1 };
			}
		}
		for (int i = 0; i < dropBoxes.size(); i++) {
			int8_t clickBuffer = dropBoxes[i].Clicked(xpos, ypos);
			if (clickBuffer > -2) {
				printf("clicked a drop box? :%d:%d - xpos:ypos %.1f:%.1f \n", i, clickBuffer, xpos, ypos);
				selectedDropBox = i;
				return { UIT_Dropbox, -1 };
			}
		}

		for (int i = 0; i < sliders.size(); i++) {
			int8_t sliderClick = sliders[i].Clicked(xpos, ypos);
			//printf("checking slider %d:%d \n", i, sliderClick);
			//printf("clickboxes, 0 - x(%d:%d) : y(%d:%d) \n", sliders[i].click[0].x, sliders[i].click[0].z, sliders[i].click[0].y, sliders[i].click[0].w);
			//printf("\t  1 - x(%d:%d) : y(%d:%d) \n", sliders[i].click[1].x, sliders[i].click[1].z, sliders[i].click[1].y, sliders[i].click[1].w);
			//printf("\t  2 - x(%d:%d) : y(%d:%d) \n", sliders[i].click[2].x, sliders[i].click[2].z, sliders[i].click[2].y, sliders[i].click[2].w);
			if (sliderClick != -1) {
				return { UIT_Slider, i * 3 + sliderClick };
			}
		}
		for (int i = 0; i < checkBoxes.size(); i++) {
			//printf("checking check box %d \n", i);
			//printf("click bounds - x(%d:%d) : y(%d:%d) \n",
				//checkBoxes[i].clickBox.x, checkBoxes[i].clickBox.z,
				//checkBoxes[i].clickBox.y, checkBoxes[i].clickBox.w
			//);
			if (checkBoxes[i].Clicked(xpos, ypos)) {
				return { UIT_Checkbox, i };
			}
		}
		for (int i = 0; i < clickText.size(); i++) {
			if (clickText[i].Clicked(xpos, ypos)) {
				return { UIT_ClickTextBox, i };
			}
		}
		for (int i = 0; i < typeBoxes.size(); i++) {
			if (typeBoxes[i].Clicked(xpos, ypos)) {
				return { UIT_TypeBox, i };
			}
		}
		for (int i = 0; i < controlBoxes.size(); i++) {

			if (controlBoxes[i].Clicked(xpos, ypos)) { //come back to this
				return { UIT_VariableTypeBox, i };
			}
		}
		for (int i = 0; i < menuBars.size(); i++) {
			menuBars[i].Clicked(xpos, ypos);
		}

		return returnVal;
	}

	void MenuModule::drawObjects(VkCommandBuffer cmdBuf, uint8_t frameIndex, bool background) {

		//printf("drawing objects in menu module \n");
		/*
		* go front to back
			Combo,
			Slider,
			BracketButton,
			Bracket,
			ClickBox,
			unChecked,
			CheckedBox,
		*/
		Simple2DPushConstantData push{};

		model2D->bind(cmdBuf);

		if (checkBoxes.size() > 0) {
			push.color = glm::vec3{ 1.f };
			for (int i = 0; i < checkBoxes.size(); i++) {
				if (checkBoxes[i].isChecked) {
					if (lastBindedTexture != textureIDs[MT_Checked]) {
						vkCmdBindDescriptorSets(
							cmdBuf,
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							PipelineManager::pipeLayouts[PL_2d],
							0, 1,
							//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
							EWETexture::getUIDescriptorSets(textureIDs[MT_Checked], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
							0, nullptr
						);
						lastBindedTexture = textureIDs[MT_Checked];
					}
					push.scaleOffset = glm::vec4(checkBoxes[i].button.transform.scale, checkBoxes[i].button.transform.translation);
					vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_2d], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
					model2D->draw(cmdBuf);
				}
			}
			for (int i = 0; i < checkBoxes.size(); i++) {
				if (!checkBoxes[i].isChecked) {
					if (lastBindedTexture != textureIDs[MT_Unchecked]) {
						vkCmdBindDescriptorSets(
							cmdBuf,
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							PipelineManager::pipeLayouts[PL_2d],
							0, 1,
							//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
							EWETexture::getUIDescriptorSets(textureIDs[MT_Unchecked], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
							0, nullptr
						);
						lastBindedTexture = textureIDs[MT_Unchecked];
					}
					push.scaleOffset = glm::vec4(checkBoxes[i].button.transform.scale, checkBoxes[i].button.transform.translation);
					vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_2d], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
					model2D->draw(cmdBuf);
				}
			}
		}

		if (sliders.size() > 0) {
			push.color = glm::vec3(1.f);
			if (lastBindedTexture != textureIDs[MT_Slider]) {
				vkCmdBindDescriptorSets(
					cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_2d],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_Slider], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				lastBindedTexture = textureIDs[MT_Slider];
			}

			for (int i = 0; i < sliders.size(); i++) {
				push.scaleOffset = glm::vec4(sliders[i].slider.mat2(), sliders[i].slider.translation);
				vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_2d], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
				model2D->draw(cmdBuf);
			}
			if (lastBindedTexture != textureIDs[MT_BracketButton]) {
				vkCmdBindDescriptorSets(
					cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_2d],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_BracketButton], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				lastBindedTexture = textureIDs[MT_BracketButton];
			}
			for (int i = 0; i < sliders.size(); i++) {
				push.scaleOffset = glm::vec4(sliders[i].bracketButtons.first.mat2(), sliders[i].bracketButtons.first.translation);
				vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_2d], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
				model2D->draw(cmdBuf);
				push.scaleOffset = glm::vec4(sliders[i].bracketButtons.second.mat2(), sliders[i].bracketButtons.second.translation);
				vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_2d], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
				model2D->draw(cmdBuf);
			}
			if (lastBindedTexture != textureIDs[MT_Bracket]) {
				vkCmdBindDescriptorSets(
					cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_2d],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_Bracket], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				lastBindedTexture = textureIDs[MT_Bracket];
			}
			for (int i = 0; i < sliders.size(); i++) {
				push.scaleOffset = glm::vec4(sliders[i].bracket.mat2(), sliders[i].bracket.translation);
				vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_2d], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
				model2D->draw(cmdBuf);
			}
		}

		if (controlBoxes.size() > 0) {
			//printf("before draw objects control boxes \n");
			push.color = glm::vec3(1.f);
			if (lastBindedTexture != textureIDs[MT_Button]) {
				vkCmdBindDescriptorSets(
					cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_2d],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_Button], frameIndex), //need to change this texture to a button texture, + -
					0, nullptr
				);
				lastBindedTexture = textureIDs[MT_Button];
			}
			for (int i = 0; i < controlBoxes.size(); i++) {
				for (int j = 0; j < controlBoxes[i].variableControls.size(); j++) {
					//printf("variable controls size, j - %d:%d \n", controlBoxes[i].variableControls.size(), j);
					for (int k = 0; k < controlBoxes[i].variableControls[j].buttons.size(); k++) {
						push.scaleOffset = glm::vec4(controlBoxes[i].variableControls[j].buttons[k].first.transform.scale, controlBoxes[i].variableControls[j].buttons[k].first.transform.translation);
						vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_2d], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
						model2D->draw(cmdBuf);
						push.scaleOffset = glm::vec4(controlBoxes[i].variableControls[j].buttons[k].second.transform.scale, controlBoxes[i].variableControls[j].buttons[k].second.transform.translation);
						vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_2d], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
						model2D->draw(cmdBuf);
					}
				}
			}
			//printf("after control boxes \n");
		}

		if (images.size() > 0) {
			//not considering for texture ordering
			push.color = glm::vec3(1.f);
			for (int i = 0; i < images.size(); i++) {
				if (lastBindedTexture != images[i].textureID) {
					vkCmdBindDescriptorSets(
						cmdBuf,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						PipelineManager::pipeLayouts[PL_2d],
						0, 1,
						//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
						EWETexture::getUIDescriptorSets(images[i].textureID, frameIndex), //need to change this texture to a button texture, + -
						0, nullptr
					);
					lastBindedTexture = images[i].textureID;
				}
				push.scaleOffset = glm::vec4(images[i].transform.scale, images[i].transform.translation);
				vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_2d], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
				model2D->draw(cmdBuf);
			}
		}
		//? wtf is this
		/*
		if (hasBackground) {
			push.color = backgroundColor;
			vkCmdBindDescriptorSets(
				cmdBuf,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_2d],
				0, 1,
				//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
				EWETexture::getUIDescriptorSets(textureIDs[MT_Base], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up
				0, nullptr
			);
			push.offset = glm::vec3(backgroundTransform.translation, 1.f);
			//need color array
			push.scale = backgroundTransform.scale;
			vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_2d], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
			model2D->draw(cmdBuf);
		}
		*/
		/*
		if (background) {
			push.color = { 1.f,1.f,1.f };
			vkCmdBindDescriptorSets(
				cmdBuf,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_2d],
				0, 1,
				//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
				EWETexture::getUIDescriptorSets(textureIDs[MT_background], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up
				0, nullptr
			);
			push.offset = { 0.f,0.f, 1.f };
			//need color array
			push.scale = { 2.f, 2.f };
			vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_2d], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
			model2D->draw(cmdBuf);
		}
		*/
		//printf("after rendering bakground \n");
	}

	void MenuModule::drawNineUI(VkCommandBuffer cmdBuf, uint8_t frameIndex) {
		//printf("beginning nine ui \n");
		nineUIModel->bind(cmdBuf);
		NineUIPushConstantData push{};
		if (comboBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			if (lastBindedTexture != MT_NineUI) {
				vkCmdBindDescriptorSets(
					cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineUI], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				lastBindedTexture = MT_NineUI;
			}
			for (int i = 0; i < comboBoxes.size(); i++) {
				push.offset = glm::vec4(comboBoxes[i].activeOption.transform.translation, 1.f, 1.f);
				//need color array
				push.scale = comboBoxes[i].activeOption.transform.scale;
				vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(cmdBuf);
				if (comboBoxes[i].currentlyDropped) {
					for (int j = 0; j < comboBoxes[i].comboOptions.size(); j++) {
						push.offset = glm::vec4(comboBoxes[i].comboOptions[j].transform.translation, 1.f, 1.f);
						//need color array
						push.scale = comboBoxes[i].comboOptions[j].transform.scale;
						if (j == comboBoxes[i].currentlySelected) {
							push.color = glm::vec3{ .4f, .4f, 1.f };
							vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
							nineUIModel->draw(cmdBuf);
							push.color = glm::vec3{ .5f, .35f, .25f };
						}
						else {
							vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
							nineUIModel->draw(cmdBuf);
						}
					}
				}
			}
		}
		if (dropBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			if (lastBindedTexture != MT_NineUI) {
				vkCmdBindDescriptorSets(
					cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineUI], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				lastBindedTexture = MT_NineUI;
			}
			for (int i = 0; i < dropBoxes.size(); i++) {
				push.offset = glm::vec4(dropBoxes[i].dropper.transform.translation, 1.f, 1.f);
				//need color array
				if (dropBoxes[i].currentlyDropped) {
					push.color = glm::vec3{ .75f, .35f, .25f };
				}
				push.scale = dropBoxes[i].dropper.transform.scale;
				vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(cmdBuf);
				push.color = glm::vec3{ .5f, .35f, .25f };
				if (dropBoxes[i].currentlyDropped) {
					push.offset = glm::vec4(dropBoxes[i].dropBackground.translation, 0.5f, 1.f);
					push.scale = dropBoxes[i].dropBackground.scale;
					vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
					nineUIModel->draw(cmdBuf);
				}
			}
		}

		if (clickText.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			if (lastBindedTexture != textureIDs[MT_NineUI]) {
				vkCmdBindDescriptorSets(
					cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineUI], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				lastBindedTexture = textureIDs[MT_NineUI];
			}
			for (int i = 0; i < clickText.size(); i++) {
				push.offset = glm::vec4(clickText[i].transform.translation, 1.f, 1.f);
				//need color array
				push.scale = clickText[i].transform.scale;
				vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(cmdBuf);
				//printf("drawing click text \n");
			}
		}

		if (typeBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			if (lastBindedTexture != textureIDs[MT_NineUI]) {
				vkCmdBindDescriptorSets(
					cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineUI], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				lastBindedTexture = textureIDs[MT_NineUI];
			}
			for (int i = 0; i < typeBoxes.size(); i++) {
				push.offset = glm::vec4(typeBoxes[i].transform.translation, 1.f, 1.f);
				//need color array
				push.scale = typeBoxes[i].transform.scale;
				vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(cmdBuf);
				//printf("drawing click text \n");
			}
		}

		//printf("before nineui control box \n");
		if (controlBoxes.size() > 0) {
			if (lastBindedTexture != textureIDs[MT_NineUI]) {
				vkCmdBindDescriptorSets(
					cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineUI], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				lastBindedTexture = textureIDs[MT_NineUI];
			}
			for (int i = 0; i < controlBoxes.size(); i++) {
				for (int j = 0; j < controlBoxes[i].variableControls.size(); j++) {
					for (int k = 0; k < controlBoxes[i].variableControls[j].typeBoxes.size(); k++) {
						if (controlBoxes[i].variableControls[j].isSelected(k)) {
							push.color = glm::vec3{ .6f, .5f, .4f };
						}
						else {
							push.color = glm::vec3{ .5f, .35f, .25f };
						}
						push.offset = glm::vec4(controlBoxes[i].variableControls[j].typeBoxes[k].transform.translation, 1.f, 1.f);
						push.scale = controlBoxes[i].variableControls[j].typeBoxes[k].transform.scale;
						vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
						nineUIModel->draw(cmdBuf);
					}
				}
				push.color = glm::vec3{ .3f, .25f, .15f };
				push.offset = glm::vec4(controlBoxes[i].transform.translation, 1.f, 1.f);
				push.scale = controlBoxes[i].transform.scale;
				vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(cmdBuf);
			}
			//printf("after nine ui control boxes \n");
		}

		if (menuBars.size() > 0) {
			for (int i = 0; i < menuBars.size(); i++) {

				if (menuBars[i].dropBoxes.size() > 0) { //drawing these here instead of tumblingg these with the earlier drop boxes because i dont want to draw the dropper box
					push.color = glm::vec3{ .5f, .35f, .25f };
					for (int j = 0; j < menuBars[i].dropBoxes.size(); j++) {
						push.color = glm::vec3{ .5f, .35f, .25f };
						if (menuBars[i].dropBoxes[j].currentlyDropped) {
							push.offset = glm::vec4(menuBars[i].dropBoxes[j].dropBackground.translation, 0.5f, 1.f);
							push.scale = menuBars[i].dropBoxes[j].dropBackground.scale;
							vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
							nineUIModel->draw(cmdBuf);
						}
					}
				}
			}

			if (lastBindedTexture != textureIDs[MT_NineFade]) {
				vkCmdBindDescriptorSets(
					cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameIndex + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineFade], frameIndex), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				lastBindedTexture = textureIDs[MT_NineFade];
			}
			for (int i = 0; i < menuBars.size(); i++) {

				push.color = glm::vec3{ .86f, .5f, .5f };
				push.offset = glm::vec4(menuBars[i].transform.translation, 0.1f, 1.f);
				push.scale = menuBars[i].transform.scale;
				vkCmdPushConstants(cmdBuf, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(cmdBuf);
			}
		}
		lastBindedTexture = -1;
	}

	void MenuModule::resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) {
		//i think its just clickboxes?
		for (int i = 0; i < sliders.size(); i++) {
			sliders[i].resizeWindow(rszWidth, rszHeight); //ok why is this given a pointer to window size
		}
		for (int i = 0; i < comboBoxes.size(); i++) {
			comboBoxes[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		for (int i = 0; i < checkBoxes.size(); i++) {
			checkBoxes[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		for (int i = 0; i < labels.size(); i++) {
			labels[i].x *= rszWidth / oldWidth;
			labels[i].y *= rszHeight / oldHeight;
		}
		for (int i = 0; i < clickText.size(); i++) {
			clickText[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		for (int i = 0; i < dropBoxes.size(); i++) {
			dropBoxes[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		for (int i = 0; i < typeBoxes.size(); i++) {
			typeBoxes[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		for (int i = 0; i < controlBoxes.size(); i++) {
			controlBoxes[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		for (int i = 0; i < menuBars.size(); i++) {
			menuBars[i].init(rszWidth, rszHeight);
		}
	}

	void MenuModule::drawText(TextOverlay* textOverlay) {
		for (int i = 0; i < comboBoxes.size(); i++) {
			textOverlay->addText(comboBoxes[i].activeOption.textStruct);
			if (comboBoxes[i].currentlyDropped) {
				for (int j = 0; j < comboBoxes[i].comboOptions.size(); j++) {
					textOverlay->addText(comboBoxes[i].comboOptions[j].textStruct);
				}
			}
		}
		for (int i = 0; i < dropBoxes.size(); i++) {
			textOverlay->addText(dropBoxes[i].dropper.textStruct);
			if (dropBoxes[i].currentlyDropped) {
				for (int j = 0; j < dropBoxes[i].dropOptions.size(); j++) {
					textOverlay->addText(dropBoxes[i].dropOptions[j]);
				}
			}
		}
		for (int i = 0; i < checkBoxes.size(); i++) {
			textOverlay->addText(checkBoxes[i].label);
		}
		for (int i = 0; i < labels.size(); i++) {
			textOverlay->addText(labels[i]);
		}
		for (int i = 0; i < clickText.size(); i++) {
			textOverlay->addText(clickText[i].textStruct);
		}
		for (int i = 0; i < typeBoxes.size(); i++) {
			textOverlay->addText(typeBoxes[i].textStruct);
		}
		for (int i = 0; i < controlBoxes.size(); i++) {
			textOverlay->addText(controlBoxes[i].label);
			for (int j = 0; j < controlBoxes[i].variableControls.size(); j++) {
				for (int k = 0; k < controlBoxes[i].variableControls[j].typeBoxes.size(); k++) {
					textOverlay->addText(controlBoxes[i].variableControls[j].typeBoxes[k].textStruct);
				}
				textOverlay->addText(controlBoxes[i].variableControls[j].dataLabel);
			}
		}
		for (int i = 0; i < menuBars.size(); i++) {
			for (int j = 0; j < menuBars[i].dropBoxes.size(); j++) {
				textOverlay->addText(menuBars[i].dropBoxes[j].dropper.textStruct);
				if (menuBars[i].dropBoxes[j].currentlyDropped) {
					for (int k = 0; k < menuBars[i].dropBoxes[j].dropOptions.size(); k++) {
						textOverlay->addText(menuBars[i].dropBoxes[j].dropOptions[k]);
					}
				}
			}
		}
	}

	std::string MenuModule::getInputName(int keyCode) {
		//std::cout << "get mouse? : " << keyCode << std::endl;
		//std::cout << "get mouse key : " << keyCode << std::endl;
		if (keyCode == 0) {
			return "LMB";
		}
		else if (keyCode == 1) {
			return "RMB";
		}
		else if (keyCode == 2) {
			return "MMB";
		}
		else if (keyCode <= 10) {
			std::string tempString = "MB";
			tempString += std::to_string(keyCode - 2);
			// std::cout << "other mouse key : " << tempString << std::endl;
			printf("other mouse key : %s \n", tempString.c_str());
			return tempString;
		}
		else if (keyCode == GLFW_KEY_SPACE) {
			return "SPACE";
		}
		else if (keyCode == GLFW_KEY_LEFT_SHIFT) {
			return "L Shift";
		}
		else if (keyCode == GLFW_KEY_LEFT_CONTROL) {
			return "L Ctrl";
		}
		else if (keyCode == GLFW_KEY_RIGHT_CONTROL) {
			return "R Ctrl";
		}
		else if (keyCode == GLFW_KEY_RIGHT_SHIFT) {
			return "R Shift";
		}
		else if (keyCode == GLFW_KEY_TAB) {
			return "Tab";
		}
		std::string tempString;
		if (((keyCode >= GLFW_KEY_0) && (keyCode <= GLFW_KEY_9)) || ((keyCode >= GLFW_KEY_A) && (keyCode <= GLFW_KEY_Z))) {
			tempString = glfwGetKeyName(keyCode, 0);
		}
		else {
			tempString = "unknown:";
			tempString += std::to_string(keyCode);
		}
		//std::cout << "standard key?" << std::endl;
		//char tempChar = glfwGetKeyName(keyCode, 0);
		//std::cout << "before getkeyname" << std::endl;

		//std::cout << "end of get mouse" << std::endl;
		return tempString;
	};
}