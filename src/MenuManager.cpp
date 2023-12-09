#include "GUI/MenuManager.h"

namespace EWE {
	MenuManager* MenuManager::menuManagerPtr = nullptr;

	MenuManager::MenuManager(float screenWidth, float screenHeight, EWEDevice& eweDevice, GLFWwindow* windowPtr, std::shared_ptr<TextOverlay> textOverlay) : windowPtr{ windowPtr }, textOverlay{ textOverlay }, screenWidth{ screenWidth }, screenHeight{ screenHeight } {
		if (menuManagerPtr != nullptr) {
			printf("created two menu managers? \n");
			throw std::exception("created two menu managers?");
		}
		menuManagerPtr = this;
		currentScene = 0;
		//menuModules[menu_controls_temp] = std::make_unique<ControlsTempMM>(screenWidth, screenHeight);
		MenuModule::initTextures(eweDevice);

		//MenuModule::changeMenuStateFromMM = changeMenuStateFromMM;

		printf("passing device to menu module constructor \n");		
		menuModules[menu_audio_settings] = std::make_unique<AudioMM>(screenWidth, screenHeight, windowPtr);
		printf("after audio \n");
		menuModules[menu_graphics_settings] = std::make_unique<GraphicsMM>(screenWidth, screenHeight);
		printf("after graphics \n");


		MenuModule::changeMenuStateFromMM = changeMenuStateFromMM;
		//menuRef.clickTextCallback.push_back(&MenuManager::DiscardReturnCallback);

	}
	void MenuManager::windowResize(std::pair<uint32_t, uint32_t> windowDim) {
		menuManagerPtr->windowWasResized = true;
		float rszWidth = static_cast<float>(windowDim.first);
		float rszHeight = static_cast<float>(windowDim.second);
		menuManagerPtr->textOverlay->windowResize(rszWidth, rszHeight);

		for (auto iter = menuManagerPtr->menuModules.begin(); iter != menuManagerPtr->menuModules.end(); iter++) {
			iter->second->resizeWindow(rszWidth, menuManagerPtr->screenWidth, rszHeight, menuManagerPtr->screenHeight);
		}
		menuManagerPtr->screenWidth = rszWidth;
		menuManagerPtr->screenHeight = rszHeight;
	}
	void MenuManager::drawMenuObjects(VkCommandBuffer cmdBuf, uint8_t frameIndex) {
		//printf("draw menu objects in uihandler \n");
		if (isActive) {
			//printf("Drawing menu \n");
			menuModules[currentMenuState]->drawObjects(cmdBuf, frameIndex, false);//(gameState == 0));
		}

	}

	void MenuManager::changeMenuState(uint8_t nextMenu, unsigned char nextScene) { //nextScene is really just turning on mouse or not
		printf("beginning of change menu state \n");
		//std::cout << "newcurrentScene : " << +newcurrentScene << std::endl;
		if (nextScene != 255) {
			if (nextScene != 0) {
				//printf("new*currentScene = %u \n", newcurrentScene);
			}
			else {
				//printf("new*currentScene = %u \n", newcurrentScene);
				glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			whichScene = nextScene;
		}
		if (nextMenu == 0) {
			glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		currentMenuState = nextMenu;
		printf("end of change menu state \n");
	}

	void MenuManager::staticMouseCallback(GLFWwindow* window, int button, int action, int mods) {
		//if ((menuManagerPtr->selectedKey >= 0) && (action == 1)) {
			//menuManagerPtr->setKey(button);
			//return;
		//}

		if ((button == GLFW_MOUSE_BUTTON_1) && (action == GLFW_PRESS)) {
			double xpos = 0;
			double ypos = 0;
			glfwGetCursorPos(window, &xpos, &ypos);
			printf("%.0f, %.0f  ~ mouse coords \n", xpos, ypos);
			//menuManagerPtr->lastClicked = menuManagerPtr->anythingClicked(xpos, ypos);
			//printf("last clicked pair - %d:%d \n", menuManagerPtr->lastClicked.first, menuManagerPtr->lastClicked.second);

			menuManagerPtr->menuModules[menuManagerPtr->currentMenuState]->processClick(xpos, ypos);
			//menuManagerPtr->clickReturns.emplace(menuManagerPtr->menuModules[menuManagerPtr->currentMenuState]->processClick(xpos, ypos));
			 
			if (MenuModule::clickReturns.size() > 0) {
				printf("clickReturns front : %d \n", MenuModule::clickReturns.front());
			}
		}
		/*
		else if ((button == GLFW_MOUSE_BUTTON_1) && (action == GLFW_RELEASE) && (menuManagerPtr->callbackSlider != -1)) {
			if (menuManagerPtr->currentState == menu_controls) {
				SettingsJSON::tempSettings.sensitivity = menuManagerPtr->menuMap[menu_controls]->sliders[0].mySens;
			}
			if (menuManagerPtr->currentMenuState == menu_audio_settings) {
				//menuManagerPtr->soundEngine->setVolume(menuManagerPtr->grabbedSlider - 1, menuManagerPtr->sliderVector[menuManagerPtr->grabbedSlider].slidePosition);
				printf("before setting volume on slider release, grabbed slider : %d  \n", menuManagerPtr->callbackSlider);
				SettingsJSON::tempSettings.setVolume(menuManagerPtr->callbackSlider, menuManagerPtr->menuMap[menu_audio_settings]->sliders[menuManagerPtr->callbackSlider].slidePosition);
				printf("after setting volume on slider rlease \n");
			}
			menuManagerPtr->callbackSlider = -1;
		}
		*/
	}

	void MenuManager::staticKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		/* this was largely replaced with new boxes, that I haven't implemented yet. I don't think I currently have anything typeable
		if ((menuManagerPtr->selectedKey == -2) && (action == 1)) {
			menuManagerPtr->type(key);
		}
		*/
		if ((key == GLFW_KEY_ESCAPE) && (action == 1)) {// && ((*menuManagerPtr->currentScene == scene_game) || (*menuManagerPtr->currentScene == scene_target))) {
			std::cout << "pressed escape while in menu" << std::endl;
			//menuManagerPtr->isActive = false;
			//menuManagerPtr->escapePressed = true;
			//glfwSetCursorPosCallback(menuManagerPtr->windowPtr, nullptr);
			MenuModule::clickReturns.push(MCR_EscapePressed);
			//menuManagerPtr->inputHandler->returnFocus();
		}

	}
}