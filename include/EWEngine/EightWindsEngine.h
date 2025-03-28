#pragma once

#include "EWEngine/MainWindow.h"
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Renderer.h"
#include "EWEngine/Graphics/Descriptors.h"

#include "EWEngine/Systems/Rendering/Skin/SkinRS.h"

#include "EWEngine/Systems/Rendering/advanced_render_system.h"
//#include "LevelBuilder/LevelBuilder.h"
#include "EWEngine/GUI/UIHandler.h"
//#include "EWEngine/graphicsimGuiHandler.h"
#include "EWEngine/LoadingScreen/LeafSystem.h"
#include "EWEngine/GUI/MenuManager.h"
#include "EWEngine/Systems/PipelineSystem.h"

#include "EWEngine/Graphics/LightBufferObject.h"

#include "EWEngine/Graphics/Texture/Sampler.h"

#include <functional>
#include <memory>
#include <vector>
#include <chrono>

#define RENDER_TIME 0.0069444444f
#define LOGIC_TIME 0.0040
//#define LOGIC_TIME 0.0001

#define BENCHMARKING_GPU true



namespace EWE {
	class EightWindsEngine {
	public:

		//i want to manually control all construciton here
		EightWindsEngine(std::string windowName);
		//fourth will be class member construciton without brackets

		~EightWindsEngine();

		EightWindsEngine(const EightWindsEngine&) = delete;
		EightWindsEngine& operator=(const EightWindsEngine&) = delete;

		//uint64_t boneBufferSize;
		float elapsedGPUMS;
		float averageElapsedGPUMS = 0.f;
		float totalElapsedGPUMS = 0.f;
		uint32_t averageElapsedGPUCounter = 0;

		MainWindow mainWindow; //first
		EWEDevice eweDevice; //second
		EWECamera camera{};
		EWERenderer eweRenderer; //third
		//ComputeHandler computeHandler; //4th???

		LeafSystem* leafSystem;

		std::unique_ptr<std::thread> logicThread;
		std::unique_ptr<std::thread> renderThread;


#if BENCHMARKING_GPU
		VkQueryPool queryPool[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
		float gpuTicksPerSecond = 0;
#endif
		UIHandler uiHandler;

		AdvancedRenderSystem advancedRS;

		Image_Manager imageManager;
		MenuManager menuManager;
		SkinRenderSystem skinnedRS;

		double timeTracker = 0.0f;

		uint32_t beginRoundFrames = 0; //move this out

		TransformComponent viewerTransform{};
		LightBufferObject lbo;

		//std::chrono::high_resolution_clock::time_point currentTime;// = std::chrono::high_resolution_clock::now();

		//LARGE_INTEGER frequency;
		//LARGE_INTEGER QPCstart, QPCend;

		double totalLogicTime = 0.0f;
		double averageLogicTime = 0.f;
		int logicFramesCounted = 0;
		float highestLogicTime = 0.0f;
		double highestRenderTime = 0.0;

		double totalRenderTime = 0.0;
		double averageRenderTime = 0.0;
		int renderFramesCounted = 0;
		double peakRenderTime = 0.0;
		double minRenderTime = 100.0;

		double renderFPS = 1.0 / 144.0;
		bool pointLightsEnabled = false;
		bool displayingRenderInfo = false;
		bool timestampsAvailable = false;

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		void InitGlobalBuffers();

		//this is changed just so that it doesnt overlap with the old name and i know to change it
		void BeginRenderX();

		bool BeginFrame();

		bool BeginFrameAndRender();
//#define RENDER_OBJECT_DEBUG

		void Draw2DObjects();
		void Draw3DObjects(double dt);
		void DrawText(double dt);
		void DrawObjects(double dt);
		void Render2D(bool menuActive);

		void EndRender();
		void EndFrame();

		void LoadingScreen();

		void FinishLoading();

		void EndEngineLoadScreen();
		bool GetLoadingScreenProgress() {
			return (!finishedLoadingScreen);// || (loadingTime < 2.0);
		}

	private:
		bool finishedLoadingScreen = false;
		bool loadingEngine = true;
		double loadingTime = 0.f;

#if BENCHMARKING_GPU
		void QueryTimestampBegin();
		void QueryTimestampEnd();
		void CreateQueryPool();
		bool previouslySubmitted[MAX_FRAMES_IN_FLIGHT] = { false, false };

		struct TimestampData {
			uint64_t result;
			uint64_t availability;
		};
		TimestampData timestamps[4];
#endif


	};
}

