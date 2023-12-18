#include "EWEngine/systems/advanced_render_system.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <stdexcept>
#include <iostream>


#define DEBUGGING_PIPELINES false
#define DEBUGGING_DYNAMIC_PIPE false

#define GLOBAL_POOL_MAX_SETS 2000

//#define GRASS_ENABLED false

namespace EWE {

	//descriptorsetlayouts are in a vector vector
	//the first layert of the vector (vector<vector>>) designates a pipeline
	//the second layer (the vector inside the vector) designates the descriptorsets in that pipeline

	AdvancedRenderSystem::AdvancedRenderSystem(EWEDevice& device, VkPipelineRenderingCreateInfo const& pipeRenderInfo, ObjectManager& objectManager, MenuManager& menuManager) : eweDevice{ device }, objectManager{ objectManager }, menuManager{ menuManager } {
		printf("ARS constructor \n");
		initGlobalPool(GLOBAL_POOL_MAX_SETS);
		EWETexture::buildSetLayouts(device);
		EWETexture::setGlobalPool(globalPool);
		PipelineManager::initStaticVariables();

		model2D = EWEModel::generate2DQuad(device);
		materialHandlerInstance = MaterialHandler::getMaterialHandlerInstance();
		printf("after ARS constructor finished \n");
	}

	AdvancedRenderSystem::~AdvancedRenderSystem() {
#if true//DECONSTRUCTION_DEBUG
		std::cout << "entering ARS deconstructor " << std::endl;
#endif

		PipelineManager::cleanupStaticVariables(eweDevice);
		EWEPipeline::cleanShaderModules(eweDevice);

		//globalPool->resetPool();
		//globalPool->~EWEDescriptorPool();
		globalPool.reset();
		//vkDestroyCommandPool(eweDevice.device, globalPool->)
#if true//DECONSTRUCTION_DEBUG
		printf("end of ARS deconstructor \n");
#endif
	}
	void AdvancedRenderSystem::updateLoadingPipeline(VkPipelineRenderingCreateInfo const& pipeRenderInfo) {
		PipelineManager::initPipelines(pipeRenderInfo, Pipe_loading, eweDevice);
	}
	void AdvancedRenderSystem::updatePipelines(ObjectManager& objectManager, VkPipelineRenderingCreateInfo const& pipeRenderInfo) {

		std::list<Pipeline_Enum> pipeList;

#if false//LEVEL_BUILDER
		pipeList.push_back(Pipe_textured);
		pipeList.push_back(Pipe_fbx);
		pipeList.push_back(Pipe_sprite);
		pipeList.push_back(Pipe_bobTrans);
		pipeList.push_back(Pipe_grid);


#endif

		//i need a better way of doing this lol
		if (objectManager.texturedGameObjects.size() > 0) {
			pipeList.push_back(Pipe_textured);
		}
		if (objectManager.grassField.size() > 0) {
			pipeList.push_back(Pipe_grass);
		}
		if (objectManager.skybox.first) {
			pipeList.push_back(Pipe_skybox);
		}

		//pipeList.push_back(Pipe_boneWeapon);
		pipeList.push_back(Pipe_textured);
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

		pipeList.sort();
		pipeList.unique();
		for (auto listIter = pipeList.begin(); listIter != pipeList.end(); listIter++) {
			PipelineManager::initPipelines(pipeRenderInfo, *listIter, eweDevice);
		}
		if (PipelineManager::pipelines.find(Pipe_2d) == PipelineManager::pipelines.end()) {
			PipelineManager::initPipelines(pipeRenderInfo, Pipe_2d, eweDevice);
			PipelineManager::initPipelines(pipeRenderInfo, Pipe_NineUI, eweDevice);
		}

		updateMaterialPipelines(pipeRenderInfo);

		//printf("returning from update pipelines \n");
	}

