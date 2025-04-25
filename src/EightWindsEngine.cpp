#include "EWEngine/EightWindsEngine.h"

//#include "keyboard_movement_controller.h" //this is for a free camera, which is currently not utilized
#include "EWEngine/Graphics/Device_Buffer.h"
#include "EWEngine/Graphics/Camera.h"
#include "EWEngine/Systems/Rendering/Pipelines/Dimension2.h"
#include "EWEngine/Systems/Rendering/Pipelines/Pipe_Skybox.h"

#include "EWEngine/Systems/Rendering/Rigid/RigidRS.h"

//#include <array>
//#include <chrono>
#include <stdexcept>
#include <iostream>
#include <cmath>
#include <thread>
#include <future>

#include <chrono>

#define ARENA_MAP true
#define GRASS_MAP false

#define ENGINE_VERSION "1.0.0.0"

#define RENDER_DEBUG false


namespace EWE {


	inline void printmat4(lab::mat4& theMatrix, const std::string& matrixName) {
		printf("matrix values : %s\n", matrixName.c_str());
		for(uint8_t i = 0; i < 4; i++) {
			printf("\t%.3f:%.3f:%.3f:%.3f\n", theMatrix.columns[i].x, theMatrix.columns[i].y, theMatrix.columns[i].z, theMatrix.columns[i].w);
		}
	}


	EightWindsEngine::EightWindsEngine(std::string windowName) :
		//first, any members not mentioned here with brackets will be initialized
		//second, any memberss in this section will be initialized
		renderFramework{windowName},
		//imguiHandler{ mainWindow.getGLFWwindow(), MAX_FRAMES_IN_FLIGHT, eweRenderer.getSwapChainRenderPass() },
		uiHandler{ SettingsJSON::settingsData.screenDimensions, renderFramework.mainWindow.getGLFWwindow(), renderFramework.eweRenderer.MakeTextOverlay() },
		advancedRS{ menuManager },
		menuManager{ renderFramework.mainWindow.getGLFWwindow(), uiHandler.GetTextOverlay()},
		skinnedRS{ }
	{
		printf("after finishing construction of engine\n");
		EWEPipeline::PipelineConfigInfo::pipelineRenderingInfoStatic = renderFramework.eweRenderer.getPipelineInfo();

		printf("eight winds constructor, ENGINE_VERSION: %s \n", ENGINE_VERSION);
		camera.SetPerspectiveProjection(lab::DegreesToRadians(70.0f), renderFramework.eweRenderer.GetAspectRatio(), 0.1f, 10000.0f);

		viewerTransform.translation = { -20.f, 21.f, -20.f };
		camera.UpdateViewData(viewerTransform.translation, { 0.f, 19.5f, 0.f });

		//prettify this later
		lbo.ambientColor = { 0.1f, 0.1f, 0.1f, 0.1f }; //w is alignment only?
		lbo.sunlightDirection = { 1.f, 3.f, 1.f, 0.f };
		lbo.sunlightDirection.Normalize();
		lbo.sunlightColor = { 0.8f,0.8f, 0.8f, 0.5f };

		DescriptorHandler::InitGlobalDescriptors(lbo);
		InitGlobalBuffers();
		//printf("back to ui handler? \n");
		advancedRS.takeUIHandlerPtr(&uiHandler);
		//advancedRS.updateLoadingPipeline();
		uiHandler.isActive = false;
		Dimension2::Init();

		PipelineSystem::Emplace(Pipe::skybox, reinterpret_cast<PipelineSystem*>(Construct<Pipe_Skybox>({})));

		displayingRenderInfo = SettingsJSON::settingsData.renderInfo;
		RigidRenderingSystem::Initialize();

		printf("end of EightWindsEngine constructor \n");
	}
	void EightWindsEngine::FinishLoading() {
		printf("before init descriptors \n");
#if DRAWING_POINTS
		DescriptorHandler::initDescriptors(bufferMap);
#endif
		printf("after init descriptors \n");
		//advancedRS.updateMaterialPipelines();

		pointLightsEnabled = SettingsJSON::settingsData.pointLights;
		if (!pointLightsEnabled) {
			lbo.numLights = 0;
		}
	}

	EightWindsEngine::~EightWindsEngine() {


		Dimension2::Destruct();
		PipelineSystem::Destruct();
#if DECONSTRUCTION_DEBUG
		printf("beginning of EightWindsEngine deconstructor \n");
#endif
#if BENCHMARKING_GPU
		EWE_VK(vkDestroyQueryPool, VK::Object->vkDevice, queryPool[0], nullptr);
		EWE_VK(vkDestroyQueryPool, VK::Object->vkDevice, queryPool[1], nullptr);
#endif
		DescriptorHandler::Cleanup();


		RigidRenderingSystem::Destruct();
		MaterialPipelines::CleanupStaticVariables();

		MenuModule::cleanup();

#if DECONSTRUCTION_DEBUG
		printf("end of EightWindsEngine deconstructor \n");
#endif
	}

