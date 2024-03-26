#pragma once

#include "EWEngine/MainWindow.h"
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Renderer.h"
#include "EWEngine/Graphics/Descriptors.h"

#include "EWEngine/Systems/Rendering/Skin/SkinRS.h"

#include "EWEngine/Systems/Rendering/advanced_render_system.h"
#include "EWEngine/ObjectManager.h"
//#include "LevelBuilder/LevelBuilder.h"
#include "EWEngine/GUI/UIHandler.h"
//#include "EWEngine/graphicsimGuiHandler.h"
#include "EWEngine/LoadingScreen/LeafSystem.h"
#include "EWEngine/GUI/MenuManager.h"
#include "EWEngine/Systems/PipelineSystem.h"

#include "EWEngine/Graphics/LightBufferObject.h"

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
		float elapsedGPUMS = 0.f;
		float averageElapsedGPUMS = 0.f;
		float totalElapsedGPUMS = 0.f;
		uint32_t averageElapsedGPUCounter = 0;

		MainWindow mainWindow; //first
		EWEDevice eweDevice; //second
		EWERenderer eweRenderer; //third
		//ComputeHandler computeHandler; //4th???

		double renderFPS = 1.0 / 144.0;
		bool pointLightsEnabled = false;
		bool displayingRenderInfo = false;

		AdvancedRenderSystem advancedRS;
		SkinRenderSystem skinnedRS;
		Texture_Manager textureManager;

		LeafSystem* leafSystem;

		std::unique_ptr<std::thread> logicThread;
		std::unique_ptr<std::thread> renderThread;


#if BENCHMARKING_GPU
		VkQueryPool queryPool = VK_NULL_HANDLE;
		float gpuTicksPerSecond = 0;
#endif

		ObjectManager objectManager;
		UIHandler uiHandler;
		MenuManager menuManager;

		double timeTracker = 0.0f;

		uint32_t beginRoundFrames = 0; //move this out

		std::unordered_map<Buffer_Enum, std::vector<EWEBuffer*>> bufferMap;

		EWECamera camera;
		EWEGameObject viewerObject{ EWEGameObject::createGameObject() };
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

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		void initGlobalBuffers();

		FrameInfo beginRender();
//#define RENDER_OBJECT_DEBUG

		void draw2DObjects(FrameInfo& frameInfo);
		void draw3DObjects(FrameInfo& frameInfo, double dt);
		void drawText(FrameInfo& frameInfo, double dt);
		void drawObjects(FrameInfo& frameInfo, double dt);

		void endRender(FrameInfo& frameInfo);

		void loadingScreen();

		void finishLoading();

		void endEngineLoadScreen() {
			printf("~~~~ ENDING LOADING SCREEN ~~~ \n");
			loadingEngine = false;
		}
		bool getLoadingScreenProgress() {
			return (!finishedLoadingScreen) || (loadingTime < 2.0);
		}
		//bool endlessPaused = false;
	private:
		bool finishedLoadingScreen = false;
		bool loadingEngine = true;
		double loadingTime = 0.f;
	};
}