	void AdvancedRenderSystem::renderGameObjects(FrameInfo &frameInfo) {
#if DEBUGGING_PIPELINES
		printf("getting into render game objects \n");
#endif

		renderSkybox(frameInfo);
	
		//renderSimpleGameObjects(frameInfo);
		//renderBonedWeapons(frameInfo);
		
		renderTexturedGameObjects(frameInfo);

#if RENDERING_EIGHT_WINDS
		RenderSpikyball(frameInfo);
#endif

		RenderDynamicMaterials(frameInfo);
#if DEBUGGING_PIPELINES
		printf("after rendering dynamic \n");
#endif

		renderVisualEffects(frameInfo);

		RenderLightning(frameInfo);

		if (shouldRenderPoints){ //shouldRenderPoints) {
#if DRAWING_POINTS
			renderPointLights(frameInfo);
#endif
			//printf("rendering points \n");
		}


		//transparency always needs to be last
		//renderSprites(frameInfo);
#if RENDERING_EIGHT_WINDS
		if (PlayerObject::poVector.size() > 0) {
			bool playerTransparent = false;
			for (int i = 0; i < PlayerObject::poVector.size(); i++) {
				if (PlayerObject::poVector[i].DrawBorD()) {
					playerTransparent = true;
					break;
				}
			}
			if (playerTransparent) {
				renderTransparentGameObjects(frameInfo);
			}
		}
#endif

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
			throw std::exception("skybox nullptr");
			return;
		}
#if DEBUGGING_PIPELINES
		printf("drawing skybox \n");
#endif
		//skyboxPipeline->bind(frameInfo.cmdIndexPair.first);
		PipelineManager::pipelines[Pipe_skybox]->bind(frameInfo.cmdIndexPair.first);
		vkCmdBindDescriptorSets(
			frameInfo.cmdIndexPair.first,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			PipelineManager::pipeLayouts[PL_skybox],
			0, 1,
			DescriptorHandler::getDescSet(DS_global, frameInfo.cmdIndexPair.second),
			0,
			nullptr
		);
		
		vkCmdBindDescriptorSets(
			frameInfo.cmdIndexPair.first,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			PipelineManager::pipeLayouts[PL_skybox],
			1, 1,
			EWETexture::getSkyboxDescriptorSets(frameInfo.cmdIndexPair.second),
			0,
			nullptr
		);

