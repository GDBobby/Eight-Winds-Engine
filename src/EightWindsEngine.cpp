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


namespace EWE {
	inline void stdcoutamat4(glm::mat4& theMatrix, std::string matrixName) {
		std::cout << matrixName << " 0 : " << theMatrix[0].x << ":" << theMatrix[0].y << ":" << theMatrix[0].z << ":" << theMatrix[0].w << std::endl;
		std::cout << matrixName << " 1 : " << theMatrix[1].x << ":" << theMatrix[1].y << ":" << theMatrix[1].z << ":" << theMatrix[1].w << std::endl;
		std::cout << matrixName << " 2 : " << theMatrix[2].x << ":" << theMatrix[2].y << ":" << theMatrix[2].z << ":" << theMatrix[2].w << std::endl;
		std::cout << matrixName << " 3 : " << theMatrix[3].x << ":" << theMatrix[3].y << ":" << theMatrix[3].z << ":" << theMatrix[3].w << std::endl;
	}
	EightWindsEngine::EightWindsEngine(std::string windowName) :
		//first, any members not mentioned here with brackets will be initialized
		//second, any memberss in this section will be initialized

		mainWindow{ windowName },
		eweDevice{ mainWindow },
		camera{},
		eweRenderer{ mainWindow, eweDevice, camera },
		//computeHandler{eweDevice},
		objectManager{},
		//imguiHandler{ mainWindow.getGLFWwindow(), eweDevice, MAX_FRAMES_IN_FLIGHT, eweRenderer.getSwapChainRenderPass() },

		/*2000 is ballparked, if its not set high enough then all textures will be moved, and invalidate the data*/
		uiHandler{ SettingsJSON::settingsData.getDimensions(), eweDevice, mainWindow.getGLFWwindow(), eweRenderer.makeTextOverlay() },
		menuManager{ uiHandler.getScreenWidth(), uiHandler.getScreenHeight(), eweDevice, mainWindow.getGLFWwindow(), uiHandler.getTextOverlay() },
		advancedRS{ eweDevice, objectManager, menuManager},
		skinnedRS{ eweDevice },
		textureManager{eweDevice}
	{
		EWEPipeline::PipelineConfigInfo::pipelineRenderingInfoStatic = eweRenderer.getPipelineInfo();

		printf("eight winds constructor, ENGINE_VERSION: %s \n", ENGINE_VERSION);
		camera.setPerspectiveProjection(glm::radians(70.0f), eweRenderer.getAspectRatio(), 0.1f, 10000.0f);

		viewerObject.transform.translation = { -20.f, 21.f, -20.f };
		camera.newViewTarget(viewerObject.transform.translation, { 0.f, 19.5f, 0.f }, glm::vec3(0.f, 1.f, 0.f));
		initGlobalBuffers();
		DescriptorHandler::initGlobalDescriptors(bufferMap, eweDevice);
		//printf("back to ui handler? \n");
		advancedRS.takeUIHandlerPtr(&uiHandler);
		//advancedRS.updateLoadingPipeline();
		uiHandler.isActive = false;
		leafSystem = std::make_unique<LeafSystem>(eweDevice);
		Dimension2::init(eweDevice);
		PipelineSystem::emplace(Pipe_skybox, reinterpret_cast<PipelineSystem*>(new Pipe_Skybox(eweDevice)));

		displayingRenderInfo = SettingsJSON::settingsData.renderInfo;
		RigidRenderingSystem::getRigidRSInstance();

		printf("end of EightWindsEngine constructor \n");
	}
	void EightWindsEngine::finishLoading() {
		printf("before init descriptors \n");
		DescriptorHandler::initDescriptors(bufferMap);
		printf("after init descriptors \n");
		//advancedRS.updateMaterialPipelines();

		pointLightsEnabled = SettingsJSON::settingsData.pointLights;
		if (!pointLightsEnabled) {
			lbo.numLights = 0;
		}
	}

	EightWindsEngine::~EightWindsEngine() {
		Dimension2::destruct(eweDevice);
		PipelineSystem::destruct(eweDevice);
#if DECONSTRUCTION_DEBUG
		printf("beginning of EightWindsEngine deconstructor \n");
#endif
		printf("after deconstructig level manager \n");
		vkDestroyQueryPool(eweDevice.device(), queryPool, nullptr);
		DescriptorHandler::cleanup(eweDevice);


		RigidRenderingSystem::destruct();
		MaterialPipelines::cleanupStaticVariables(eweDevice);

		for (auto& bufferType : bufferMap) {
			for (auto& buffer : bufferType.second) {
				delete buffer;
			}
		}
		bufferMap.clear();
		MenuModule::cleanup();

#if DECONSTRUCTION_DEBUG
		printf("end of EightWindsEngine deconstructor \n");
#endif
	}

