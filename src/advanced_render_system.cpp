#include "EWEngine/Systems/Rendering/advanced_render_system.h"

#include "EWEngine/Systems/PipelineSystem.h"
#include "EWEngine/Systems/Rendering/Pipelines/Dimension2.h"
#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"
#include "EWEngine/Graphics/Texture/Image_Manager.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <stdexcept>
#include <iostream>
#include <set>


namespace EWE {

	//descriptorsetlayouts are in a vector vector
	//the first layert of the vector (vector<vector>>) designates a pipeline
	//the second layer (the vector inside the vector) designates the descriptorsets in that pipeline

	AdvancedRenderSystem::AdvancedRenderSystem(MenuManager& menuManager) : menuManager{ menuManager } {
#if EWE_DEBUG
		printf("ARS constructor \n");
#endif

		model2D = Basic_Model::Quad2D();
#if EWE_DEBUG
		printf("after ARS constructor finished \n");
#endif
	}

	AdvancedRenderSystem::~AdvancedRenderSystem() {
#if true//DECONSTRUCTION_DEBUG
		std::cout << "entering ARS deconstructor " << std::endl;
#endif
		EWEPipeline::CleanShaderModules();

		EWEDescriptorPool::DestructPools();


#if true//DECONSTRUCTION_DEBUG
		printf("end of ARS deconstructor \n");
#endif
	}

	void AdvancedRenderSystem::CreateSkyboxDescriptor(ImageID skyboxImgID) {
		if (skyboxDescriptors[0] != VK_NULL_HANDLE) {
			EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, &skyboxDescriptors[0]);
			EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, &skyboxDescriptors[1]);
		}
		if (skyboxEDSL == nullptr) {
			skyboxEDSL = PipelineSystem::At(Pipe::skybox)->GetDSL();
		}

		assert(skyboxEDSL != nullptr);
		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			EWEDescriptorWriter descWriter{ skyboxEDSL, DescriptorPool_Global };
			DescriptorHandler::AddCameraDataToDescriptor(descWriter, i);
#if DESCRIPTOR_IMAGE_IMPLICIT_SYNCHRONIZATION
			descWriter.WriteImage(skyboxImgID);
#else
			descWriter.WriteImage(Image_Manager::GetDescriptorImageInfo(skyboxImgID));
#endif
			skyboxDescriptors[i] = descWriter.Build();
		}
#if DEBUG_NAMING
		DebugNaming::SetObjectName(skyboxDescriptors[0], VK_OBJECT_TYPE_DESCRIPTOR_SET, "skybox desc[0]");
		DebugNaming::SetObjectName(skyboxDescriptors[1], VK_OBJECT_TYPE_DESCRIPTOR_SET, "skybox desc[1]");
#endif
	}

	void AdvancedRenderSystem::renderGameObjects(float time) {
#if DEBUGGING_PIPELINES
		printf("getting into render game objects \n");
#endif
		if (drawSkybox) {
			renderSkybox();
		}
#if DEBUGGING_PIPELINES
		printf("after rendering dynamic \n");
#endif

		RenderLightning();

		if (shouldRenderPoints){
#if DRAWING_POINTS
			renderPointLights();
#endif
		}

#if DEBUGGING_PIPELINES
		printf("end of rendering \n");
#endif
	}

	void AdvancedRenderSystem::renderSkybox() {
		assert(skyboxModel != nullptr);
#if DEBUGGING_PIPELINES
		printf("drawing skybox \n");
#endif
		auto pipe = PipelineSystem::At(Pipe::skybox);
		pipe->BindPipeline();
		pipe->BindDescriptor(0, &skyboxDescriptors[VK::Object->frameIndex]);

		pipe->BindModel(skyboxModel);
		pipe->DrawModel();
		
	}

	void AdvancedRenderSystem::RenderLightning() {
		//printf("beginning render lightning \n");
		//i need to pull this data out of the voids gaze source, if it still exists (rip)
	}
	/*
	void AdvancedRenderSystem::RenderGrass(float time) {

		if (objectManager.grassField.size() == 0) { return; }
#if DEBUGGING_PIPELINES
		printf("Drawing grass \n");
#endif
		PipelineSystem* pipe = PipelineSystem::At(Pipe::grass);
		pipe->BindPipeline();
		//objectManager.grassTextureID
		pipe->BindDescriptor(0, &grassDescriptors[VK::Object->frameIndex]);


		UVScrollingPushData push{ glm::vec2{glm::mod(time / 6.f, 1.f), glm::mod(time / 9.f, 1.f)} };
		pipe->Push(&push);

		for (const auto& grassField : objectManager.grassField) {
			pipe->DrawInstanced(grassField.model);
		}
#if DEBUGGING_PIPELINES
		printf("after drawing grass \n");
#endif
	}
	*/

	void AdvancedRenderSystem::render2DGameObjects(bool menuActive) {

		//printf("beginning r2d : %d \n", frameInfo.menuActive);
		//if (frameInfo.menuActive || GameUI::GetActive()) {
		if (!menuActive) {
			
			if (uiHandler->overlay) {
				if (!uiHandler->overlay->GetActive()) {
					return;
				}
			}
			else {
				return;
			}
			
		}

		if (menuActive) {
			Dimension2::BindArrayPipeline(); 
			menuManager.drawNewNine();
			
			menuManager.drawNewMenuObejcts();

			if (uiHandler->overlay) {
				uiHandler->overlay->DrawObjects();
			}
			if (menuManager.DrawingImages()) {
				Dimension2::BindSingularPipeline();
				menuManager.DrawImages();
			}
		}
		else {
			if (uiHandler->overlay) {
				uiHandler->overlay->DrawObjects();
			}
		}
	}
}