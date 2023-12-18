#include "EWEngine/GUI/UIHandler.h"


#define USING_IMGUI false
#if USING_IMGUI
#include "EWEngine/graphics/imGuiHandler.h"
#endif

//#include <sstream>
//#include <iomanip>


namespace EWE {

	UIHandler* UIHandler::uiPointer;
	//GLFWwindow* UIHandler::windowPtr:

	UIHandler::UIHandler(std::pair<uint32_t, uint32_t> dimensions, EWEDevice& eweDevice, GLFWwindow* window, TextOverlay* txtOverlay)
		: screenWidth{ static_cast<float>(dimensions.first) }, screenHeight{ static_cast<float>(dimensions.second) }, windowPtr{ window }, textOverlay{ std::shared_ptr<TextOverlay>(txtOverlay) } {
		printf("beg uiHandler construction, dimensions - %.1f:%.1f \n", screenWidth, screenHeight);

		//textOverlay = std::make_unique<TextOverlay>(eweDevice, EWESwapChain->getFrameBuffers(), EWESwapChain->width(), EWESwapChain->height(), EWESwapChain->getRenderPass(), 1.f);


		//MenuModule::changeMenuStateFromMM = changeMenuStateFromMM;
		printf("before sound engine \n");
		soundEngine = SoundEngine::getSoundEngineInstance();
		printf("after sound engine \n");
		/*
		menuMap[menu_main] = std::make_unique<MainMenuMM>(screenWidth, screenHeight);
		printf("after main \n");
		//menuMap[menu_controls] = std::make_unique<ControlsMM>(screenWidth, screenHeight);
		menuMap[menu_controls_temp] = std::make_unique<ControlsTempMM>(screenWidth, screenHeight);
		printf("after controls \n");

		menuMap[menu_audio_settings] = std::make_unique<AudioMM>(screenWidth, screenHeight);
		printf("after audio \n");
		menuMap[menu_graphics_settings] = std::make_unique<GraphicsMM>(screenWidth, screenHeight);
		printf("after graphics \n");
		menuMap[menu_main_play_selection] = std::make_unique<PlaySelectionMM>(screenWidth, screenHeight);
		printf("after main \n");
		menuMap[menu_play] = std::make_unique<GameMenuMM>(screenWidth, screenHeight);
		printf("after main \n");
		menuMap[menu_target_selection] = std::make_unique<TargetSelectionMM>(screenWidth, screenHeight);

		printf("passing device to menu module constructor \n");
		menuMap[menu_level_up] = std::make_unique<LevelUpMM>(eweDevice, screenWidth, screenHeight);
		menuMap[menu_level_fire] = std::make_unique<FireLevelMM>(eweDevice, screenWidth, screenHeight);
		menuMap[menu_level_lightning] = std::make_unique<LightningLevelMM>(eweDevice, screenWidth, screenHeight);
		menuMap[menu_endless_result] = std::make_unique<EndScreenMM>(eweDevice, screenWidth, screenHeight);
		printf("after menu module \n");

#if USING_STEAM
		menuMap[menu_lobby] = std::make_unique<LobbyMM>(screenWidth, screenHeight);
		menuMap[menu_ladder] = std::make_unique<LadderMM>(screenWidth, screenHeight);
		menuMap[menu_target_completion] = std::make_unique<TargetCompletionMM>(screenWidth, screenHeight);
		menuMap[menu_target_leaderboard] = std::make_unique<TargetLeaderboardMM>(screenWidth, screenHeight);
		menuMap[menu_level_builder] = std::make_unique<LevelBuilderMM>(screenWidth, screenHeight, windowPtr);
#endif
*/

		//printf("after imagers in ui handler \n");

		uiPointer = this;
		windowPtr = window;
		//InputHandler::InitializeInputHandler(window);
		//printf("after input handler \n");

		printf("after sound engine construction \n");
		//printf("after sound engine \n");
		//gameUI = std::make_shared<GameUI>();// &menuObjects[MO_GameUI]);
		//printf("before menu objects MO_GameUI \n");
		//printf("after menu objects MO_GameUI \n");
		//gameUI->giveMatchPtr(&matchState);

		backgroundObject.transform2d.scale = glm::vec2{ 2.f };
		backgroundObject.color = { 0.01f, 0.01f, 0.01f };

		
#if USING_STEAM
		netHandler = NetHandler::GetNetHandlerInstance();
		netHandler->inputHandler = InputHandler::GetInputHandlerInstance();
#endif
		//initSoundSettings();
		//menuMap[menu_graphics_settings]->checkBoxes[0].isChecked = SettingsJSON::settingsData.grassEnabled;
		//menuMap[menu_graphics_settings]->checkBoxes[1].isChecked = SettingsJSON::settingsData.pointLights;
		//menuMap[menu_graphics_settings]->checkBoxes[2].isChecked = SettingsJSON::settingsData.renderInfo;

		//printf("before setting my sens in nethandler from uih \n");
		//netHandler->SetMySens(SettingsJSON::settingsData.sensitivity);
		//menuMap[menu_controls]->initSensitivity(SettingsJSON::settingsData.sensitivity, screenWidth, screenHeight);
		//menuMap[menu_main]->labels[1].string = GAME_VERSION;
		//printf("after setting my sens in nethandler from uih \n");
		//printf("file sens? : %u \n", SettingsJSON::settingsData.sensitivity);
		//sliderVector[0].giveSens(SettingsJSON::settingsData.sensitivity);
		//printf("after constructing all boxes \n");

		//for(int i = 0; i < )
		//soundEngine->setVolume(0, SettingsJSON::settingsData.masterVolume);

		printf("end of ui construction \n");
	}