	void EightWindsEngine::initGlobalBuffers() {
		lbo.ambientColor = { 0.1f, 0.1f, 0.1f, 0.1f }; //w is alignment only?
		lbo.sunlightDirection = { 1.f, 3.f, 1.f, 0.f };
		lbo.sunlightDirection = glm::normalize(lbo.sunlightDirection);
		lbo.sunlightColor = { 0.8f,0.8f, 0.8f, 0.5f };

		bufferMap[Buff_ubo].resize(MAX_FRAMES_IN_FLIGHT);
		bufferMap[Buff_gpu].resize(MAX_FRAMES_IN_FLIGHT);


		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			//allocate buffer memory
			bufferMap[Buff_ubo][i] = new EWEBuffer(eweDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			bufferMap[Buff_ubo][i]->map();
			bufferMap[Buff_gpu][i] = new EWEBuffer(eweDevice, sizeof(LightBufferObject), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			bufferMap[Buff_gpu][i]->map();

			//printf("mapping lbo \n");
			bufferMap[Buff_gpu][i]->writeToBuffer(&lbo);
			bufferMap[Buff_gpu][i]->flush();

		}
		camera.setBuffers(&bufferMap[Buff_ubo]);

		lbo.sunlightColor = { 0.8f,0.8f, 0.8f, 0.5f };
	}

	void EightWindsEngine::loadingScreen() {
		printf("BEGINNING LEAF RENDER ~~~~~~~~~~~~~~~~ \n");
		//printf("beginning of leaf loading screen, thread ID : %d \n", std::this_thread::get_id());
		//QueryPerformanceCounter(&QPCstart);
		double renderThreadTime = 0.0;
		const double renderTimeCheck = 1.0 / 60.0;
		auto startThreadTime = std::chrono::high_resolution_clock::now();
		auto endThreadTime = startThreadTime;


		viewerObject.transform.translation = { -20.f, 21.f, -20.f };
		camera.newViewTarget(viewerObject.transform.translation, { 0.f, 19.5f, 0.f }, glm::vec3(0.f, 1.f, 0.f));
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			camera.bindUBO(i);
		}
		//LARGE_INTEGER averageStart;
		//QueryPerformanceCounter(&averageStart);
		//LARGE_INTEGER averageEnd;
		//printf("loading screen entry \n");
		//SyncHub::getSyncHubInstance()->waitOnTransferFence();

		//printf("starting loading thread loop \n");
		while (loadingEngine || (loadingTime < 2.0)) {
			
			//printf("loading render looop : %.2f \n", loadingTime);

			endThreadTime = std::chrono::high_resolution_clock::now();
			renderThreadTime += std::chrono::duration<double, std::chrono::seconds::period>(endThreadTime - startThreadTime).count();
			startThreadTime = endThreadTime;

			//auto newTime = std::chrono::high_resolution_clock::now();
			//QueryPerformanceCounter(&QPCend);
			//renderThreadTime += static_cast<double>(QPCend.QuadPart - QPCstart.QuadPart) / frequency.QuadPart;
			//QPCstart = QPCend;
			if (renderThreadTime > renderTimeCheck) {
				loadingTime += renderTimeCheck;
				//printf("rendering loading thread start??? \n");
				
				auto frameInfo = eweRenderer.beginFrame();
				if (frameInfo.cmdBuf != VK_NULL_HANDLE) {
					int frameIndex = eweRenderer.getFrameIndex();

#if false//BENCHMARKING_GPU
					if (queryPool == VK_NULL_HANDLE) {
						gpuTicksPerSecond = eweDevice.getProperties().limits.timestampPeriod / 1e9f;

						VkQueryPoolCreateInfo queryPoolInfo = {};
						queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
						queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
						queryPoolInfo.queryCount = 2; // We'll use two timestamps if we run into trouble with framebuffers use 4
						vkCreateQueryPool(eweDevice.device(), &queryPoolInfo, nullptr, &queryPool);
					}
					else {
						//shouldnt be activated until after the command has already been submitted at least once
						uint64_t timestampStart, timestampEnd;
						vkGetQueryPoolResults(eweDevice.device(), queryPool, 0, 1, sizeof(uint64_t) * 2, &timestampStart, sizeof(uint64_t), VK_QUERY_RESULT_WAIT_BIT);
						vkGetQueryPoolResults(eweDevice.device(), queryPool, 1, 1, sizeof(uint64_t) * 2, &timestampEnd, sizeof(uint64_t), VK_QUERY_RESULT_WAIT_BIT);
						elapsedGPUMS = static_cast<float>(timestampEnd - timestampStart) * gpuTicksPerSecond * 1000.f;
						totalElapsedGPUMS += elapsedGPUMS;
						averageElapsedGPUCounter++;
						if (averageElapsedGPUCounter == 100) {
							averageElapsedGPUCounter = 0;
							averageElapsedGPUMS = totalElapsedGPUMS / 100.f;
							totalElapsedGPUMS = 0.f;
						}
						//need to add this to the text renderer
						//printf("elapsed GPU seconds : %.5f \n", elapsedGPUMS);
					}

					vkCmdResetQueryPool(commandBufferPair.first, queryPool, 0, 2);
					vkCmdWriteTimestamp(commandBufferPair.first, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 0);
#endif

					eweRenderer.beginSwapChainRenderPass(frameInfo.cmdBuf);
					leafSystem->fallCalculation(static_cast<float>(renderThreadTime), frameIndex);
#if false//BENCHMARKING
					uiHandler.Benchmarking(renderThreadTime, peakRenderTime, averageRenderTime, minRenderTime, highestRenderTime, averageLogicTime, BENCHMARKING_GPU, elapsedGPUMS, averageElapsedGPUMS);
#endif
					leafSystem->render(frameInfo);
					//uiHandler.drawMenuMain(commandBuffer);
					eweRenderer.endSwapChainRenderPass(frameInfo.cmdBuf);
					if (eweRenderer.endFrame()) {
						//printf("dirty swap on end\n");
						std::pair<uint32_t, uint32_t> tempPair = eweRenderer.getExtent();
						//printf("swap chain extent? %i : %i", tempPair.first, tempPair.second);
					}
				}
				else {
					std::pair<uint32_t, uint32_t> tempPair = eweRenderer.getExtent();
					//printf("swap chain extent on start? %i : %i", tempPair.first, tempPair.second);
					menuManager.windowResize(tempPair);
				}
				renderThreadTime = 0.f;
				//printf("end rendering thread \n");
				
			}
			//printf("end of render thread loop \n");
		}
		finishedLoadingScreen = true;
		printf(" ~~~~ END OF LOADING SCREEN FUNCTION \n");
	}
	FrameInfo EightWindsEngine::beginRender() {
		//printf("begin render \n");
		FrameInfo frameInfo{ eweRenderer.beginFrame() };
		if (frameInfo.cmdBuf) {
			if (displayingRenderInfo) {
#if BENCHMARKING_GPU
				if (queryPool == VK_NULL_HANDLE) {
					gpuTicksPerSecond = eweDevice.getProperties().limits.timestampPeriod / 1e9f;

					VkQueryPoolCreateInfo queryPoolInfo = {};
					queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
					queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
					queryPoolInfo.queryCount = 2; // We'll use two timestamps if we run into trouble with framebuffers use 4
					VkResult result = vkCreateQueryPool(eweDevice.device(), &queryPoolInfo, nullptr, &queryPool);
					if (result != VK_SUCCESS) {
						printf("create query pool failed : %d \n", result);
						queryPool = VK_NULL_HANDLE;
					}
				}
				else {
					//printf("before non-null \n");
					//shouldnt be activated until after the command has already been submitted at least once
					uint64_t timestampStart, timestampEnd;
					vkGetQueryPoolResults(eweDevice.device(), queryPool, 0, 1, sizeof(uint64_t) * 2, &timestampStart, sizeof(uint64_t), VK_QUERY_RESULT_WAIT_BIT);
					vkGetQueryPoolResults(eweDevice.device(), queryPool, 1, 1, sizeof(uint64_t) * 2, &timestampEnd, sizeof(uint64_t), VK_QUERY_RESULT_WAIT_BIT);
					elapsedGPUMS = static_cast<float>(timestampEnd - timestampStart) * gpuTicksPerSecond * 1000.f;
					totalElapsedGPUMS += elapsedGPUMS;
					averageElapsedGPUCounter++;
					if (averageElapsedGPUCounter == 100) {
						averageElapsedGPUCounter = 0;
						averageElapsedGPUMS = totalElapsedGPUMS / 100.f;
						totalElapsedGPUMS = 0.f;
					}
					//need to add this to the text renderer
					//printf("elapsed GPU seconds : %.5f \n", elapsedGPUMS);
				}

				vkCmdResetQueryPool(frameInfo.cmdBuf, queryPool, 0, 2);
				vkCmdWriteTimestamp(frameInfo.cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 0);

#endif
			}
			eweRenderer.beginSwapChainRenderPass(frameInfo.cmdBuf);
			skinnedRS.setFrameIndex(frameInfo.index);
		}
		else {
			//std::pair<uint32_t, uint32_t> tempPair = EWERenderer.getExtent(); //debugging swap chain resize
			//printf("swap chain extent on start? %i : %i", tempPair.first, tempPair.second);
			//menuManager.windowResize(eweRenderer.getExtent());
		}

		return frameInfo;
	}
	void EightWindsEngine::draw2DObjects(FrameInfo frameInfo) {
		advancedRS.render2DGameObjects(frameInfo, menuManager.getMenuActive());
	}
	void EightWindsEngine::drawObjects(FrameInfo frameInfo, double dt) {
		PipelineSystem::setFrameInfo(frameInfo);
		draw3DObjects(frameInfo, dt);
		draw2DObjects(frameInfo);
		drawText(frameInfo, dt);
	}
	void EightWindsEngine::draw3DObjects(FrameInfo frameInfo, double dt) {
		timeTracker = glm::mod(timeTracker + dt, glm::two_pi<double>());

		if (pointLightsEnabled) {
			PointLight::update(static_cast<float>(dt), objectManager.pointLights);
			for (int i = 0; i < objectManager.pointLights.size(); i++) {
				lbo.pointLights[i].position = glm::vec4(objectManager.pointLights[i].transform.translation, 1.f);
				lbo.pointLights[i].color = glm::vec4(objectManager.pointLights[i].color, objectManager.pointLights[i].lightIntensity);
			}
			lbo.numLights = static_cast<uint8_t>(objectManager.pointLights.size());
			bufferMap[Buff_gpu][frameInfo.index]->writeToBuffer(&lbo);
			bufferMap[Buff_gpu][frameInfo.index]->flush();
		}

		camera.ViewTargetDirect(frameInfo.index);
#ifdef RENDER_OBJECT_DEBUG
		std::cout << "before rendering game objects \n";
#endif
		advancedRS.renderGameObjects(frameInfo, timeTracker);
#ifdef RENDER_OBJECT_DEBUG
		std::cout << "before skin render \n";
#endif
		skinnedRS.render(frameInfo);
#ifdef RENDER_OBJECT_DEBUG
		std::cout << "render 2d game objects \n";
#endif
#ifdef RENDER_OBJECT_DEBUG
		std::cout << "before text render \n";
#endif

#ifdef RENDER_OBJECT_DEBUG
		std::cout << "after rendering game objects \n";
#endif

	}

