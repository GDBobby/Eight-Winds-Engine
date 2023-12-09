/*
#pragma once

#include "lve_device.hpp"
#include "graphics/lve_pipeline.h"
#include "lve_descriptors.h"
#include "graphics/lve_buffer.h"
#include "lve_texture.h"

#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <random>

#define ENABLE_VALIDATION false
#define DEFAULT_FENCE_TIMEOUT 100000000000

/
*TO USE CLOTH
* CALL PREPARE ONCE in the main menu, i dont really know hwy it needs a command buffer maybe remove that later
* if(cloth.prepared), maybe make this a get function, isPrepared()
* updatecomputeubo, buildcomputecommandbuffer BEFORE BEGINNING SWAP CHAIN RENDER PASS
* after beginning swapchain render pass, calll buildCommandBuffers()

EXAMPLE

cloth.updateGraphicsUBO(ubo.projection, ubo.view);
if (cloth.prepared) {
	//std::cout << "compute?" << std::endl;
	cloth.updateComputeUBO(logicTime);
	cloth.buildComputeCommandBuffer(commandBuffer);
}
lveRenderer.beginSwapChainRenderPass(commandBuffer);
cloth.buildCommandBuffers(commandBuffer);

/

namespace lve {
	class Cloth {
	private:
		LveDevice& lveDevice;

		VkShaderModule computeShaderModule;
		//VkShaderModule clothFragModule;
		//VkShaderModule clothVertModule;
		VkPipelineCache pipelineCache;
		float timer = 0.0f;

		bool paused = false;;
		//float frameTime;

		VkSemaphore semaphorePresentComplete;
		VkSemaphore semaphoreRenderComplete;
		LveDescriptorPool* globalPool;

		uint32_t clothTextureID;

	public:
		Cloth(LveDevice& device, LveDescriptorPool* pool) : lveDevice{ device }, globalPool{pool} {
			//clothTexture = std::make_unique<LveTexture>(LveTexture::createTexture(device, "sumo2.png"));
			compute.commandBuffers.resize(2);
			//prepare(globalPool, commandBuffer, dt);
		}
		//std::vector<VkCommandBuffer> cmdBuffers;
		uint32_t sceneSetup = 0;
		uint32_t readSet = 0;
		uint32_t indexCount;
		bool simulateWind = false;
		bool specializedComputeQueue = false;
		bool prepared = false;

		// Resources for the graphics part of the example
		struct {
			std::unique_ptr<LveDescriptorSetLayout> descriptorSetLayout;
			VkDescriptorSet descriptorSet;
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<LvePipeline> clothPipeline;
			std::unique_ptr<LveBuffer> indexBuffer;
			std::unique_ptr<LveBuffer> uniformBuffer;
			struct graphicsUBO {
				glm::mat4 projection;
				glm::mat4 view;
				glm::vec4 lightPos = glm::vec4(-2.0f, 4.0f, -2.0f, 1.0f);
			} ubo;
		} graphics;

		// Resources for the compute part of the example
		struct {
			struct StorageBuffers {
				std::unique_ptr<LveBuffer> inputBuffer;
				std::unique_ptr<LveBuffer> outputBuffer;
			} storageBuffers;
			struct Semaphores {
				VkSemaphore ready{ 0L };
				VkSemaphore complete{ 0L };
			} semaphores;
			std::unique_ptr<LveBuffer> uniformBuffer;
			VkQueue queue;
			VkCommandPool commandPool;
			std::vector<VkCommandBuffer> commandBuffers;
			std::unique_ptr<LveDescriptorSetLayout> descriptorSetLayout;
			std::vector<VkDescriptorSet> descriptorSets;
			VkPipelineLayout pipelineLayout;
			VkPipeline pipeline;
			struct computeUBO {
				float deltaT = 0.0f;
				float particleMass = 0.1f;
				float springStiffness = 2000.0f;
				float damping = 0.25f;
				float restDistH;
				float restDistV;
				float restDistD;
				float sphereRadius = 1.0f;
				glm::vec4 spherePos = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
				glm::vec4 gravity = glm::vec4(0.0f, 9.8f, 0.0f, 0.0f);
				glm::ivec2 particleCount;
			} ubo;
		} compute;

		// SSBO cloth grid particle declaration
		struct Particle {
			glm::vec4 pos;
			glm::vec4 vel;
			glm::vec4 uv;
			glm::vec4 normal;
			float pinned;
			glm::vec3 _pad0;
		};

		struct ClothStruct {
			glm::uvec2 gridsize = glm::uvec2(40, 40);
			glm::vec2 size = glm::vec2(5.0f);
		} clothStruct;

		~Cloth() {
			// Graphics
			// 
			// unique pointer buffers wont need to be destroyed
			//graphics.indexBuffer->destroy();
			//graphics.uniformBuffer.destroy();
			//vkDestroyPipeline(lveDevice.device(), graphics.pipelines.cloth, nullptr);
			//vkDestroyPipeline(lveDevice.device(), graphics.pipelines.sphere, nullptr);
			vkDestroyPipelineLayout(lveDevice.device(), graphics.pipelineLayout, nullptr);
			//vkDestroyDescriptorSetLayout(lveDevice.device(), graphics.descriptorSetLayout, nullptr);
			//textureCloth.destroy();

			// Compute
			//compute.storageBuffers.input.destroy();
			//compute.storageBuffers.output.destroy();
			//compute.uniformBuffer.destroy();
			vkDestroyPipelineLayout(lveDevice.device(), compute.pipelineLayout, nullptr);
			//vkDestroyDescriptorSetLayout(lveDevice.device(), compute.descriptorSetLayout, nullptr);
			vkDestroyPipeline(lveDevice.device(), compute.pipeline, nullptr);
			vkDestroySemaphore(lveDevice.device(), compute.semaphores.ready, nullptr);
			vkDestroySemaphore(lveDevice.device(), compute.semaphores.complete, nullptr);
			vkDestroyCommandPool(lveDevice.device(), compute.commandPool, nullptr);
			vkDestroyShaderModule(lveDevice.device(), computeShaderModule, nullptr);
			//clothTexture->destroy();
		}


		void addGraphicsToComputeBarriers(VkCommandBuffer commandBuffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);
		void addComputeToComputeBarriers(VkCommandBuffer commandBuffer);
		void addComputeToGraphicsBarriers(VkCommandBuffer commandBuffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);

		void buildCommandBuffers(VkCommandBuffer drawCmdBuffers);

		void updateGraphicsUBO(glm::mat4 projection, glm::mat4 view) {
			graphics.ubo.projection = projection;
			graphics.ubo.view = view;
			memcpy(graphics.uniformBuffer->getMappedMemory(), &graphics.ubo, sizeof(graphics.ubo));
		}

		// todo: check barriers (validation, separate compute queue)
		void buildComputeCommandBuffer(VkCommandBuffer cmdBuffer);

		// Setup and fill the compute shader storage buffers containing the particles
		void prepareStorageBuffers();

		void setupLayoutsAndDescriptors();

		void preparePipelines(VkPipelineRenderingCreateInfo const& pipeRenderInfo);

		void prepareCompute();

		// Prepare and initialize uniform buffer containing shader uniforms
		void prepareUniformBuffers(float dt);

		void updateComputeUBO(float dt);

		void render(VkCommandBuffer cmdBuffer, float dt) {
			if (!prepared) { return; }
			draw(cmdBuffer);
			updateComputeUBO(dt);
		}
		void predraw();
		void draw(VkCommandBuffer drawCmdBuffer);

		void prepare(VkCommandBuffer cmdBuffer, float dt, VkPipelineRenderingCreateInfo const& pipeRenderInfo) {
			//VulkanExampleBase::prepare();
			// Make sure the code works properly both with different queues families for graphics and compute and the same queue family

	//#ifdef DEBUG_FORCE_SHARED_GRAPHICS_COMPUTE_QUEUE
	//			lveDevice.findPhysicalQueueFamilies().computeFamily = lveDevice.findPhysicalQueueFamilies().graphicsFamily;
	//#endif
	//#endif
			if (!prepared) {
				// Check whether the compute queue family is distinct from the graphics queue family
				specializedComputeQueue = lveDevice.getGraphicsIndex() != lveDevice.getComputeIndex();
				//std::cout << "queue is specialized!" << std::endl;
				//loadAssets();
				prepareStorageBuffers();
				prepareUniformBuffers(dt);
				//setupDescriptorPool();
				setupLayoutsAndDescriptors();
				preparePipelines(renderPass);
				prepareCompute();
				buildCommandBuffers(cmdBuffer);
				prepared = true;
			}
		}

		void render(float dt, VkCommandBuffer cmdBuffer) {
			if (!prepared) { return; }
			draw(cmdBuffer);

			updateComputeUBO(dt);
		}
	};
}
*/