		objectManager.skybox.first->bind(frameInfo.cmdIndexPair.first);
		objectManager.skybox.first->draw(frameInfo.cmdIndexPair.first);
		
	}

	inline void AdvancedRenderSystem::renderTexturedGameObjects(FrameInfo& frameInfo) {
		bool texturePipeBinded = false;
		if ((objectManager.texturedGameObjects.size() > 0)) {
#if DEBUGGING_PIPELINES
			printf("Drawing texutered game objects \n");
#endif
			//texturedPipeline->bind(frameInfo.cmdIndexPair.first);
			texturePipeBinded = true;
			PipelineManager::pipelines[Pipe_textured]->bind(frameInfo.cmdIndexPair.first);

			vkCmdBindDescriptorSets(
				frameInfo.cmdIndexPair.first,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_textured],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.cmdIndexPair.second),
				0,
				nullptr
			);


			//std::cout << "post-bind textured" << std::endl;
			int currentBindedTextureID = -1;

			//std::cout << "textured game o bject size : " << objectManager.texturedGameObjects.size() << std::endl;
			for (int i = 0; i < objectManager.texturedGameObjects.size(); i++) {
				if (objectManager.texturedGameObjects[i].isTarget && (!objectManager.texturedGameObjects[i].activeTarget)) {
					continue;
				}
				if ((objectManager.texturedGameObjects[i].textureID == -1) || (objectManager.texturedGameObjects[i].model == nullptr)) {
					std::cout << "why does a textured game object have no texture, or no model?? " << std::endl;
					continue;
				}
				else if (objectManager.texturedGameObjects[i].textureID != currentBindedTextureID) {

					vkCmdBindDescriptorSets(
						frameInfo.cmdIndexPair.first,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						PipelineManager::pipeLayouts[PL_textured],
						1, 1,
						EWETexture::getDescriptorSets(objectManager.texturedGameObjects[i].textureID, frameInfo.cmdIndexPair.second),
						0, nullptr
					);
					currentBindedTextureID = objectManager.texturedGameObjects[i].textureID;
				}
				//printf("drawing now : %d \n", i);
				SimplePushConstantData push{};
				push.modelMatrix = objectManager.texturedGameObjects[i].transform.mat4();
				push.normalMatrix = objectManager.texturedGameObjects[i].transform.normalMatrix();
				//std::cout << "pre-bind/draw : " << i;
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_textured], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
				objectManager.texturedGameObjects[i].model->bind(frameInfo.cmdIndexPair.first);
				objectManager.texturedGameObjects[i].model->draw(frameInfo.cmdIndexPair.first);
				//std::cout << " ~ post-bind/draw : " << i << std::endl;
			}
		}
		
	}
	void AdvancedRenderSystem::RenderDynamicMaterials(FrameInfo& frameInfo) {
		//ill replace this shit eventually


		const std::map<ShaderFlags, std::map<TextureID, std::vector<MaterialInfo>>>& matMapTemp = materialHandlerInstance->getMaterialMap(); //not clean and get because shit shouldnt be deleted runtime??? at least not currently
		for(auto iter = matMapTemp.begin(); iter != matMapTemp.end(); iter++) {
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
			if (flags & 128) {
				printf("should not have bonesin static rendering \n");
				throw std::exception("should not have boens here");
			}
			PipelineManager::dynamicMaterialPipeline[flags]->bind(frameInfo.cmdIndexPair.first);

			uint8_t pipeLayoutIndex = ( 
				((flags & 16) >> 4) + 
				((flags & 8) >> 3) + 
				((flags & 4) >> 2) + 
				((flags & 2) >> 1) + 
				(flags & 1) +
				(((flags & 128) >> 7) * MAX_SMART_TEXTURE_COUNT) +
				(((flags & 64) >> 6) * MAX_SMART_TEXTURE_COUNT * 2)	
			);
			/*
			printf("dynamic material pipeLayoutIndex:hasBump : %d:%d \n", pipeLayoutIndex, flags & 16);
			if ((flags & 16)) {
				printf("has bump and - %d:%d:%d:%d:%d \n",(flags & 16) >> 4, (flags & 8) >> 3, (flags & 4) >> 2, (flags & 2) >> 1, (flags & 1));
			}
			*/
			//printf("flags:pipeIndex - %d:%d \n", flags, pipeLayoutIndex);
			vkCmdBindDescriptorSets(
				frameInfo.cmdIndexPair.first,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.cmdIndexPair.second),
				0,
				nullptr
			);
#if DEBUGGING_DYNAMIC_PIPE
			printf("after binding global \n");
#endif
			/*
			if (flags & 128) { //if has bones
#if DEBUGGING_DYNAMIC_PIPE
				printf("after binding bones \n");
#endif
				int32_t bindedTexture = -1;

				//experiment with shifting the bone descriptor set index ahead of the texture set index,
				//i.e. bonedescriptor 2 -> 1, texturedescriptor 1->2
				//for (int i = 0; i < iter->second.size(); i++) {
				for(auto iterTexID = iter->second.begin(); iterTexID != iter->second.end(); iterTexID++){
#ifdef _DEBUG
					if(iterTexID->second.size() == 0){
						printf("SHOULD NOT HAVE A 0 SIZE VECTOR THATS STILL MAPPED \n");
						continue;
					}
#endif
					if (iterTexID->second[0].actorType != Actor_None) {
						if (bindedTexture != iterTexID->first) {
							vkCmdBindDescriptorSets(
								frameInfo.cmdIndexPair.first,
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex],
								2, 1,
								EWETexture::getDescriptorSets(iterTexID->first, frameInfo.cmdIndexPair.second),
								0, nullptr
							);
							bindedTexture = iterTexID->first;
						}
					}
					else {
						if (bindedTexture != iterTexID->first) {
							vkCmdBindDescriptorSets(
								frameInfo.cmdIndexPair.first,
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex],
								1, 1,
								EWETexture::getDescriptorSets(iterTexID->first, frameInfo.cmdIndexPair.second),
								0, nullptr
							);
							bindedTexture = iterTexID->first;
						}
					}

					Actor_Type boneDescriptorBinded = Actor_None;
					for (int i = 0; i < iterTexID->second.size(); i++) {
						if (iterTexID->second[i].actorType != Actor_None) {
#if DEBUGGING_DYNAMIC_PIPE
							printf("!MOI NONE \n");
							printf("before binding texture : %d \n", iterTexID->first);
#endif

#if DEBUGGING_DYNAMIC_PIPE
							printf("after binding texture \n");
#endif

#if DEBUGGING_DYNAMIC_PIPE
							printf("before binding player mesh \n");
#endif
							*
							for (int j = 0; j < PlayerObject::poVector.size(); j++) {
								if (iterTexID->second[i].actorType == Player_character) {
#if DEBUGGING_DYNAMIC_PIPE
									printf("pushing and drawing player \n");
#endif
									vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PlayerPushConstantData), &playerPush[j]);
									iterTexID->second[i].meshPtr->draw(frameInfo.cmdIndexPair.first);
								}
								else if (iterTexID->second[i].actorType == Player_weapon) {
#if DEBUGGING_DYNAMIC_PIPE
									printf("pushing and drawing player weapon \n");
#endif

									vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PlayerPushConstantData), &weaponPush[j]);
									iterTexID->second[i].meshPtr->draw(frameInfo.cmdIndexPair.first);
								}
							}
							*
							if (iterTexID->second[i].actorType == Player_character) {
								iterTexID->second[i].meshPtr->bind(frameInfo.cmdIndexPair.first);
#if DEBUGGING_DYNAMIC_PIPE
								printf("pushing and drawing player \n");
#endif

								if (boneDescriptorBinded != Player_character) {
									vkCmdBindDescriptorSets(
										frameInfo.cmdIndexPair.first,
										VK_PIPELINE_BIND_POINT_GRAPHICS,
										PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex],
										1, 1,
										//&bonedDescriptorSet[frameInfo.cmdIndexPair.second * 2 + j],
										DescriptorHandler::getDescSet(DS_boned, frameInfo.cmdIndexPair.second),
										0,
										nullptr
									);

									boneDescriptorBinded = Player_character;
								}
								vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PlayerPushConstantData), &playerPush);
								iterTexID->second[i].meshPtr->draw(frameInfo.cmdIndexPair.first);

							}
							else if (iterTexID->second[i].actorType == Player_weapon) {
								iterTexID->second[i].meshPtr->bind(frameInfo.cmdIndexPair.first);
#if DEBUGGING_DYNAMIC_PIPE
								printf("pushing and drawing player weapon \n");
#endif
#if RENDERING_EIGHT_WINDS
								if (!playerBoneDescriptorBinded) {
									vkCmdBindDescriptorSets(
										frameInfo.cmdIndexPair.first,
										VK_PIPELINE_BIND_POINT_GRAPHICS,
										PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex],
										1, 1,
										//&bonedDescriptorSet[frameInfo.cmdIndexPair.second * 2 + j],
										DescriptorHandler::getDescSet(DS_boned, frameInfo.cmdIndexPair.second),
										0,
										nullptr
									);
								}

								vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PlayerPushConstantData), &weaponPush);
								iterTexID->second[i].meshPtr->draw(frameInfo.cmdIndexPair.first);
#else
								if (boneDescriptorBinded != Player_character) {
									vkCmdBindDescriptorSets(
										frameInfo.cmdIndexPair.first,
										VK_PIPELINE_BIND_POINT_GRAPHICS,
										PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex],
										1, 1,
										//&bonedDescriptorSet[frameInfo.cmdIndexPair.second * 2 + j],
										DescriptorHandler::getDescSet(DS_boned, frameInfo.cmdIndexPair.second),
										0,
										nullptr
									);
									boneDescriptorBinded = Player_character;
								}
								vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PlayerPushConstantData), &weaponPush);
								iterTexID->second[i].meshPtr->draw(frameInfo.cmdIndexPair.first);
#endif
							}
							else if (iterTexID->second[i].actorType > Player_weapon) {

								if (flags & 64) {
									bool renderedActorType = levelManager->drawActorTypeWithMesh(
										frameInfo.cmdIndexPair.first, 
										iterTexID->second[i].actorType, 
										iterTexID->second[i].meshPtr, 
										iterTexID->second[i].actorType == boneDescriptorBinded, 
										frameInfo.cmdIndexPair.second, pipeLayoutIndex);

									if (renderedActorType) {
										boneDescriptorBinded = iterTexID->second[i].actorType;
									}
								}
								else {
									printf("Drawing monster without it being instaced \n");
									throw std::exception("invalid draw, monster no instance");
								}
							}
						}
						else {

							SimplePushConstantData push{};
							push.modelMatrix = iterTexID->second[i].ownerTransform->mat4();
							push.normalMatrix = iterTexID->second[i].ownerTransform->normalMatrix();

							vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
							iterTexID->second[i].meshPtr->bind(frameInfo.cmdIndexPair.first);
							//printf("assimpNT post-bind : %d \n", j);
							iterTexID->second[i].meshPtr->draw(frameInfo.cmdIndexPair.first);
						}
					}
				}
			}
			else {
				//no bone
				//printf("rrendering non-boned, pipeLayoutIndex : %d \n", pipeLayoutIndex);
				*/
				int32_t bindedTexture = -1;
				for (auto iterTexID = iter->second.begin(); iterTexID != iter->second.end(); iterTexID++) {
					if (bindedTexture != iterTexID->first) {
						vkCmdBindDescriptorSets(
							frameInfo.cmdIndexPair.first,
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex],
							1, 1,
							EWETexture::getDescriptorSets(iterTexID->first, frameInfo.cmdIndexPair.second),
							0, nullptr
						);
						bindedTexture = iterTexID->first;
					}



					for (int i = 0; i < iterTexID->second.size(); i++) {

						if (!(*iterTexID->second[i].drawable)) { continue; }
						SimplePushConstantData push{ iterTexID->second[i].ownerTransform->mat4(), iterTexID->second[i].ownerTransform->normalMatrix() };

						vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
						iterTexID->second[i].meshPtr->bind(frameInfo.cmdIndexPair.first);
						//printf("assimpNT post-bind : %d \n", j);
						iterTexID->second[i].meshPtr->draw(frameInfo.cmdIndexPair.first);
					}
				}
			//}

