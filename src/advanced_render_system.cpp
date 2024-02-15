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



#define DEBUGGING_PIPELINES false
#define DEBUGGING_DYNAMIC_PIPE false


//#define GRASS_ENABLED false

namespace EWE {

	//descriptorsetlayouts are in a vector vector
	//the first layert of the vector (vector<vector>>) designates a pipeline
	//the second layer (the vector inside the vector) designates the descriptorsets in that pipeline

	AdvancedRenderSystem::AdvancedRenderSystem(EWEDevice& device, ObjectManager& objectManager, MenuManager& menuManager) : eweDevice{ device }, objectManager{ objectManager }, menuManager{ menuManager } {
		printf("ARS constructor \n");
		EWEDescriptorPool::BuildGlobalPool(device);

		model2D = Basic_Model::generate2DQuad(device);
		printf("after ARS constructor finished \n");
	}

	AdvancedRenderSystem::~AdvancedRenderSystem() {
#if true//DECONSTRUCTION_DEBUG
		std::cout << "entering ARS deconstructor " << std::endl;
#endif
		EWEPipeline::cleanShaderModules(eweDevice);

		EWEDescriptorPool::DestructPools();


#if true//DECONSTRUCTION_DEBUG
		printf("end of ARS deconstructor \n");
#endif
	}
	void AdvancedRenderSystem::updatePipelines() {

		std::set<Pipeline_Enum> pipeSet;

#if false//LEVEL_BUILDER
		pipeList.push_back(Pipe_textured);
		pipeList.push_back(Pipe_fbx);
		pipeList.push_back(Pipe_sprite);
		pipeList.push_back(Pipe_bobTrans);
		pipeList.push_back(Pipe_grid);


#endif

		//i need a better way of doing this lol
		if (objectManager.texturedGameObjects.size() > 0) {
			pipeSet.emplace(Pipe_textured);
		}
		if (objectManager.grassField.size() > 0) {
			pipeSet.emplace(Pipe_grass);
		}
		if (objectManager.skybox.first) {
			pipeSet.emplace(Pipe_skybox);
		}

		//pipeList.push_back(Pipe_boneWeapon);
		pipeSet.emplace(Pipe_textured);
		/* i need a better way of doing this
		pipeList.push_back(Pipe_spikyBall);
		pipeList.push_back(Pipe_alpha);
		pipeList.push_back(Pipe_visualEffect);
		pipeList.push_back(Pipe_sprite);
		pipeList.push_back(Pipe_lightning);
		pipeList.push_back(Pipe_orbOverlay);
		*/
		

#if DRAWING_POINTS
		if (objectManager.pointLights.size() > 0) {
			pipeList.push_back(Pipe_pointLights);
		}
#endif

		for (auto& pipe : pipeSet) {
			//PipelineManager::initPipelines(pipe, eweDevice);
		}
		//if (PipelineManager::pipelines.find(Pipe_2d) == PipelineManager::pipelines.end()) {
		//	PipelineManager::initPipelines(pipeRenderInfo, Pipe_2d, eweDevice);
		//	PipelineManager::initPipelines(pipeRenderInfo, Pipe_NineUI, eweDevice);
		//}

		//printf("returning from update pipelines \n");
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

		RigidRenderingSystem::render(frameInfo);
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
			PipelineManager::pipelines[Pipe_sprite]->bind(frameInfo.commandBuffer);
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
		if (!objectManager.skybox.first) {
			printf("skybox model null ptr? \n");
			throw std::runtime_error("skybox nullptr");
			return;
		}
#if DEBUGGING_PIPELINES
		printf("drawing skybox \n");
#endif
		//skyboxPipeline->bind(frameInfo.frameInfo.cmdBuf);
		auto pipe = PipelineSystem::at(Pipe_skybox);
		pipe->bindPipeline();
		pipe->bindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameInfo.index));
		pipe->bindDescriptor(1, Texture_Manager::getDescriptorSet(objectManager.skybox.second));

		pipe->bindModel(objectManager.skybox.first.get());
		pipe->drawModel();
		
	}

	inline void AdvancedRenderSystem::renderTexturedGameObjects(FrameInfo& frameInfo) {
		bool texturePipeBinded = false;
		if ((objectManager.texturedGameObjects.size() > 0)) {
			auto pipe = PipelineSystem::at(Pipe_textured);
#if DEBUGGING_PIPELINES
			printf("Drawing texutered game objects \n");
#endif
			//texturedPipeline->bind(frameInfo.frameInfo.cmdBuf);
			texturePipeBinded = true;
			pipe->bindPipeline();
			pipe->bindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameInfo.index));


			//std::cout << "post-bind textured" << std::endl;
			int currentBindedTextureID = -1;

			SimplePushConstantData push{};
			//std::cout << "textured game o bject size : " << objectManager.texturedGameObjects.size() << std::endl;
			for(auto& textureGameObject : objectManager.texturedGameObjects){

				if (textureGameObject.isTarget && (!textureGameObject.activeTarget)) {
					continue;
				}
				if ((textureGameObject.textureID == TEXTURE_UNBINDED) || (textureGameObject.model == nullptr)) {
					std::cout << "why does a textured game object have no texture, or no model?? " << std::endl;
					continue;
				}
				else if (textureGameObject.textureID != currentBindedTextureID) {

					pipe->bindDescriptor(0, Texture_Manager::getDescriptorSet(textureGameObject.textureID));

					currentBindedTextureID = textureGameObject.textureID;
				}
				//printf("drawing now : %d \n", i);
				push.modelMatrix = textureGameObject.transform.mat4();
				push.normalMatrix = textureGameObject.transform.normalMatrix();
				//std::cout << "pre-bind/draw : " << i;

				pipe->bindModel(textureGameObject.model.get());
				pipe->pushAndDraw(&push);
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
		PipelineManager::pipelines[Pipe_pointLight]->bind(frameInfo.frameInfo.cmdBuf);

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
		PipelineSystem* pipe = PipelineSystem::at(Pipe_grass);
		pipe->bindPipeline();
		pipe->bindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameInfo.index));
		pipe->bindDescriptor(1, Texture_Manager::getDescriptorSet(objectManager.grassTextureID));


		UVScrollingPushData push{ glm::vec2{glm::mod(time / 6.f, 1.f), glm::mod(time / 9.f, 1.f)} };
		pipe->push(&push);

		for (const auto& grassField : objectManager.grassField) {
			pipe->drawInstanced(grassField.model.get());
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
		bool pipe2dBinded = false;
		if (menuActive) {
			if (menuManager.drawingNineUI()) {
				Dimension2::bindNineUI(frameInfo.cmdBuf, frameInfo.index);
				menuManager.drawNewNine();
				//PipelineManager::pipelines[Pipe_NineUI]->bind(frameInfo.frameInfo.cmdBuf);
				//frameInfo.currentlyBindedTexture = TEXTURE_UNBINDED;
				//menuManager.drawNineUI(frameInfo);
			}
			
			//PipelineManager::pipelines[Pipe_2d]->bind(frameInfo.frameInfo.cmdBuf);
			pipe2dBinded = true;
			//frameInfo.currentlyBindedTexture = TEXTURE_UNBINDED;
			//menuManager.drawMenuObjects(frameInfo);
			Dimension2::bind2D(frameInfo.cmdBuf, frameInfo.index);
			menuManager.drawNewMenuObejcts();
		}


		//printf("binding textures from in game even if game isnt active \n");
		if (uiHandler->overlay) {
			if (!pipe2dBinded) {
				Dimension2::bind2D(frameInfo.cmdBuf, frameInfo.index);
				//PipelineManager::pipelines[Pipe_2d]->bind(frameInfo.frameInfo.cmdBuf);
				//frameInfo.currentlyBindedTexture = TEXTURE_UNBINDED;
			}
			uiHandler->overlay->drawObjects(frameInfo);
		}
	}


