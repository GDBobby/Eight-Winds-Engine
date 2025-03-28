#include "EWEngine/GUI/MenuManager.h"

namespace EWE {
	MenuManager* MenuManager::menuManagerPtr = nullptr;

	MenuManager::MenuManager(GLFWwindow* windowPtr, TextOverlay* textOverlay) : windowPtr{ windowPtr }, textOverlay{ textOverlay } {
		assert(menuManagerPtr == nullptr && "created two menu managers?");
#if EWE_DEBUG
		printf("beginning menu manager construction\n");
#endif

		menuManagerPtr = this;
		currentScene = 0;

		MenuModule::initTextures();

		menuModules.try_emplace(menu_audio_settings, std::make_unique<AudioMM>(windowPtr));
		menuModules.try_emplace(menu_graphics_settings, std::make_unique<GraphicsMM>());


		MenuModule::ChangeMenuStateFromMM = ChangeMenuStateFromMM;

	}
	void MenuManager::WindowResize(SettingsInfo::ScreenDimensions windowDim) {
		menuManagerPtr->windowWasResized = true;
		const float nextWidth = static_cast<float>(windowDim.width);
		const float nextHeight = static_cast<float>(windowDim.height);
		glm::vec2 resizeRatio{
			nextWidth / VK::Object->screenWidth,
			nextHeight / VK::Object->screenHeight
		};
		VK::Object->screenWidth = nextWidth;
		VK::Object->screenHeight = nextHeight;

		menuManagerPtr->textOverlay->WindowResize();

		for (auto iter = menuManagerPtr->menuModules.begin(); iter != menuManagerPtr->menuModules.end(); iter++) {
			iter->second->ResizeWindow(resizeRatio);
		}
	}
	void MenuManager::drawNewMenuObejcts() {
		if (isActive) {
			//printf("Drawing menu \n");
			menuModules.at(currentMenuState)->DrawNewObjects();//(gameState == 0));
		}
	}

	void MenuManager::SetCurrentMenu(uint8_t currentMenu) {
		assert(menuModules.contains(currentMenu));
		currentMenuState = currentMenu;
		currentModule = menuModules.at(currentMenuState).get();
	}
	void MenuManager::ChangeMenuState(uint8_t nextMenu, uint8_t nextScene) { //nextScene is really just turning on mouse or not
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
		currentModule = menuModules.at(currentMenuState).get();
		printf("end of change menu state \n");
	}

	void MenuManager::staticMouseCallback(GLFWwindow* window, int button, int action, int mods) {
		//if ((menuManagerPtr->selectedKey >= 0) && (action == 1)) {
			//menuManagerPtr->setKey(button);
			//return;
		//}

		if ((menuManagerPtr->isActive) && (button == GLFW_MOUSE_BUTTON_1) && (action == GLFW_PRESS)) {
			double xpos = 0;
			double ypos = 0;
			glfwGetCursorPos(window, &xpos, &ypos);
			printf("%.0f, %.0f  ~ mouse coords \n", xpos, ypos);
			//menuManagerPtr->lastClicked = menuManagerPtr->anythingClicked(xpos, ypos);
			//printf("last clicked pair - %d:%d \n", menuManagerPtr->lastClicked.first, menuManagerPtr->lastClicked.second);
			
			menuManagerPtr->menuModules.at(menuManagerPtr->currentMenuState)->ProcessClick(xpos, ypos);
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
			//MenuModule::clickReturns.push(MCR_EscapePressed);
			menuManagerPtr->escapeCallback();
			//menuManagerPtr->escapeMenu();
			//menuManagerPtr->inputHandler->returnFocus();
		}

	}
}