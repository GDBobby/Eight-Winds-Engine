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
		std::shared_ptr<TextOverlay> textOverlay;
	public:

		std::unordered_map<uint16_t, std::unique_ptr<MenuModule>> menuModules;
		float screenWidth, screenHeight;
		GLFWwindow* windowPtr;
		uint8_t* currentScene;
		//std::queue<MenuClickReturn> clickReturns;
		uint8_t currentMenuState = 0;

		MenuManager(float screenWidth, float screenHeight, EWEDevice& eweDevice, GLFWwindow* windowPtr, std::shared_ptr<TextOverlay> textOverlay);
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
			glfwSetCursorPosCallback(windowPtr, nullptr);
		}
		void escapeMenu() {
			isActive = false;
			escapePressed = true;
			glfwSetCursorPosCallback(windowPtr, nullptr);
		}

		std::pair<float, float> getScreenDimensions() {
			return { screenWidth, screenHeight };
		}

		void changeMenuState(uint8_t newState, unsigned char newGameState = 255);
		static void changeMenuStateFromMM(uint8_t newState, unsigned char newGameState = 255) {
			menuManagerPtr->changeMenuState(newState, newGameState);
		}
		void giveMenuFocus() {
			//glfwFocusWindow(windowPtr);

			glfwSetKeyCallback(windowPtr, staticKeyCallback);
			glfwSetMouseButtonCallback(windowPtr, staticMouseCallback);

			glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			isActive = true;
		}

		static void windowResize(std::pair<uint32_t, uint32_t> windowDim);

		void drawMenuObjects(VkCommandBuffer cmdBuf, uint8_t frameIndex);

		bool getMenuActive() {
			return isActive;
		}

		void drawText() {
			if (isActive) {
				menuModules[currentMenuState]->drawText(textOverlay.get());
			}
		}
		bool drawingNineUI() { return menuModules[currentMenuState]->drawingNineUI(); }
		void drawNineUI(VkCommandBuffer cmdBuf, uint8_t frameIndex) { menuModules[currentMenuState]->drawNineUI(cmdBuf, frameIndex); }

		static void staticKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void staticMouseCallback(GLFWwindow* window, int button, int action, int mods);
	};
}