	//std::pair<UIComponentTypes, int8_t> UIHandler::anythingClicked(double xpos, double ypos) {
	//	printf("checking click in : %d \n", currentState);
	//	return menuMap[currentState].checkClick(xpos, ypos);
	//}
	/*
	void UIHandler::changeMenuState(MenuStates newState, unsigned char newGameState) {
		printf("beginning of change menu state \n");
		//std::cout << "newgamestate : " << +newGameState << std::endl;
		if (newGameState != 255) {
			if (newGameState != 0) {
				//printf("newGameState = %u \n", newGameState);
			}
			else {
				//printf("newGameState = %u \n", newGameState);
				glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			gameState = newGameState;
		}
#if USING STEAM
		if (newState == menu_lobby_list) {
			netHandler->RefreshLobbies();
			//gameLobby->getLobbyList();
			while (netHandler->GetRequestingLobbies()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			dynamic_cast<LobbyListMM*>(menuMap[menu_lobby_list].get())->populateList();
			//if greater than 2, that would imply we have lobbies stored in the vector
			//and we want to erase to only get fresh lobbies

			printf("menu lobby list clickable size? : %d \n", menuMap[menu_lobby_list]->clickText.size());
		}
		else if (newState == menu_lobby) {
		
			NetHandler::LobbyStatus tempLobbyStatus = netHandler->getLobbyStatus();
			if (tempLobbyStatus == NetHandler::LS_host) {
				menuMap[menu_lobby]->clickText[0].textStruct.string = "Start";
			}
			else if (tempLobbyStatus == NetHandler::LS_member) {
				menuMap[menu_lobby]->clickText[0].textStruct.string = "Ready";
			}
		}
		else if (newState == 0) {
#else
		if (newState == 0) {
#endif
			glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		currentState = newState;
		printf("end of change menu state \n");
	}
	*/

#if BENCHMARKING
	void UIHandler::Benchmarking(double time, double peakTime, double averageTime, double highTime, double avgLogic, bool benchmarkingGPU, float elapsedGPUMS, float averageGPU) {
		textOverlay->addDefaultText(time, peakTime, averageTime, highTime);
		if (avgLogic > 0.0f) {
			std::stringstream ss;
			ss.str("");
			ss << std::fixed << std::setprecision(4);
			ss << "average Logic Time: " << avgLogic;
			TextStruct passer{ ss.str(), 0.f, screenHeight - (100.f * textOverlay->scale), TA_left, 1.f };
			textOverlay->addText(passer);
			//addText(TextStruct{ ss.str(), 0.f, frameBufferHeight - (100.f * scale), TA_left, 1.f });
		}
		if (benchmarkingGPU) {
			std::stringstream ss;
			ss.str("");
			ss << std::fixed << std::setprecision(4);
			ss << "last GPU Time: " << elapsedGPUMS;
			TextStruct passer{ ss.str(), 0.f, screenHeight - (120.f * textOverlay->scale), TA_left, 1.f };
			textOverlay->addText(passer);
			//addText(TextStruct{ ss.str(), 0.f, frameBufferHeight - (100.f * scale), TA_left, 1.f });

			if (averageGPU > 0.f) {
				std::stringstream ss;
				ss.str("");
				ss << std::fixed << std::setprecision(4);
				ss << "average GPU Time: " << averageGPU;
				TextStruct passer{ ss.str(), 0.f, screenHeight - (140.f * textOverlay->scale), TA_left, 1.f };
				textOverlay->addText(passer);
			}
		}
	}
#endif
	/*
	void UIHandler::drawObstacleUI() {
		std::stringstream ss;
		//printf("draw obstacle ui \n");
		//printf("target count ayo??? %d:%d \n", *activeTargets, *maxTargets);
		ss << std::to_string(*activeTargets) << ":" << std::to_string(*maxTargets);
		//printf("after target count string stream \n");
		obstacleStructs[0].string = ss.str();
		textOverlay->addText(obstacleStructs[0]);
		//ss.clear(); need to figure out how to fully erase ss

		//printf("time elapsed \n");
		std::stringstream ss2;
		ss2 << std::fixed << std::setprecision(2) << timeElapsed;
		obstacleStructs[1].string = ss2.str();
		textOverlay->addText(obstacleStructs[1]);

		if (*activeTargets == 0) {
			//printf("conclusion? \n");
			std::stringstream ss3;
			ss3 << "Congratulations : " << std::fixed << std::setprecision(2) << obstacleStartTime - timeElapsed;
			obstacleStructs[2].string = ss3.str();
			textOverlay->addText(obstacleStructs[2]);
		}
		//printf("returning draw targets \n");
	}
	*/