	void EightWindsEngine::drawText(FrameInfo frameInfo, double dt) {
		uiHandler.beginTextRender();
#if BENCHMARKING
		if (displayingRenderInfo) {
			uiHandler.Benchmarking(dt, peakRenderTime, averageRenderTime, highestRenderTime, averageLogicTime, BENCHMARKING_GPU, elapsedGPUMS, averageElapsedGPUMS);
		}
#endif

#ifdef RENDER_OBJECT_DEBUG
		std::cout << "before drawing menu \n";
#endif
		uiHandler.drawOverlayText(frameInfo.cmdBuf, displayingRenderInfo);
		menuManager.drawText();
		uiHandler.endTextRender(frameInfo.cmdBuf);
	}

	void EightWindsEngine::endRender(FrameInfo frameInfo) {
		//printf("end render \n");


#if BENCHMARKING_GPU
		if (displayingRenderInfo) {
			vkCmdWriteTimestamp(frameInfo.cmdBuf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool, 1);
		}
#endif
		eweRenderer.endSwapChainRenderPass(frameInfo.cmdBuf);
		//printf("after ending swap chain \n");
		if (eweRenderer.endFrame()) {
			printf("dirty swap on end\n");
			//std::pair<uint32_t, uint32_t> tempPair = EWERenderer.getExtent(); //debugging swap chain recreation
			//printf("swap chain extent? %i : %i", tempPair.first, tempPair.second);
			//uiHandler.windowResize(eweRenderer.getExtent());
			menuManager.windowResize(eweRenderer.getExtent());
		}
	}
}