#if DEBUGGING_DYNAMIC_PIPE
			printf("finished drawing dynamic material flag : %d \n", flags);
#endif
		}
#if DEBUGGING_PIPELINES || DEBUGGING_DYNAMIC_PIPE
		printf("finished dynamic render \n");
#endif
	}
	void AdvancedRenderSystem::renderVisualEffects(FrameInfo& frameInfo) {

	}
#if DRAWING_POINTS
	void AdvancedRenderSystem::renderPointLights(FrameInfo& frameInfo) {
		if (objectManager.pointLights.size() < 1) {
			return;
		}
		PipelineManager::pipelines[Pipe_pointLight]->bind(frameInfo.cmdIndexPair.first);

		if (pointLightDescriptorSet[frameInfo.cmdIndexPair.second] != VK_NULL_HANDLE) {
			vkCmdBindDescriptorSets(
				frameInfo.cmdIndexPair.first,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_pointLight],
				0, 1,
				&pointLightDescriptorSet[frameInfo.cmdIndexPair.second],
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

			vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_pointLight], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);
			vkCmdDraw(frameInfo.cmdIndexPair.first, 6, 1, 0, 0);
		}
		/*
		push.position = glm::vec4(objectManager.sunPoint.transform.translation, 1.f);
		push.color = glm::vec4(objectManager.sunPoint.color, objectManager.sunPoint.lightIntensity);
		push.radius = objectManager.sunPoint.transform.scale.x;

		vkCmdPushConstants(frameInfo.cmdIndexPair.first, pointlightLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);
		vkCmdDraw(frameInfo.cmdIndexPair.first, 6, 1, 0, 0);
		*/
	}
