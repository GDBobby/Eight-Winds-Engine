#pragma once

#include "../Graphics/TextOverlay.h"
#include "../SoundEngine.h"
#include "../Graphics/EWE_texture.h"
//#include "GameUI.h"
#include "Overlay.h"

#define BENCHMARKING true

namespace EWE{
	class UIHandler {
	private:
	public:
		bool resyncingNetPlay = false;

		UIHandler(std::pair<uint32_t, uint32_t> dimensions, EWEDevice& eweDevice, GLFWwindow* window, TextOverlay* txtOverlay);

		~UIHandler() {
#if DECONSTRUCTION_DEBUG
			printf("beg of uihandler deconstructor \n");
#endif
			textOverlay.reset();
#if DECONSTRUCTION_DEBUG
			printf("uihandler deconstructor \n");
#endif
		}

#if USING_STEAM
		void LeavingLobby() {
			dynamic_cast<LobbyMM*>(menuMap[menu_lobby_list].get())->playerNames.first = "Player 1";
			dynamic_cast<LobbyMM*>(menuMap[menu_lobby_list].get())->playerNames.second = "Player 2";
		}
#endif
		/*
		bool ResetRound(uint8_t playerLocalIndex) {
			switch (roundState) {
				//do match state here
				case 0: {
					//player 0 won
#if USING_STEAM
					netHandler->LobbyAddKill(playerLocalIndex);
#endif
					gameUI->addKill(playerLocalIndex);
					break;
				}
				case 1: {
#if USING_STEAM
					netHandler->LobbyAddKill(1 - playerLocalIndex);
#endif
					gameUI->addKill(1 - playerLocalIndex);
					break;
				}
			}
			timeElapsed = 100.f;
			if (matchState >= 0) {
				printf("the game is over in uihandler \n");
				return true;
			}
			return false;
		}
		std::shared_ptr<GameUI> gameUI;
		*/
		//void ObstacleReset() { timeElapsed = obstacleStartTime; }

		std::shared_ptr<OverlayBase> overlay;
		/*
		bool windowWasResized = false;

		void windowResize(std::pair<uint32_t, uint32_t> windowDim) {
			windowWasResized = true;
			float rszWidth = static_cast<float>(windowDim.first);
			float rszHeight = static_cast<float>(windowDim.second);
			textOverlay->windowResize(rszWidth, rszHeight);

			for (auto iter = menuMap.begin(); iter != menuMap.end(); iter++) {
				iter->second->resizeWindow(rszWidth, screenWidth, rszHeight, screenHeight);
			}
			screenWidth = rszWidth;
			screenHeight = rszHeight;
		}
		*/

		static UIHandler* uiPointer;
		GLFWwindow* windowPtr;



		int16_t selectedKey = -1;
		//void setKey(int keyCode);
		void type(int keyCode);
		std::string typedString;
		bool textSent = false;

		bool isActive = true;
		bool escapePressed = false;

		void Benchmarking(double time, double peakTime, double averageTime, double highTime, double avgLogic, bool benchmarkingGPU, float elapsedGPUMS, float averageGPU);
		void drawMenuMain(VkCommandBuffer commandBuffer, bool displayingRenderInfo);

		void beginTextRender() {
			textOverlay->beginTextUpdate();
		}
		void endTextRender(VkCommandBuffer cmdBuf) {
			textOverlay->endTextUpdate();
			textOverlay->draw(cmdBuf);
		}

		unsigned int* activeTargets = 0;
		unsigned int* maxTargets = 0;

		GameObject2D backgroundObject{ GameObject2D::createGameObject() };
		//std::vector<std::vector<GameObject2D>>& getGameObjects() { return menuObjects; }

#if USING_STEAM
		std::shared_ptr<NetHandler> netHandler;
#endif
		/*
		void takeMySensPtr(uint8_t* mySensPtr) {
			mySens = mySensPtr;
			//*mySens = sliderVector[0].mySens;
			*mySens = SettingsJSON::settingsData.sensitivity;
			
			//dynamic_cast<ControlsMM*>(menuMap[menu_controls].get())->takeSensPtr(mySens);
		}
		*/

		//std::map<MenuStates, std::unique_ptr<MenuModule>> menuMap;



		float getScreenWidth() {
			return screenWidth;
		}
		float getScreenHeight() {
			return screenHeight;
		}

		std::shared_ptr<TextOverlay> getTextOverlay() {
			//static uint8_t returnCount = 0;
			//returnCount++;
			//if (returnCount > 1) {
				//im sure there's a better way to do this?
				//throw std::exception("only copy this once, to MenuManager");
			//}
			return textOverlay;
		}

	private:
		std::shared_ptr<TextOverlay> textOverlay;
		enum menu_objects {
			combo_objects,
			uiobjects,
			slider_object,
			sbracket_object,
		};
		//float timeElapsed = 0.0f;

		float screenWidth;
		float screenHeight;

		std::shared_ptr<SoundEngine> soundEngine;

		//std::shared_ptr<InputHandler> inputHandler;

		//uint8_t* mySens{ nullptr };
		//std::list<NetHandler::Lobby_t>* uiLobbyList;

		std::shared_ptr<EWEModel> EWEModel{};

	};
}