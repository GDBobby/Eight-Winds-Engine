#include "EWEngine/Systems/Rendering/advanced_render_system.h"

#include "EWEngine/Systems/PipelineSystem.h"
#include "EWEngine/Systems/Rendering/Pipelines/Dimension2.h"
#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"
#include "EWEngine/Graphics/Texture/Texture_Manager.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <stdexcept>
#include <iostream>
#include <set>





//#define GRASS_ENABLED false

namespace EWE {

	//descriptorsetlayouts are in a vector vector
	//the first layert of the vector (vector<vector>>) designates a pipeline
	//the second layer (the vector inside the vector) designates the descriptorsets in that pipeline

	AdvancedRenderSystem::AdvancedRenderSystem(ObjectManager& objectManager, MenuManager& menuManager) : objectManager{ objectManager }, menuManager{ menuManager } {
		printf("ARS constructor \n");
		EWEDescriptorPool::BuildGlobalPool();

		model2D = Basic_Model::Quad2D(Queue::transfer);
		printf("after ARS constructor finished \n");
	}

	AdvancedRenderSystem::~AdvancedRenderSystem() {
#if true//DECONSTRUCTION_DEBUG
		std::cout << "entering ARS deconstructor " << std::endl;
#endif
		EWEPipeline::cleanShaderModules();

		EWEDescriptorPool::DestructPools();


#if true//DECONSTRUCTION_DEBUG
		printf("end of ARS deconstructor \n");
#endif
	}

	void AdvancedRenderSystem::renderGameObjects(FrameInfo &frameInfo, float time) {
#if DEBUGGING_PIPELINES
		printf("getting into render game objects \n");
#endif
		if (drawSkybox) {
			renderSkybox(frameInfo);
		}
		//renderSimpleGameObjects(frameInfo);
		//renderBonedWeapons(frameInfo);
		
		renderTexturedGameObjects(frameInfo);

		//RenderDynamicMaterials(frameInfo);

		RigidRenderingSystem::Render(frameInfo);
#if DEBUGGING_PIPELINES
		printf("after rendering dynamic \n");
#endif

		renderVisualEffects(frameInfo);

		RenderLightning(frameInfo);

		if (shouldRenderPoints){ //shouldRenderPoints) {
#if DRAWING_POINTS
			renderPointLights(frameInfo);
#endif
		}

#if DEBUGGING_PIPELINES
		printf("end of rendering \n");
#endif
	}

	void AdvancedRenderSystem::renderSprites(FrameInfo& frameInfo) {
		/*
			PipelineManager::pipelines[Pipe::sprite]->bind(frameInfo.commandBuffer);
			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_sprite],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.frameIndex),
				0,
				nullptr
			);
			uint32_t currentBindedTextureID = -1;

			for (auto iter = frameInfo.objectManager.spriteBuildObjects.begin(); iter != frameInfo.objectManager.spriteBuildObjects.end(); iter++) {
				if ((iter->second.textureID == -1) || (iter->second.model == nullptr)) {
					std::cout << "why does a textured game object have no texture, or no model?? " << std::endl;
					continue;
				}
				else if (iter->second.textureID != currentBindedTextureID) {
					//std::cout << "texture desriptor set size : " << textureDescriptorSets.size() << std::endl;
					//std::cout << "iterator value : " << iter->second.textureID * 2 + frameInfo.frameIndex << std::endl;

					vkCmdBindDescriptorSets(
						frameInfo.commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						PipelineManager::pipeLayouts[PL_sprite],
						1, 1,
						//&EWETexture::modeDescriptorSets[iter->second.textureID * 2 + frameInfo.frameIndex],
						EWETexture::getSpriteDescriptorSets(iter->second.textureID, frameInfo.frameIndex),
						0, nullptr
					);
					currentBindedTextureID = iter->second.textureID;
				}
				//printf("drawing now : %d \n", i);
				SimplePushConstantData push{};
				push.modelMatrix = iter->second.transform.mat4();
				push.normalMatrix = iter->second.transform.normalMatrix();
				//std::cout << "pre-bind/draw : " << i;
				vkCmdPushConstants(frameInfo.commandBuffer, PipelineManager::pipeLayouts[PL_sprite], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
				iter->second.model->bind(frameInfo.commandBuffer);
				iter->second.model->draw(frameInfo.commandBuffer);
			}


			EWETexture::newSpriteFrame();
*/
	}

