#pragma once
#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/Pipeline.h"
#include "EWEngine/Systems/Ocean/OceanStructs.h"

//this is largely self-contained, and builds its own systems.
//	as i expand the engine, and these systems are needed in mroe places, 
//	ill remove them from this object and abstract them a little

namespace EWE {
	namespace Ocean {

#define CASCADE_COUNT 4
		class Ocean {

			const uint16_t ocean_resolution{ 1024 };
			const uint16_t cascade_count{ 4 };

			InitialFrequencySpectrumGPUData ifsGPUData{};
			
			VkDescriptorSet oceanTextures = VK_NULL_HANDLE;
			VkImage oceanImages = VK_NULL_HANDLE;
			VkDeviceMemory oceanImageMemory = VK_NULL_HANDLE;
			VkDescriptorImageInfo oceanImageDescriptor{};


		//RENDER BEGIN
			EWEPipeline* renderPipeline;
			VkPipelineLayout renderPipeLayout;
			std::array<EWEBuffer*, 2> renderParamsBuffer;


			std::unique_ptr<EWEModel> oceanModel;
		//RENDER END

			float time = 0.f;

		public:
			Ocean();
			~Ocean();

			//maybe construct the command buffer here?
			void ComputeUpdate(std::array<VkCommandBuffer, 5> oceanBuffers, float dt);

			void RenderUpdate(FrameInfo frameInfo);

			void createBuffers();
			void createRenderPipeline(VkPipelineRenderingCreateInfo const& pipeRenderInfo);

			void transferOceanToGraphics(VkCommandBuffer cmdBuffer) {
				/*
				std::vector<VkImage> transferVertImages{};
				transferVertImages.reserve(3);
				cascade[0].addVertTransferImages(transferVertImages);
				cascade[1].addVertTransferImages(transferVertImages);
				cascade[2].addVertTransferImages(transferVertImages);
				device.transferImageStage(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, transferVertImages);

				std::vector<VkImage> transferFragImages{};
				transferFragImages.reserve(6);
				cascade[0].addFragTransferImages(transferFragImages);
				cascade[1].addFragTransferImages(transferFragImages);
				cascade[2].addFragTransferImages(transferFragImages);
				device.transferImageStage(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, transferFragImages);
				*/
			}

			void transferGraphicsToOcean(VkCommandBuffer cmdBuf) {
				/*
				std::vector<VkImage> transferVertImages{};
				transferVertImages.reserve(3);
				cascade[0].addVertTransferImages(transferVertImages);
				cascade[1].addVertTransferImages(transferVertImages);
				cascade[2].addVertTransferImages(transferVertImages);
				device.transferImageStage(cmdBuf, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, transferVertImages);

				std::vector<VkImage> transferFragImages{};
				transferFragImages.reserve(6);
				cascade[0].addFragTransferImages(transferFragImages);
				cascade[1].addFragTransferImages(transferFragImages);
				cascade[2].addFragTransferImages(transferFragImages);
				device.transferImageStage(cmdBuf, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, transferFragImages);
				*/
			}

			void prepareStorageImage();
		};
	}
}