#endif

	void AdvancedRenderSystem::RenderLightning(FrameInfo& frameInfo) {
		//printf("beginning render lightning \n");
		//i need to pull this data out of the voids gaze source, if it still exists (rip)
	}
	void AdvancedRenderSystem::RenderGrass(FrameInfo& frameInfo) {

		if (objectManager.grassField.size() == 0) { return; }
#if DEBUGGING_PIPELINES
		printf("Drawing grass \n");
#endif
		PipelineManager::pipelines[Pipe_grass]->bind(frameInfo.cmdIndexPair.first);
		vkCmdBindDescriptorSets(
			frameInfo.cmdIndexPair.first,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			PipelineManager::pipeLayouts[PL_grass],
			0, 1,
			DescriptorHandler::getDescSet(DS_global, frameInfo.cmdIndexPair.second),
			0,
			nullptr
		);
		vkCmdBindDescriptorSets(
			frameInfo.cmdIndexPair.first,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			PipelineManager::pipeLayouts[PL_grass],
			1, 1,
			EWETexture::getDescriptorSets(objectManager.grassTextureID, frameInfo.cmdIndexPair.second),
			0,
			nullptr
		);

		UVScrollingPushData push{ glm::vec2{glm::mod(frameInfo.time / 6.f, 1.f), glm::mod(frameInfo.time / 9.f, 1.f)} };
		vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_grass], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UVScrollingPushData), &push);

		for (int i = 0; i < objectManager.grassField.size(); i++) {
			objectManager.grassField[i].model->BindAndDrawInstance(frameInfo.cmdIndexPair.first, 0);
		}
#if DEBUGGING_PIPELINES
		printf("after drawing grass \n");