	void AdvancedRenderSystem::renderSkybox(FrameInfo& frameInfo) {
		assert(objectManager.skybox.first != nullptr);
#if DEBUGGING_PIPELINES
		printf("drawing skybox \n");
#endif
		//skyboxPipeline->bind(frameInfo.frameInfo.cmdBuf);
		auto pipe = PipelineSystem::At(Pipe::skybox);
		pipe->BindPipeline();
		pipe->BindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameInfo.index));
		pipe->BindDescriptor(1, &objectManager.skybox.second);

		pipe->BindModel(objectManager.skybox.first);
		pipe->DrawModel();
		
	}

	inline void AdvancedRenderSystem::renderTexturedGameObjects(FrameInfo& frameInfo) {
		if ((objectManager.texturedGameObjects.size() > 0)) {
			auto pipe = PipelineSystem::At(Pipe::textured);
#if DEBUGGING_PIPELINES
			printf("Drawing texutered game objects \n");
#endif
			//texturedPipeline->bind(frameInfo.frameInfo.cmdBuf);
			pipe->BindPipeline();
			pipe->BindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameInfo.index));


			//std::cout << "post-bind textured" << std::endl;
			TextureDesc currentBindedTextureID = TEXTURE_UNBINDED_DESC;

			SimplePushConstantData push{};
			//std::cout << "textured game o bject size : " << objectManager.texturedGameObjects.size() << std::endl;
			for(auto& textureGameObject : objectManager.texturedGameObjects){

				if (textureGameObject.isTarget && (!textureGameObject.activeTarget)) {
					continue;
				}
				if ((textureGameObject.textureID == TEXTURE_UNBINDED_DESC) || (textureGameObject.model == nullptr)) {
					std::cout << "why does a textured game object have no texture, or no model?? " << std::endl;
					continue;
				}
				else if (textureGameObject.textureID != currentBindedTextureID) {

					pipe->BindDescriptor(0, &textureGameObject.textureID);

					currentBindedTextureID = textureGameObject.textureID;
				}
				//printf("drawing now : %d \n", i);
				push.modelMatrix = textureGameObject.transform.mat4();
				push.normalMatrix = textureGameObject.transform.normalMatrix();
				//std::cout << "pre-bind/draw : " << i;

				pipe->BindModel(textureGameObject.model.get());
				pipe->PushAndDraw(&push);
				//std::cout << " ~ post-bind/draw : " << i << std::endl;
			}
		}
		
	}

	void AdvancedRenderSystem::renderVisualEffects(FrameInfo& frameInfo) {

	}
#if DRAWING_POINTS
	void AdvancedRenderSystem::renderPointLights(FrameInfo& frameInfo) {
		if (objectManager.pointLights.size() < 1) {
			return;
		}
		PipelineManager::pipelines[Pipe::pointLight]->bind(frameInfo.frameInfo.cmdBuf);

		if (pointLightDescriptorSet[frameInfo.frameInfo.index] != VK_NULL_HANDLE) {
			vkCmdBindDescriptorSets(
				frameInfo.frameInfo.cmdBuf,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_pointLight],
				0, 1,
				&pointLightDescriptorSet[frameInfo.frameInfo.index],
				0,
				nullptr
			);
		}
		else {
			std::cout << "point light null handle descriptor " << std::endl;
		}

		PointLightPushConstants push{};
		for (int i = 0; i < objectManager.pointLights.size(); i++) {
			push.position = glm::vec4(objectManager.pointLights[i].transform.translation, 1.f);
			push.color = glm::vec4(objectManager.pointLights[i].color, objectManager.pointLights[i].lightIntensity);
			push.radius = objectManager.pointLights[i].transform.scale.x;

			vkCmdPushConstants(frameInfo.frameInfo.cmdBuf, PipelineManager::pipeLayouts[PL_pointLight], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);
			vkCmdDraw(frameInfo.frameInfo.cmdBuf, 6, 1, 0, 0);
		}
		/*
		push.position = glm::vec4(objectManager.sunPoint.transform.translation, 1.f);
		push.color = glm::vec4(objectManager.sunPoint.color, objectManager.sunPoint.lightIntensity);
		push.radius = objectManager.sunPoint.transform.scale.x;

		vkCmdPushConstants(frameInfo.frameInfo.cmdBuf, pointlightLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);
		vkCmdDraw(frameInfo.frameInfo.cmdBuf, 6, 1, 0, 0);
		*/
	}
#endif

	void AdvancedRenderSystem::RenderLightning(FrameInfo const& frameInfo) {
		//printf("beginning render lightning \n");
		//i need to pull this data out of the voids gaze source, if it still exists (rip)
	}
	void AdvancedRenderSystem::RenderGrass(FrameInfo const& frameInfo, float time) {

		if (objectManager.grassField.size() == 0) { return; }
#if DEBUGGING_PIPELINES
		printf("Drawing grass \n");
#endif
		PipelineSystem* pipe = PipelineSystem::At(Pipe::grass);
		pipe->BindPipeline();
		pipe->BindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameInfo.index));
		pipe->BindDescriptor(1, &objectManager.grassTextureID);


		UVScrollingPushData push{ glm::vec2{glm::mod(time / 6.f, 1.f), glm::mod(time / 9.f, 1.f)} };
		pipe->Push(&push);

		for (const auto& grassField : objectManager.grassField) {
			pipe->DrawInstanced(grassField.model.get());
		}
#if DEBUGGING_PIPELINES
		printf("after drawing grass \n");
#endif
	}

	void AdvancedRenderSystem::render2DGameObjects(FrameInfo const& frameInfo, bool menuActive) {

		//printf("beginning r2d : %d \n", frameInfo.menuActive);
		//if (frameInfo.menuActive || GameUI::GetActive()) {
		if (!menuActive) {
			
			if (uiHandler->overlay) {
				if (!uiHandler->overlay->getActive()) {
					return;
				}
			}
			else {
				return;
			}
			
		}

		if (menuActive) {
			Dimension2::Bind2D(frameInfo.cmdBuf, frameInfo.index);
			if (menuManager.drawingNineUI()) {
				menuManager.drawNewNine();
			}
			
			menuManager.drawNewMenuObejcts();

			if (uiHandler->overlay) {
				uiHandler->overlay->drawObjects(frameInfo);
			}
		}
		else {
			if (uiHandler->overlay) {
				Dimension2::Bind2D(frameInfo.cmdBuf, frameInfo.index);
				uiHandler->overlay->drawObjects(frameInfo);
			}
		}
	}
}