	void EightWindsEngine::InitGlobalBuffers() {

		DescriptorHandler::WriteToLightBuffer(lbo);

		camera.SetBuffers();
	}

	bool EightWindsEngine::BeginFrame() {
		if (renderFramework.eweRenderer.BeginFrame()) {
#if BENCHMARKING_GPU
			QueryTimestampBegin();
#endif
			return true;
		}
		//else {
			//std::pair<uint32_t, uint32_t> tempPair = EWERenderer.getExtent(); //debugging swap chain resize
			//printf("swap chain extent on start? %i : %i", tempPair.first, tempPair.second);
			//menuManager.windowResize(eweRenderer.getExtent());
		//}
		return false;
	}

	bool EightWindsEngine::BeginFrameAndRender() {
		//printf("begin render \n");
		if (renderFramework.eweRenderer.BeginFrame()) {
#if BENCHMARKING_GPU
			QueryTimestampBegin();
#endif

			renderFramework.eweRenderer.BeginSwapChainRender();
			return true;
		}
		//else {
			//std::pair<uint32_t, uint32_t> tempPair = EWERenderer.getExtent(); //debugging swap chain resize
			//printf("swap chain extent on start? %i : %i", tempPair.first, tempPair.second);
			//menuManager.windowResize(eweRenderer.getExtent());
		//}
		return false;
	}
	void EightWindsEngine::BeginRenderX() {
		renderFramework.eweRenderer.BeginSwapChainRender();
	}

	void EightWindsEngine::Draw2DObjects() {
		advancedRS.render2DGameObjects(menuManager.getMenuActive());
	}
	void EightWindsEngine::DrawObjects(double dt) {
		Draw3DObjects(dt);

		Draw2DObjects();
		DrawText(dt);
	}
	void EightWindsEngine::Draw3DObjects(double dt) {
		timeTracker = lab::Mod(timeTracker + dt, lab::GetPI(2.0));

		if (pointLightsEnabled) {
			PointLight::update(static_cast<float>(dt), advancedRS.pointLights);
			for (int i = 0; i < advancedRS.pointLights.size(); i++) {
				lbo.pointLights[i].position = lab::vec4(advancedRS.pointLights[i].transform.translation, 1.f);
				lbo.pointLights[i].color = lab::vec4(advancedRS.pointLights[i].color, advancedRS.pointLights[i].lightIntensity);
			}
			lbo.numLights = static_cast<uint8_t>(advancedRS.pointLights.size());

			DescriptorHandler::WriteToLightBuffer(lbo);

		}

		camera.UpdateCamera<CS>();
#if RENDER_DEBUG
		std::cout << "before rendering game objects \n";
#endif
		advancedRS.renderGameObjects(static_cast<float>(timeTracker));
		
#if RENDER_DEBUG
		std::cout << "before skin render \n";
#endif
		skinnedRS.Render();

		RigidRenderingSystem::Render();
#if RENDER_DEBUG
		std::cout << "end draw3dObjects \n";
#endif

	}

	void EightWindsEngine::DrawText(double dt) {

		uiHandler.BeginTextRender();
#if BENCHMARKING
		if (displayingRenderInfo) {
			uiHandler.Benchmarking(dt, peakRenderTime, averageRenderTime, highestRenderTime, BENCHMARKING_GPU, elapsedGPUMS, averageElapsedGPUMS);
		}
#endif

#if RENDER_DEBUG
		std::cout << "before drawing menu \n";
#endif
		uiHandler.DrawOverlayText(displayingRenderInfo);
		menuManager.drawText();
		uiHandler.EndTextRender();
	}

	void EightWindsEngine::EndRender() {
		renderFramework.eweRenderer.EndSwapChainRender();
	}