	void UIHandler::drawMenuMain(VkCommandBuffer commandBuffer, bool displayingRenderInfo) {

		if (overlay) {
			overlay->drawText();
		}

		if (isActive) {
#if EIGHT_WINDS_GAME
			if (currentState == menu_lobby) {
				//menuStructs[menu_lobby].first[0].string = netHandler->GetLobbyName();
				menuMap[menu_lobby]->labels[0].string = netHandler->GetLobbyName();
				for (int i = 0; i < netHandler->GetLobbyPersonaNames().size(); i++) {
					std::string passString = netHandler->GetLobbyPersonaNames()[i];
					TextStruct passTextStruct{ passString, 400.f, 240.f + i * 40.f, TA_left, 2.f };
					textOverlay->addText(passTextStruct);
					
					int8_t memberStatus = netHandler->GetLobbyMemberReady(i);
					if (memberStatus == 1) {
						std::string passString2 = "Ready";

						TextStruct passTextStruct2{ passString2, 360.f, 240.f + i * 40.f, TA_right, 2.f };
						textOverlay->addText(passTextStruct2);
					}
					else if(memberStatus == 0)  {
						std::string passString2 = "Not Ready";
						TextStruct passTextStruct2{ passString2, 360.f, 240.f + i * 40.f, TA_right, 2.f };
						textOverlay->addText(passTextStruct2);
					}
					else if (memberStatus == 2) {
						std::string passString2 = "Host";
						TextStruct passTextStruct2{ passString2, 360.f, 240.f + i * 40.f, TA_right, 2.f };
						textOverlay->addText(passTextStruct2);
					}
					//textOverlay->addText(passString, 100.f, 240.f + i * 20.f, TA_left, 2.f);
				}
			}
			else if (currentState == menu_target_leaderboard) {
				uint8_t leaderboardRefreshed = netHandler->playfabClass->getLeaderboardRefreshed();
				if (leaderboardRefreshed > 0) {
					menuMap[menu_target_leaderboard]->labels.erase(menuMap[menu_target_leaderboard]->labels.begin() + 2, menuMap[menu_target_leaderboard]->labels.end());
					auto mountainLeaderboard = netHandler->playfabClass->getMountainLeaderboard();
					auto straightAwayLeaderboard = netHandler->playfabClass->getStraightAwayLeaderboard();
					printf("updating leaderboard in uihandler.cpp, sizes - %d:%d \n", mountainLeaderboard.size(), straightAwayLeaderboard.size());
					for (int i = 0; i < mountainLeaderboard.size(); i++) {
						menuMap[menu_target_leaderboard]->labels.emplace_back(mountainLeaderboard[i].first, screenWidth * .2f, 230.f * screenHeight / DEFAULT_HEIGHT + i * 30.f, TA_right, 1.5f);
						menuMap[menu_target_leaderboard]->labels.emplace_back(mountainLeaderboard[i].second, screenWidth * .2f, 230.f * screenHeight / DEFAULT_HEIGHT + i * 30.f, TA_left, 1.5f);
					}

					for (int i = 0; i < straightAwayLeaderboard.size(); i++) {
						menuMap[menu_target_leaderboard]->labels.emplace_back(straightAwayLeaderboard[i].first, screenWidth * .8f, 230.f * screenHeight / DEFAULT_HEIGHT + i * 30.f, TA_right, 1.5f);
						menuMap[menu_target_leaderboard]->labels.emplace_back(straightAwayLeaderboard[i].second, screenWidth * .8f, 230.f * screenHeight / DEFAULT_HEIGHT + i * 30.f, TA_left, 1.5f);
					}
				}
			}
#endif
			//printf("before drawing current state menu? \n");
			//printf("after drawing current state menu \n");
		}

#if EIGHT_WINDS_GAME
		if (netHandler->LeavingLobby()) {
			changeMenuState(menu_main_play_selection);
		}
#endif
		//printf("returning draw menu main \n");
	}

	void UIHandler::type(int keyCode) {
		//std::cout << "typing? " << std::endl;
		if (keyCode == GLFW_KEY_ENTER) {
			selectedKey = -1;
			textSent = true;
		}
		else if (keyCode != GLFW_KEY_BACKSPACE) { typedString += glfwGetKeyName(keyCode, 0); }
		else if (typedString.size() > 0) {
			typedString.pop_back();
		}
	}

}