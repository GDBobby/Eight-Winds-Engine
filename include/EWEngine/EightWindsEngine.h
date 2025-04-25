#pragma once

#include "EWEngine/Graphics/RenderFramework.h"

#include "EWEngine/MainWindow.h"
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Renderer.h"
#include "EWEngine/Graphics/Descriptors.h"

#include "EWEngine/Systems/Rendering/advanced_render_system.h"
//#include "LevelBuilder/LevelBuilder.h"
#include "EWEngine/GUI/UIHandler.h"
//#include "EWEngine/graphicsimGuiHandler.h"
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

#define BENCHMARKING_GPU true

namespace EWE {
	using CS = lab::CoordinateSystem<lab::Direction::ZDir<true>, lab::Direction::XDir<true>, lab::Direction::YDir<true>>;

	class EightWindsEngine {
	public:
		EightWindsEngine(std::string windowName);

		~EightWindsEngine();

		EightWindsEngine(const EightWindsEngine&) = delete;
		EightWindsEngine& operator=(const EightWindsEngine&) = delete;

		RenderFramework renderFramework;


		EWECamera camera{};
		UIHandler uiHandler;
		AdvancedRenderSystem advancedRS;
		MenuManager menuManager;
		SkinRenderSystem skinnedRS;

		float elapsedGPUMS;
		float averageElapsedGPUMS = 0.f;
		float totalElapsedGPUMS = 0.f;
		uint32_t averageElapsedGPUCounter = 0;
#if BENCHMARKING_GPU
		VkQueryPool queryPool[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
		float gpuTicksPerSecond = 0;
#endif

		double timeTracker = 0.0f;

		lab::Transform<float, 3> viewerTransform{};
		LightBufferObject lbo;

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

		void FinishLoading();

	private:

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

