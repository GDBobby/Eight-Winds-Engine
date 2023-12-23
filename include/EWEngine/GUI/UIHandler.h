#pragma once

#include "EWEngine/graphics/TextOverlay.h"
#include "EWEngine/SoundEngine.h"
#include "EWEngine/graphics/EWE_texture.h"
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


		std::shared_ptr<OverlayBase> overlay;

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

		std::shared_ptr<EWEModel> EWEModel{};

	};
}