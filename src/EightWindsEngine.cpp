#include "EWEngine/EightWindsEngine.h"

//#include "keyboard_movement_controller.h" //this is for a free camera, which is currently not utilized
#include "EWEngine/Graphics/Device_Buffer.h"
#include "EWEngine/Graphics/Camera.h"
#include "EWEngine/Systems/Rendering/Pipelines/Dimension2.h"
#include "EWEngine/Systems/Rendering/Pipelines/Pipe_Skybox.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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
	inline void printmat4(glm::mat4& theMatrix, const std::string& matrixName) {
		printf("matrix values : %s\n", matrixName.c_str());
		for(uint8_t i = 0; i < 4; i++) {
			printf("\t%.3f:%.3f:%.3f:%.3f\n", theMatrix[i].x, theMatrix[i].y, theMatrix[i].z, theMatrix[i].w);
		}
	}

	void EightWindsEngine::EndEngineLoadScreen() {
		printf("~~~~ ENDING LOADING SCREEN ~~~ \n");

		//dependent on this not being in the graphics thread, or it'll infinitely loop
		SyncHub* syncHub = SyncHub::GetSyncHubInstance();

		while (syncHub->CheckFencesForUsage()) {/*printf("waiting on fences\n");*/ std::this_thread::sleep_for(std::chrono::nanoseconds(1)); }

		loadingEngine = false;
	}


	EightWindsEngine::EightWindsEngine(std::string windowName) :
		//first, any members not mentioned here with brackets will be initialized
		//second, any memberss in this section will be initialized

		mainWindow{ windowName },
		eweDevice{ mainWindow },
		eweRenderer{ mainWindow, camera },
		//computeHandler{},
		objectManager{},
		//imguiHandler{ mainWindow.getGLFWwindow(), MAX_FRAMES_IN_FLIGHT, eweRenderer.getSwapChainRenderPass() },

		/*2000 is ballparked, if its not set high enough then all textures will be moved, and invalidate the data*/
		uiHandler{ SettingsJSON::settingsData.getDimensions(), mainWindow.getGLFWwindow(), eweRenderer.MakeTextOverlay() },
		advancedRS{ objectManager, menuManager },
		textureManager{ },
		menuManager{ uiHandler.getScreenWidth(), uiHandler.getScreenHeight(), mainWindow.getGLFWwindow(), uiHandler.getTextOverlay() },
		skinnedRS{ }
	{
		printf("after finishing construction of engine\n");
		EWEPipeline::PipelineConfigInfo::pipelineRenderingInfoStatic = eweRenderer.getPipelineInfo();

		printf("eight winds constructor, ENGINE_VERSION: %s \n", ENGINE_VERSION);
		camera.SetPerspectiveProjection(glm::radians(70.0f), eweRenderer.GetAspectRatio(), 0.1f, 100000.0f);

		viewerObject.transform.translation = { -20.f, 21.f, -20.f };
		camera.NewViewTarget(viewerObject.transform.translation, { 0.f, 19.5f, 0.f });
		InitGlobalBuffers();
		DescriptorHandler::InitGlobalDescriptors(bufferMap);
		//printf("back to ui handler? \n");
		advancedRS.takeUIHandlerPtr(&uiHandler);
		//advancedRS.updateLoadingPipeline();
		uiHandler.isActive = false;
		leafSystem = Construct<LeafSystem>({});
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
		Deconstruct(leafSystem);
#if DECONSTRUCTION_DEBUG
		printf("beginning of EightWindsEngine deconstructor \n");
#endif
		EWE_VK(vkDestroyQueryPool, VK::Object->vkDevice, queryPool[0], nullptr);
		EWE_VK(vkDestroyQueryPool, VK::Object->vkDevice, queryPool[1], nullptr);
		DescriptorHandler::Cleanup();


		RigidRenderingSystem::Destruct();
		MaterialPipelines::CleanupStaticVariables();

		for (auto& dsl : TextureDSLInfo::descSetLayouts) {
			Deconstruct(dsl.second);
		}
		TextureDSLInfo::descSetLayouts.clear();
		Texture_Manager::GetTextureManagerPtr()->Cleanup();

		for (auto& bufferType : bufferMap) {
			for (auto& buffer : bufferType.second) {
				Deconstruct(buffer);
			}
		}
		bufferMap.clear();
		MenuModule::cleanup();

#if DECONSTRUCTION_DEBUG
		printf("end of EightWindsEngine deconstructor \n");
#endif
	}

	void EightWindsEngine::InitGlobalBuffers() {
		lbo.ambientColor = { 0.1f, 0.1f, 0.1f, 0.1f }; //w is alignment only?
		lbo.sunlightDirection = { 1.f, 3.f, 1.f, 0.f };
		lbo.sunlightDirection = glm::normalize(lbo.sunlightDirection);
		lbo.sunlightColor = { 0.8f,0.8f, 0.8f, 0.5f };

		bufferMap.try_emplace(Buff_ubo, std::vector<EWEBuffer*>{MAX_FRAMES_IN_FLIGHT});
		bufferMap.try_emplace(Buff_gpu, std::vector<EWEBuffer*>{MAX_FRAMES_IN_FLIGHT});

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			//allocate buffer memory
			bufferMap.at(Buff_ubo)[i] = Construct<EWEBuffer>({ sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT });
			bufferMap.at(Buff_ubo)[i]->Map();

			bufferMap.at(Buff_gpu)[i] = EWEBuffer::CreateAndInitBuffer(&lbo, sizeof(LightBufferObject), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);


		}
		camera.SetBuffers(&bufferMap.at(Buff_ubo));

		lbo.sunlightColor = { 0.8f,0.8f, 0.8f, 0.5f };
	}

	void EightWindsEngine::LoadingScreen() {
		printf("BEGINNING LEAF RENDER ~~~~~~~~~~~~~~~~ \n");
		//printf("beginning of leaf loading screen, thread ID : %d \n", std::this_thread::get_id());
		//QueryPerformanceCounter(&QPCstart);

		viewerObject.transform.translation = { -20.f, 21.f, -20.f };
		camera.NewViewTarget(viewerObject.transform.translation, { 0.f, 19.5f, 0.f }, glm::vec3(0.f, 1.f, 0.f));
		camera.BindBothUBOs();
		
		//LARGE_INTEGER averageStart;
		//QueryPerformanceCounter(&averageStart);
		//LARGE_INTEGER averageEnd;
		//printf("loading screen entry \n");
		//SyncHub::GetSyncHubInstance()->waitOnTransferFence();
		SyncHub* syncHub = SyncHub::GetSyncHubInstance();
#if EWE_DEBUG
		printf("before init leaf data on GPU\n");
#endif
		leafSystem->InitData();
#if EWE_DEBUG
		printf("after init leaf data\n");
#endif

		leafSystem->LoadLeafModel();
#if EWE_DEBUG
		printf("after leaf mesh\n");
#endif
		
		leafSystem->LoadLeafTexture();
#if EWE_DEBUG
		printf("after leaf texture\n");
#endif

		//EWE_VK(vmaCheckCorruption(EWEDevice::GetAllocator(), UINT32_MAX));


		double renderThreadTime = 0.0;
		const double renderTimeCheck = 1000.0 / 60.0;
		auto startThreadTime = std::chrono::high_resolution_clock::now();
		auto endThreadTime = startThreadTime;
		//printf("starting loading thread loop \n");
		while (loadingEngine || (loadingTime < 2000.0)) {

			endThreadTime = std::chrono::high_resolution_clock::now();
			renderThreadTime += std::chrono::duration<double, std::chrono::milliseconds::period>(endThreadTime - startThreadTime).count();
			startThreadTime = endThreadTime;

			//auto newTime = std::chrono::high_resolution_clock::now();
			//QueryPerformanceCounter(&QPCend);
			//renderThreadTime += static_cast<double>(QPCend.QuadPart - QPCstart.QuadPart) / frequency.QuadPart;
			//QPCstart = QPCend;
			if (renderThreadTime > renderTimeCheck) {
				loadingTime += renderTimeCheck;
				//printf("rendering loading thread start??? \n");
				syncHub->RunGraphicsCallbacks();

				if (eweRenderer.BeginFrame()) {
					
					eweRenderer.BeginSwapChainRenderPass();
					leafSystem->FallCalculation(static_cast<float>(renderThreadTime / 1000.0));

					leafSystem->Render();
					//uiHandler.drawMenuMain(commandBuffer);
					eweRenderer.EndSwapChainRenderPass();
					if (eweRenderer.EndFrame()) {
						menuManager.windowResize(eweRenderer.GetExtent());
					}
				}
				else {

					menuManager.windowResize(eweRenderer.GetExtent());
				}
				
				renderThreadTime = 0.0;
				//printf("end rendering thread \n");
			}
			//printf("end of render thread loop \n");
		}
		finishedLoadingScreen = true;
		CreateQueryPool();
#if EWE_DEBUG
		printf(" ~~~~ END OF LOADING SCREEN FUNCTION \n");
#endif
	}
	bool EightWindsEngine::BeginRenderWithoutPass() {
		if (eweRenderer.BeginFrame()) {
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

	bool EightWindsEngine::BeginRender() {
		//printf("begin render \n");
		if (eweRenderer.BeginFrame()) {
#if BENCHMARKING_GPU
			QueryTimestampBegin();
#endif

			eweRenderer.BeginSwapChainRenderPass();
			return true;
		}
		//else {
			//std::pair<uint32_t, uint32_t> tempPair = EWERenderer.getExtent(); //debugging swap chain resize
			//printf("swap chain extent on start? %i : %i", tempPair.first, tempPair.second);
			//menuManager.windowResize(eweRenderer.getExtent());
		//}
		return false;
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
		timeTracker = glm::mod(timeTracker + dt, glm::two_pi<double>());

		if (pointLightsEnabled) {
			PointLight::update(static_cast<float>(dt), objectManager.pointLights);
			for (int i = 0; i < objectManager.pointLights.size(); i++) {
				lbo.pointLights[i].position = glm::vec4(objectManager.pointLights[i].transform.translation, 1.f);
				lbo.pointLights[i].color = glm::vec4(objectManager.pointLights[i].color, objectManager.pointLights[i].lightIntensity);
			}
			lbo.numLights = static_cast<uint8_t>(objectManager.pointLights.size());
			bufferMap.at(Buff_gpu)[VK::Object->frameIndex]->WriteToBuffer(&lbo);
			bufferMap.at(Buff_gpu)[VK::Object->frameIndex]->Flush();
		}

		camera.ViewTargetDirect();
#if RENDER_DEBUG
		std::cout << "before rendering game objects \n";
#endif
		advancedRS.renderGameObjects(timeTracker);
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
			uiHandler.Benchmarking(dt, peakRenderTime, averageRenderTime, highestRenderTime, averageLogicTime, BENCHMARKING_GPU, elapsedGPUMS, averageElapsedGPUMS);
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
		eweRenderer.EndSwapChainRenderPass();
	}

	void EightWindsEngine::EndFrame() {
		//printf("end render \n");
#if BENCHMARKING_GPU
		QueryTimestampEnd();
#endif
		//printf("after ending swap chain \n");
		if (eweRenderer.EndFrame()) {
#if EWE_DEBUG
			printf("dirty swap on end\n");
#endif
			//std::pair<uint32_t, uint32_t> tempPair = EWERenderer.getExtent(); //debugging swap chain recreation
			//printf("swap chain extent? %i : %i", tempPair.first, tempPair.second);
			//uiHandler.windowResize(eweRenderer.getExtent());
			menuManager.windowResize(eweRenderer.GetExtent());
		}
	}
#if BENCHMARKING_GPU
	void EightWindsEngine::CreateQueryPool() {
		if (!displayingRenderInfo) {
			return;
		}
		const VkPhysicalDeviceLimits devLimits = VK::Object->properties.limits;
		const float timestampPeriod = devLimits.timestampPeriod;
		if (timestampPeriod == 0.0f || !devLimits.timestampComputeAndGraphics) {
			displayingRenderInfo = false;
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
	}

	void EightWindsEngine::QueryTimestampBegin() {

		if (displayingRenderInfo) {
			//printf("before non-null \n");
			//shouldnt be activated until after the command has already been submitted at least once
			if (previouslySubmitted[VK::Object->frameIndex]) {
				uint64_t& timestampStart = timestamps[VK::Object->frameIndex * 2];
				uint64_t& timestampEnd = timestamps[VK::Object->frameIndex * 2 + 1];
				EWE_VK(vkGetQueryPoolResults, VK::Object->vkDevice, queryPool[VK::Object->frameIndex], 0, 1, sizeof(uint64_t) * 2, &timestampStart, sizeof(uint64_t) * 2, VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
				elapsedGPUMS = static_cast<float>(timestampEnd - timestampStart) * gpuTicksPerSecond;
				totalElapsedGPUMS += elapsedGPUMS;
				averageElapsedGPUCounter++;
				if (averageElapsedGPUCounter == 100) {
					averageElapsedGPUCounter = 0;
					averageElapsedGPUMS = totalElapsedGPUMS / 100.f;
					totalElapsedGPUMS = 0.f;
				}
			}
			else {
				previouslySubmitted[VK::Object->frameIndex] = true;
			}
			EWE_VK(vkCmdResetQueryPool, VK::Object->GetFrameBuffer(), queryPool[VK::Object->frameIndex], 0, 2);
			

			EWE_VK(vkCmdWriteTimestamp, VK::Object->GetFrameBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool[VK::Object->frameIndex], 0);
		}
	}
	void EightWindsEngine::QueryTimestampEnd() {
		if (displayingRenderInfo) {
			EWE_VK(vkCmdWriteTimestamp, VK::Object->GetFrameBuffer(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool[VK::Object->frameIndex], 1);
		}
	}
#endif //BENCHMARKING_GPU
}