#endif
	}

	void AdvancedRenderSystem::render2DGameObjects(FrameInfo2D& frameInfo) {

		//printf("beginning r2d \n");
		//if (frameInfo.menuActive || GameUI::GetActive()) {
		if (!frameInfo.menuActive) {
			
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
		if (frameInfo.menuActive) {
			if (menuManager.drawingNineUI()) {
				PipelineManager::pipelines[Pipe_NineUI]->bind(frameInfo.cmdIndexPair.first);
				menuManager.drawNineUI(frameInfo.cmdIndexPair.first, frameInfo.cmdIndexPair.second);
			}
			PipelineManager::pipelines[Pipe_2d]->bind(frameInfo.cmdIndexPair.first);
			pipe2dBinded = true;
			menuManager.drawMenuObjects(frameInfo.cmdIndexPair.first, frameInfo.cmdIndexPair.second);
		}


		//printf("binding textures from in game even if game isnt active \n");
		if (uiHandler->overlay) {
			if (!pipe2dBinded) {
				PipelineManager::pipelines[Pipe_2d]->bind(frameInfo.cmdIndexPair.first);
			}
			uiHandler->overlay->drawObjects(frameInfo.cmdIndexPair);
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


			const std::map<ShaderFlags, std::map<TextureID, std::vector<MaterialInfo>>>& matMapTemp = materialHandlerInstance->getMaterialMap(); //not clean and get because shit shouldnt be deleted runtime??? at least not currently
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

				PipelineManager::dynamicMaterialPipeline[flags]->bind(frameInfo.cmdIndexPair.first);

				uint8_t pipeLayoutIndex = ((flags & 16) >> 4) + ((flags & 8) >> 3) + ((flags & 4) >> 2) + ((flags & 2) >> 1) + (flags & 1) + (((flags & 128) >> 7) * 6);
				/*
				printf("dynamic material pipeLayoutIndex:hasBump : %d:%d \n", pipeLayoutIndex, flags & 16);
				if ((flags & 16)) {
					printf("has bump and - %d:%d:%d:%d:%d \n",(flags & 16) >> 4, (flags & 8) >> 3, (flags & 4) >> 2, (flags & 2) >> 1, (flags & 1));
				}
				*/
				//printf("flags:pipeIndex - %d:%d \n", flags, pipeLayoutIndex);
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex],
					0, 1,
					DescriptorHandler::getDescSet(DS_global, frameInfo.cmdIndexPair.second),
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
								frameInfo.cmdIndexPair.first,
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex],
								1, 1,
								EWETexture::getDescriptorSets(iterTexID->first, frameInfo.cmdIndexPair.second),
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

							vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::dynamicMaterialPipeLayout[pipeLayoutIndex], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
							iterTexID->second[i].meshPtr->bind(frameInfo.cmdIndexPair.first);
							//printf("assimpNT post-bind : %d \n", j);
							iterTexID->second[i].meshPtr->draw(frameInfo.cmdIndexPair.first);
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
			PipelineManager::pipelines[Pipe_textured]->bind(frameInfo.cmdIndexPair.first);
			//texturedPipeline->bind(frameInfo.cmdIndexPair.first);
			vkCmdBindDescriptorSets(
				frameInfo.cmdIndexPair.first,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_textured],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.cmdIndexPair.second),
				//&globalDescriptorSet[frameInfo.cmdIndexPair.second],
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
					//std::cout << "iterator value : " << iter->second.textureID * 2 + frameInfo.cmdIndexPair.second << std::endl;

					vkCmdBindDescriptorSets(
						frameInfo.cmdIndexPair.first,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						PipelineManager::pipeLayouts[PL_textured],
						1, 1,
						//&EWETexture::modeDescriptorSets[iter->second.textureID * 2 + frameInfo.cmdIndexPair.second],
						EWETexture::getDescriptorSets(iter->second.textureID, frameInfo.cmdIndexPair.second),
						0, nullptr
					);
					currentBindedTextureID = iter->second.textureID;
				}
				//printf("drawing now : %d \n", i);
				SimplePushConstantData push{};
				push.modelMatrix = iter->second.transform.mat4();
				push.normalMatrix = iter->second.transform.normalMatrix();
				//std::cout << "pre-bind/draw : " << i;
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_textured], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
				iter->second.model->bind(frameInfo.cmdIndexPair.first);
				iter->second.model->draw(frameInfo.cmdIndexPair.first);
				//std::cout << " ~ post-bind/draw : " << i << std::endl;
			}
		}
		//printf("before material objects \n");
		if (objectManager.materialBuildObjects.size() > 0) {
			printf("beginning material build object draw \n");
			PipelineManager::pipelines[Pipe_fbx]->bind(frameInfo.cmdIndexPair.first);
			vkCmdBindDescriptorSets(
				frameInfo.cmdIndexPair.first,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_fbx],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.cmdIndexPair.second),
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
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_fbx],
					1, 1,
					EWETexture::getDescriptorSets(iter->second.textureID, frameInfo.cmdIndexPair.second),
					0, nullptr
				);
				//std::cout << "predraw assimp" << std::endl;

				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_fbx], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
				iter->second.model->bind(frameInfo.cmdIndexPair.first);
				iter->second.model->draw(frameInfo.cmdIndexPair.first);
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
			PipelineManager::pipelines[Pipe_bobTrans]->bind(frameInfo.cmdIndexPair.first);
			vkCmdBindDescriptorSets(
				frameInfo.cmdIndexPair.first,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_fbx],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.cmdIndexPair.second),
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
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_fbx],
					1, 1,
					EWETexture::getDescriptorSets(iter->second.textureID, frameInfo.cmdIndexPair.second),
					0, nullptr
				);
				//std::cout << "predraw assimp" << std::endl;

				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_fbx], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
				iter->second.model->bind(frameInfo.cmdIndexPair.first);
				iter->second.model->draw(frameInfo.cmdIndexPair.first);
				//std::cout << "post draw assimp" << std::endl;

			}
		}
		*/
		//printf("before rendering sprites \n");
		if(objectManager.spriteBuildObjects.size() > 0) {

			PipelineManager::pipelines[Pipe_sprite]->bind(frameInfo.cmdIndexPair.first);
			vkCmdBindDescriptorSets(
				frameInfo.cmdIndexPair.first,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_sprite],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.cmdIndexPair.second),
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
					//std::cout << "iterator value : " << iter->second.textureID * 2 + frameInfo.cmdIndexPair.second << std::endl;

					vkCmdBindDescriptorSets(
						frameInfo.cmdIndexPair.first,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						PipelineManager::pipeLayouts[PL_sprite],
						1, 1,
						//&EWETexture::modeDescriptorSets[iter->second.textureID * 2 + frameInfo.cmdIndexPair.second],
						EWETexture::getSpriteDescriptorSets(iter->second.textureID, frameInfo.cmdIndexPair.second),
						0, nullptr
					);
					currentBindedTextureID = iter->second.textureID;
				}
				//printf("drawing now : %d \n", i);
				SimplePushConstantData push{};
				push.modelMatrix = iter->second.transform.mat4();
				push.normalMatrix = iter->second.transform.normalMatrix();
				//std::cout << "pre-bind/draw : " << i;
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_sprite], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
				iter->second.model->bind(frameInfo.cmdIndexPair.first);
				iter->second.model->draw(frameInfo.cmdIndexPair.first);
			}


			EWETexture::newSpriteFrame();
		}
		//printf("after rendering sprites \n");
		if (objectManager.lBuilderObjects.size() > 0) {
			PipelineManager::pipelines[Pipe_grid]->bind(frameInfo.cmdIndexPair.first);

			//printf("rendering lbuilder objects \n");
			//i wont have to bind this descriptor set if the playerobject branch is open, but I don't think it should happen right now, not worth checking
			vkCmdBindDescriptorSets(
				frameInfo.cmdIndexPair.first,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PipelineManager::pipeLayouts[PL_textured],
				0, 1,
				DescriptorHandler::getDescSet(DS_global, frameInfo.cmdIndexPair.second),
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
						frameInfo.cmdIndexPair.first,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						PipelineManager::pipeLayouts[PL_textured],
						1, 1,
						EWETexture::getDescriptorSets(iter->second.textureID, frameInfo.cmdIndexPair.second),
						0, nullptr
					);
					lastTextureID = iter->second.textureID;
				}

				SimplePushConstantData push{};
				push.modelMatrix = iter->second.transform.mat4();
				push.normalMatrix = iter->second.transform.normalMatrix();

				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_textured] , VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstantData), &push);
				iter->second.model->bind(frameInfo.cmdIndexPair.first);
				iter->second.model->draw(frameInfo.cmdIndexPair.first);
				//printf("rendering a builder object? \n");
			}

			//printf("end of rendering lbuilder objects \n");
		}
		//printf("after rendering the grid \n");
	}