//#define LB_DEBUGGING true
#if LEVEL_BUILDER

	void AdvancedRenderSystem::renderBuilderObjects(FrameInfo& frameInfo) {
		//printf("before simle builds \n");
		/* THIS WILL CAUSE ISSUES NEXT TIME I TRY LEVEL LOADER
		if (objectManager.builderObjects.size() > 0) {
			renderSimpleGameObjects(frameInfo);
		}
		*/
		//printf("before textured build \n");

		/* SIMPLE OBJECTS ARE NOT CURRENTLY SUPPORTED, ILL GET BACK TO THESE, MAYBE
		if (objectManager.builderObjects.size() > 0) {
			PipelineManager::pipelines[Pipe_]
		}
		*/
		if(objectManager.dynamicBuildObjects.size() > 0) {


			const std::map<MaterialFlags, std::map<TextureID, std::vector<MaterialInfo>>>& matMapTemp = RigidRenderingSystemInstance->getMaterialMap(); //not clean and get because shit shouldnt be deleted runtime??? at least not currently
			for (auto iter = matMapTemp.begin(); iter != matMapTemp.end(); iter++) {
#if DEBUGGING_DYNAMIC_PIPE
				printf("checking validity of map iter? \n");
				printf("iter->first:second - %d:%d \n", iter->first, iter->second.size());
#endif

				if (iter->second.size() == 0) {
					continue;
				}
				uint8_t flags = iter->first;
#if DEBUGGING_DYNAMIC_PIPE || DEBUGGING_PIPELINES
				printf("Drawing dynamic materials : %d \n", flags);
#endif

#ifdef _DEBUG
				if (PipelineManager::dynamicMaterialPipeline.find(flags) == PipelineManager::dynamicMaterialPipeline.end()) {
					printf("pipeline[%d] not found (uint8t:%d) \n", iter->first, flags);
				}
#endif

				PipelineManager::dynamicMaterialPipeline[flags]->bind(frameInfo.frameInfo.cmdBuf);

				uint8_t pipeLayoutIndex = ((flags & 16) >> 4) + ((flags & 8) >> 3) + ((flags & 4) >> 2) + ((flags & 2) >> 1) + (flags & 1) + (((flags & 128) >> 7) * 6);
				/*
				printf("dynamic material pipeLayoutIndex:hasBump : %d:%d \n", pipeLayoutIndex, flags & 16);
				if ((flags & 16)) {
					printf("has bump and - %d:%d:%d:%d:%d \n",(flags & 16) >> 4, (flags & 8) >> 3, (flags & 4) >> 2, (flags & 2) >> 1, (flags & 1));
				}
				*/
				//printf("flags:pipeIndex - %d:%d \n", flags, pipeLayoutIndex);
				vkCmdBindDescriptorSets(
					frameInfo.frameInfo.cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex],
					0, 1,
					DescriptorHandler::getDescSet(DS_global, frameInfo.frameInfo.index),
					0,
					nullptr
				);
#if DEBUGGING_DYNAMIC_PIPE
				printf("after binding global \n");
#endif
				if (flags & 128) { //if has bones
					//printf("rendering bones in level builder? \n");
				}
				else {//no bone
					//printf("rrendering non-boned, pipeLayoutIndex : %d \n", pipeLayoutIndex);
					int32_t bindedTexture = -1;
					for (auto iterTexID = iter->second.begin(); iterTexID != iter->second.end(); iterTexID++) {
						if (bindedTexture != iterTexID->first) {
							vkCmdBindDescriptorSets(
								frameInfo.frameInfo.cmdBuf,
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex],
								1, 1,
								EWETexture::getDescriptorSets(iterTexID->first, frameInfo.frameInfo.index),
								0, nullptr
							);
							bindedTexture = iterTexID->first;
						}

						for (int i = 0; i < iterTexID->second.size(); i++) {
							if (((BuilderModel*)iterTexID->second[i].meshPtr)->WantsChange()) {

								continue;
							}
							SimplePushConstantData push{};
							push.modelMatrix = iterTexID->second[i].ownerTransform->mat4();
							push.normalMatrix = iterTexID->second[i].ownerTransform->normalMatrix();

							vkCmdPushConstants(frameInfo.frameInfo.cmdBuf, PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
							iterTexID->second[i].meshPtr->bind(frameInfo.frameInfo.cmdBuf);
							//printf("assimpNT post-bind : %d \n", j);
							iterTexID->second[i].meshPtr->draw(frameInfo.frameInfo.cmdBuf);
						}
					}
				}

#if DEBUGGING_DYNAMIC_PIPE
				printf("finished drawing dynamic material flag : %d \n", flags);
#endif
			}
#if DEBUGGING_PIPELINES || DEBUGGING_DYNAMIC_PIPE
			printf("finished dynamic render \n");
#endif
		}

		/*
		if (objectManager.texturedBuildObjects.size() > 0) {
			printf("before textured objects \n");
			PipelineManager::pipelines[Pipe_textured]->bind(frameInfo.frameInfo.cmdBuf);
			//texturedPipeline->bind(frameInfo.frameInfo.cmdBuf);
			vkCmdBindDescriptorSets(
				frameInfo.frameInfo.cmdBuf,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_textured],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.frameInfo.index),
				//&globalDescriptorSet[frameInfo.frameInfo.index],
				0,
				nullptr
			);

			//std::cout << "post-bind textured" << std::endl;
			int currentBindedTextureID = -1;

			//std::cout << "textured game o bject size : " << objectManager.texturedGameObjects.size() << std::endl;
			for (auto iter = objectManager.texturedBuildObjects.begin(); iter != objectManager.texturedBuildObjects.end(); iter++) {
				//std::cout << "?? [" << i << "] textureGameObject iterator ~ size : " << objectManager.texturedGameObjects.size() << std::endl;
				//std::cout << "value check : " << objectManager.texturedGameObjects[i].isTarget << ":" << (!objectManager.texturedGameObjects[i].activeTarget) << ":" << objectManager.texturedGameObjects[i].textureID << ":" << (objectManager.texturedGameObjects[i].model == nullptr) << std::endl;
				//std::cout << "???? before descriptor" << std::endl;
				if ((iter->second.textureID == -1) || (iter->second.model == nullptr)) {
					std::cout << "why does a textured game object have no texture, or no model?? " << std::endl;
					continue;
				}
				else if (iter->second.textureID != currentBindedTextureID) {
					//std::cout << "texture desriptor set size : " << textureDescriptorSets.size() << std::endl;
					//std::cout << "iterator value : " << iter->second.textureID * 2 + frameInfo.frameInfo.index << std::endl;

					vkCmdBindDescriptorSets(
						frameInfo.frameInfo.cmdBuf,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						PipelineManager::pipeLayouts[PL_textured],
						1, 1,
						//&EWETexture::modeDescriptorSets[iter->second.textureID * 2 + frameInfo.frameInfo.index],
						EWETexture::getDescriptorSets(iter->second.textureID, frameInfo.frameInfo.index),
						0, nullptr
					);
					currentBindedTextureID = iter->second.textureID;
				}
				//printf("drawing now : %d \n", i);
				SimplePushConstantData push{};
				push.modelMatrix = iter->second.transform.mat4();
				push.normalMatrix = iter->second.transform.normalMatrix();
				//std::cout << "pre-bind/draw : " << i;
				vkCmdPushConstants(frameInfo.frameInfo.cmdBuf, PipelineManager::pipeLayouts[PL_textured], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
				iter->second.model->bind(frameInfo.frameInfo.cmdBuf);
				iter->second.model->draw(frameInfo.frameInfo.cmdBuf);
				//std::cout << " ~ post-bind/draw : " << i << std::endl;
			}
		}
		//printf("before material objects \n");
		if (objectManager.materialBuildObjects.size() > 0) {
			printf("beginning material build object draw \n");
			PipelineManager::pipelines[Pipe_fbx]->bind(frameInfo.frameInfo.cmdBuf);
			vkCmdBindDescriptorSets(
				frameInfo.frameInfo.cmdBuf,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_fbx],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.frameInfo.index),
				0,
				nullptr
			);
			for (auto iter = objectManager.materialBuildObjects.begin(); iter != objectManager.materialBuildObjects.end(); iter++) {
				if ((iter->second.textureID == -1) || (iter->second.model == nullptr)) {
					std::cout << "why does a textured game object have no texture, or no model?? " << std::endl;
					continue;
				}
				SimplePushConstantData push{};
				push.modelMatrix = iter->second.transform.mat4();
				push.normalMatrix = iter->second.transform.normalMatrix();

				//printf("before binding texture assimp \n");
				vkCmdBindDescriptorSets(
					frameInfo.frameInfo.cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_fbx],
					1, 1,
					EWETexture::getDescriptorSets(iter->second.textureID, frameInfo.frameInfo.index),
					0, nullptr
				);
				//std::cout << "predraw assimp" << std::endl;

				vkCmdPushConstants(frameInfo.frameInfo.cmdBuf, PipelineManager::pipeLayouts[PL_fbx], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
				iter->second.model->bind(frameInfo.frameInfo.cmdBuf);
				iter->second.model->draw(frameInfo.frameInfo.cmdBuf);
				//std::cout << "post draw assimp" << std::endl;
				
			}
			printf("end material object draw \n");
		}
		//printf("before render poitns \n");
		//if (shouldRenderPoints) { //shouldRenderPoints) {
			//renderPointLights(frameInfo);
		//}
		//printf("before transparent objects \n");
		if (objectManager.transparentBuildObjects.size() > 0) {
			PipelineManager::pipelines[Pipe_bobTrans]->bind(frameInfo.frameInfo.cmdBuf);
			vkCmdBindDescriptorSets(
				frameInfo.frameInfo.cmdBuf,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_fbx],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.frameInfo.index),
				0,
				nullptr
			);
			for (auto iter = objectManager.transparentBuildObjects.begin(); iter != objectManager.transparentBuildObjects.end(); iter++) {
				if ((iter->second.textureID == -1) || (iter->second.model == nullptr)) {
					std::cout << "why does a textured game object have no texture, or no model?? " << std::endl;
					continue;
				}
				SimplePushConstantData push{};
				push.modelMatrix = iter->second.transform.mat4();
				push.normalMatrix = iter->second.transform.normalMatrix();

				//printf("before binding texture assimp \n");
				vkCmdBindDescriptorSets(
					frameInfo.frameInfo.cmdBuf,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_fbx],
					1, 1,
					EWETexture::getDescriptorSets(iter->second.textureID, frameInfo.frameInfo.index),
					0, nullptr
				);
				//std::cout << "predraw assimp" << std::endl;

				vkCmdPushConstants(frameInfo.frameInfo.cmdBuf, PipelineManager::pipeLayouts[PL_fbx], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
				iter->second.model->bind(frameInfo.frameInfo.cmdBuf);
				iter->second.model->draw(frameInfo.frameInfo.cmdBuf);
				//std::cout << "post draw assimp" << std::endl;

			}
		}
		*/
		//printf("before rendering sprites \n");
		if(objectManager.spriteBuildObjects.size() > 0) {

			PipelineManager::pipelines[Pipe_sprite]->bind(frameInfo.frameInfo.cmdBuf);
			vkCmdBindDescriptorSets(
				frameInfo.frameInfo.cmdBuf,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_sprite],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.frameInfo.index),
				0,
				nullptr
			);
			uint32_t currentBindedTextureID = -1;

			for (auto iter = objectManager.spriteBuildObjects.begin(); iter != objectManager.spriteBuildObjects.end(); iter++) {
				if ((iter->second.textureID == -1) || (iter->second.model == nullptr)) {
					std::cout << "why does a textured game object have no texture, or no model?? " << std::endl;
					continue;
				}
				else if (iter->second.textureID != currentBindedTextureID) {
					//std::cout << "texture desriptor set size : " << textureDescriptorSets.size() << std::endl;
					//std::cout << "iterator value : " << iter->second.textureID * 2 + frameInfo.frameInfo.index << std::endl;

					vkCmdBindDescriptorSets(
						frameInfo.frameInfo.cmdBuf,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						PipelineManager::pipeLayouts[PL_sprite],
						1, 1,
						//&EWETexture::modeDescriptorSets[iter->second.textureID * 2 + frameInfo.frameInfo.index],
						EWETexture::getSpriteDescriptorSets(iter->second.textureID, frameInfo.frameInfo.index),
						0, nullptr
					);
					currentBindedTextureID = iter->second.textureID;
				}
				//printf("drawing now : %d \n", i);
				SimplePushConstantData push{};
				push.modelMatrix = iter->second.transform.mat4();
				push.normalMatrix = iter->second.transform.normalMatrix();
				//std::cout << "pre-bind/draw : " << i;
				vkCmdPushConstants(frameInfo.frameInfo.cmdBuf, PipelineManager::pipeLayouts[PL_sprite], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
				iter->second.model->bind(frameInfo.frameInfo.cmdBuf);
				iter->second.model->draw(frameInfo.frameInfo.cmdBuf);
			}


			EWETexture::newSpriteFrame();
		}
		//printf("after rendering sprites \n");
		if (objectManager.lBuilderObjects.size() > 0) {
			PipelineManager::pipelines[Pipe_grid]->bind(frameInfo.frameInfo.cmdBuf);

			//printf("rendering lbuilder objects \n");
			//i wont have to bind this descriptor set if the playerobject branch is open, but I don't think it should happen right now, not worth checking
			vkCmdBindDescriptorSets(
				frameInfo.frameInfo.cmdBuf,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_textured],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.frameInfo.index),
				0,
				nullptr
			);
			uint32_t lastTextureID = -1;

			for (auto iter = objectManager.lBuilderObjects.begin(); iter != objectManager.lBuilderObjects.end(); iter++) {
				if (!iter->second.drawable) {
					continue;
				}
				if (iter->second.textureID != lastTextureID) {
					vkCmdBindDescriptorSets(
						frameInfo.frameInfo.cmdBuf,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						PipelineManager::pipeLayouts[PL_textured],
						1, 1,
						EWETexture::getDescriptorSets(iter->second.textureID, frameInfo.frameInfo.index),
						0, nullptr
					);
					lastTextureID = iter->second.textureID;
				}

				SimplePushConstantData push{};
				push.modelMatrix = iter->second.transform.mat4();
				push.normalMatrix = iter->second.transform.normalMatrix();

				vkCmdPushConstants(frameInfo.frameInfo.cmdBuf, PipelineManager::pipeLayouts[PL_textured] , VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
				iter->second.model->bind(frameInfo.frameInfo.cmdBuf);
				iter->second.model->draw(frameInfo.frameInfo.cmdBuf);
				//printf("rendering a builder object? \n");
			}

			//printf("end of rendering lbuilder objects \n");
		}
		//printf("after rendering the grid \n");
	}
#endif
}