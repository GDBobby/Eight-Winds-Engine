#pragma once
#include "MenuModule.h"

#include "MenuModule.h"


//engine modules
#include "AudioMM.h"
#include "GraphicsMM.h"

namespace EWE {

	class MenuManager {
	private:
		static MenuManager* menuManagerPtr;
		bool isActive = true;
		bool escapePressed = false;
		bool windowWasResized = false;
		TextOverlay* textOverlay;
	public:
		void SetCurrentMenu(uint8_t currentState);
		std::unordered_map<uint16_t, std::unique_ptr<MenuModule>> menuModules;
		MenuModule* currentModule = nullptr;
		GLFWwindow* windowPtr;
		uint8_t* currentScene;
		//std::queue<MenuClickReturn> clickReturns;
		uint8_t currentMenuState = 0;

		MenuManager(GLFWwindow* windowPtr, TextOverlay* textOverlay);
		int8_t whichScene = -1;
		/*
		static void DiscardReturnCallback() {
			if (menuManagerPtr->whichScene == scene_mainmenu) {
				printf("pre main menu change menu state \n");
				menuManagerPtr->changeMenuState(menu_main);
			}
			else {// || (gameState == scene_endless)) {
				menuManagerPtr->changeMenuState(menu_play);
			}
		}
		*/
		void closeMenu() {
			isActive = false;
			//glfwSetCursorPosCallback(windowPtr, nullptr);
		}
		void escapeMenu() {
			isActive = false;
			escapePressed = true;
			//glfwSetCursorPosCallback(windowPtr, nullptr);
		}

		void ChangeMenuState(uint8_t newState, uint8_t newGameState = 255);
		static void ChangeMenuStateFromMM(uint8_t newState, unsigned char newGameState = 255) {
			menuManagerPtr->ChangeMenuState(newState, newGameState);
		}
		void giveMenuFocus() {
			//glfwFocusWindow(windowPtr);

			glfwSetKeyCallback(windowPtr, staticKeyCallback);
			glfwSetMouseButtonCallback(windowPtr, staticMouseCallback);

			glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			isActive = true;
		}

		static void WindowResize(SettingsInfo::ScreenDimensions windowDim);

		void drawNewMenuObejcts();
		//void drawMenuObjects(FrameInfo& frameInfo, bool menuActive);
		void drawNewNine() { if (isActive) { currentModule->DrawNewNine(); } }
		bool DrawingImages() { return currentModule->images.size() > 0; }
		void DrawImages() { currentModule->DrawImages(); }

		bool getMenuActive() const {
			return isActive;
		}

		void drawText() {
			if (isActive) {
				currentModule->DrawText();
			}
		}

		static void staticKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void staticMouseCallback(GLFWwindow* window, int button, int action, int mods);

		std::function<void()> escapeCallback{ nullptr };
	};
}