	void EightWindsEngine::EndFrame() {
		//printf("end render \n");
#if BENCHMARKING_GPU
		QueryTimestampEnd();
#endif
		//printf("after ending swap chain \n");
		if (renderFramework.eweRenderer.EndFrame()) {
#if EWE_DEBUG
			printf("dirty swap on end\n");
#endif
			//std::pair<uint32_t, uint32_t> tempPair = EWERenderer.getExtent(); //debugging swap chain recreation
			//printf("swap chain extent? %i : %i", tempPair.first, tempPair.second);
			//uiHandler.windowResize(eweRenderer.getExtent());
			VkExtent2D swapExtent = renderFramework.eweRenderer.GetExtent();
			SettingsInfo::ScreenDimensions resizeDimensions{};
			resizeDimensions.width = swapExtent.width;
			resizeDimensions.height = swapExtent.height;
			camera.SetPerspectiveProjection(lab::DegreesToRadians(70.0f), static_cast<float>(swapExtent.width) / static_cast<float>(swapExtent.height), 0.1f, 10000.0f);
			menuManager.WindowResize(resizeDimensions);
		}
	}
#if BENCHMARKING_GPU
	void EightWindsEngine::CreateQueryPool() {
		const VkPhysicalDeviceLimits devLimits = VK::Object->properties.limits;
		const float timestampPeriod = devLimits.timestampPeriod;
		if (timestampPeriod == 0.0f || !devLimits.timestampComputeAndGraphics) {
			timestampsAvailable = false;
			return;
		}
		gpuTicksPerSecond = timestampPeriod / 1e6f; //1e6 converts it from seconds to milliseconds

#if EWE_DEBUG
		assert(queryPool[0] == VK_NULL_HANDLE);
		assert(queryPool[1] == VK_NULL_HANDLE);
#endif

		VkQueryPoolCreateInfo queryPoolInfo = {};
		queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		queryPoolInfo.pNext = nullptr;
		queryPoolInfo.flags = 0;
		queryPoolInfo.pipelineStatistics = 0; //this could be useful https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueryPipelineStatisticFlagBits.html
		queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
		queryPoolInfo.queryCount = 2;
		EWE_VK(vkCreateQueryPool, VK::Object->vkDevice, &queryPoolInfo, nullptr, &queryPool[0]);
		EWE_VK(vkCreateQueryPool, VK::Object->vkDevice, &queryPoolInfo, nullptr, &queryPool[1]);
		timestampsAvailable = true;
	}

	void EightWindsEngine::QueryTimestampBegin() {

		if (displayingRenderInfo && timestampsAvailable) {
			//printf("before non-null \n");
			//shouldnt be activated until after the command has already been submitted at least once
			if (previouslySubmitted[VK::Object->frameIndex]) {
				TimestampData& timestampStart = timestamps[VK::Object->frameIndex * 2];
				TimestampData& timestampEnd = timestamps[VK::Object->frameIndex * 2 + 1];

#if 0 //WAITING FOR TIMESTAMP
				EWE_VK(vkGetQueryPoolResults, VK::Object->vkDevice, queryPool[VK::Object->frameIndex], 0, 2, sizeof(uint64_t) * 2, &timestampStart, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

				elapsedGPUMS = static_cast<float>(timestampEnd.result - timestampStart.result) * gpuTicksPerSecond;
				totalElapsedGPUMS += elapsedGPUMS;
				averageElapsedGPUCounter++;
				if (averageElapsedGPUCounter == 100) {
					averageElapsedGPUCounter = 0;
					averageElapsedGPUMS = totalElapsedGPUMS / 100.f;
					totalElapsedGPUMS = 0.f;
				}
#else //get only if available
				VkResult result = vkGetQueryPoolResults(
					VK::Object->vkDevice, queryPool[VK::Object->frameIndex], 
					0, 2, 
					sizeof(uint64_t) * 4, 
					&timestampStart, 
					sizeof(uint64_t) * 2, 
					VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT
				);
				if (result != VK_SUCCESS && result != VK_NOT_READY) {
					EWE_VK_RESULT(result);
				}

				if (timestampStart.result != 0) {
					elapsedGPUMS = static_cast<float>(timestampEnd.result - timestampStart.result) * gpuTicksPerSecond;
					totalElapsedGPUMS += elapsedGPUMS;
					averageElapsedGPUCounter++;
					if (averageElapsedGPUCounter == 100) {
						averageElapsedGPUCounter = 0;
						averageElapsedGPUMS = totalElapsedGPUMS / 100.f;
						totalElapsedGPUMS = 0.f;
					}
				}
#endif
			}
			else {
				previouslySubmitted[VK::Object->frameIndex] = true;
			}
			EWE_VK(vkCmdResetQueryPool, VK::Object->GetFrameBuffer(), queryPool[VK::Object->frameIndex], 0, 2);
			

			EWE_VK(vkCmdWriteTimestamp, VK::Object->GetFrameBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool[VK::Object->frameIndex], 0);
		}
	}
	void EightWindsEngine::QueryTimestampEnd() {
		if (displayingRenderInfo && timestampsAvailable) {
			EWE_VK(vkCmdWriteTimestamp, VK::Object->GetFrameBuffer(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool[VK::Object->frameIndex], 1);
		}
	}
#endif //BENCHMARKING_GPU
}