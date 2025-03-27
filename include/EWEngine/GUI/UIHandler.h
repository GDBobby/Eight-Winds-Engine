#pragma once

#include "EWEngine/Graphics/TextOverlay.h"
#include "EWEngine/Sound_Engine.h"
//#include "GameUI.h"
#include "Overlay.h"

#define BENCHMARKING true

namespace EWE{
	class UIHandler {
	private:
	public:
		bool resyncingNetPlay = false;

		UIHandler(SettingsInfo::ScreenDimensions dimensions, GLFWwindow* window, TextOverlay* txtOverlay);

		~UIHandler() {
#if DECONSTRUCTION_DEBUG
			printf("beg of uihandler deconstructor \n");
#endif
			Deconstruct(textOverlay);
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
		void DrawOverlayText(bool displayingRenderInfo);

		void BeginTextRender() {
			textOverlay->BeginTextUpdate();
		}
		void EndTextRender() {
			textOverlay->EndTextUpdate();
		}

		uint32_t* activeTargets = 0;
		uint32_t* maxTargets = 0;

		GameObject2D backgroundObject{};

		TextOverlay* GetTextOverlay() {
			//throw std::exception("only copy this once, to MenuManager");
			return textOverlay;
		}

	private:
		TextOverlay* textOverlay;
		//float timeElapsed = 0.0f;


		std::shared_ptr<SoundEngine> soundEngine;

		std::shared_ptr<EWEModel> EWEModel{};

	};
}