#endif
	
	void AdvancedRenderSystem::renderLoadingScreen(FrameInfoLoading& frameInfo) {
		PipelineManager::pipelines[Pipe_loading]->bind(frameInfo.cmdIndexPair.first);

		//printf("before binding descriptor set 0 \n");
		vkCmdBindDescriptorSets(
			frameInfo.cmdIndexPair.first,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			PipelineManager::pipeLayouts[PL_loading],
			0, 1,
			DescriptorHandler::getDescSet(DS_global, frameInfo.cmdIndexPair.second),
			0,
			nullptr
		);

		//printf("after binding descriptor set 0 \n");
		vkCmdBindDescriptorSets(
			frameInfo.cmdIndexPair.first,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			PipelineManager::pipeLayouts[PL_loading],
			1, 1,
			EWETexture::getDescriptorSets(frameInfo.leafSystem->leafTextureID, frameInfo.cmdIndexPair.second),
			0,
			nullptr
		);
		//printf("after binding descriptor set 1 \n");
		vkCmdBindDescriptorSets(
			frameInfo.cmdIndexPair.first,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			PipelineManager::pipeLayouts[PL_loading],
			2, 1,
			DescriptorHandler::getDescSet(DS_loading, frameInfo.cmdIndexPair.second),
			0, nullptr
		);
		//printf("after binding descriptor set 2 \n");

		//frameInfo.leafSystem->leafModel->BindAndDrawInstance(frameInfo.cmdIndexPair.first, frameInfo.cmdIndexPair.second);
		frameInfo.leafSystem->draw(frameInfo.cmdIndexPair.first);
		//printf("after leaf bind and draw \n");
	}

	//void AdvancedRenderSystem::initDescriptorSets()
	void AdvancedRenderSystem::initGlobalPool(unsigned int maxSets) {
		globalPool = EWEDescriptorPool::Builder(eweDevice)
			.setMaxSets(maxSets)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.build();

	}
	/* this is old af, one of the first things i did in this engine. id like to revisit it at some point
	inline void AdvancedRenderSystem::renderRocks(FrameInfo& frameInfo) {
		//simple gameObjects and playerObjects use the same pipeline
		//std::cout << "field size : " << frameInfo.rockField.size() << std::endl;
		//unsigned int rockCount = 0;
		
		for (int i = 0; i < frameInfo.rockField.size(); i++) {

			//std::cout << "currentPosition size : " << frameInfo.rockField[i].currentPosition.size() << std::endl;
			for (int j = 0; j < frameInfo.rockField[i].currentPosition.size(); j++) {
				//std::cout << "predraw simple" << std::endl;
				SimplePushConstantData push{ glm::mat4{1.f}, glm::mat3{1.f} };
				glm::vec3 tempPosition = frameInfo.rockField[i].trackPositions[frameInfo.rockField[i].currentPosition[j]];
				push.modelMatrix[3].x = tempPosition.x;
				push.modelMatrix[3].y = tempPosition.y;
				push.modelMatrix[3].z = tempPosition.z;

				vkCmdPushConstants(frameInfo.cmdIndexPair.first, simpleLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
				frameInfo.rockModel.bind(frameInfo.cmdIndexPair.first);
				frameInfo.rockModel.draw(frameInfo.cmdIndexPair.first);
				//ockCount++;
				//std::cout << "post draw simple" << std::endl;
			}
		}
		
		//std::cout << "rock count : " << rockCount << std::endl;
	}
	*/
	
	void AdvancedRenderSystem::updateMaterialPipelines(VkPipelineRenderingCreateInfo const& pipeRenderInfo) {
		//call this function every time a new material is added to the scene
		

		/*
		* its ABOUT 3 ms to init and finalize glslang, might need to check if i even need it, that would save about 2ms if its not needed, but usually when i glslang this it'll be needed
		auto startingTime = std::chrono::high_resolution_clock::now();
		printf("benchmarking cost of GLSLang \n");
		for (int i = 0; i < 1000; i++) {
			glslang::InitializeProcess();
			glslang::FinalizeProcess();
		}
		float timeTaken = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startingTime).count();
		printf("benchmarking cost of GLSLang : %.5f \n", timeTaken / 1000);
		*/

		const std::map<ShaderFlags, std::map<TextureID, std::vector<MaterialInfo>>>& matMapTemp = materialHandlerInstance->cleanAndGetMaterialMap();
		if (matMapTemp.size() > 0) {
			bool glslNeeded = false;

			for (auto iter = matMapTemp.begin(); iter != matMapTemp.end(); iter++) {
				if (PipelineManager::dynamicMaterialPipeline.find(iter->first) == PipelineManager::dynamicMaterialPipeline.end()) {
					if (!glslNeeded) {
						glslNeeded = true;
						glslang::InitializeProcess();
					}
					PipelineManager::updateMaterialPipe(iter->first, pipeRenderInfo, eweDevice);
				}
			}
			if (glslNeeded) {
				glslang::FinalizeProcess();
			}
		}
		/*
		for (auto iter = materialHandlerInstance->playerMap1.begin(); iter != materialHandlerInstance->playerMap1.end(); iter++) {
			PipelineManager::updateMaterialPipe(iter->first, renderPass, eweDevice);
		}
		*/

		//printf("material map sizes, skele, assimp, ARS - %d:%d:%d \n", AssimpModel::materialMeshes.size(), PlayerSkeleton::materialMeshes.size(), materialMeshes.size());
		//printf("material pipelines were updated, material pipeline size : %d \n", PipelineManager::dynamicMaterialPipeline.size());
	}
}