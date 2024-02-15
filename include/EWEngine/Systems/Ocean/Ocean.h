#pragma once
#include "OceanFFT.h"
#include "WaveCascades.h"
#include "EWEngine/Graphics/Model/Model.h"

//this is largely self-contained, and builds its own systems.
//	as i expand the engine, and these systems are needed in mroe places, 
//	ill remove them from this object and abstract them a little

namespace EWE {
	namespace Ocean {

#define CASCADE_COUNT 3
		class Ocean {

			const uint16_t ocean_resolution{ 1024 };
			const uint16_t noise_resolution{ 256 };
			EWEDevice& device;
			std::vector<WaveCascades> cascade;
			OceanFFT oceanFFT;
			Wave_Settings waveSettings;

			std::vector<std::array<float, 2>> gaussianNoise;

			float lengthScale[3] = { 250, 17, 5 };
			std::map<Pipe_Enum, EWE_Compute_Pipeline> ocean_compute_pipelines;
			std::array<EWEDescriptorSetLayout*, 6> cascadeDSLs = {
				nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
			};

			std::shared_ptr<EWEDescriptorPool> oceanPool;

			Ocean_Time_Struct ocean_time_struct;
			std::unique_ptr<EWEBuffer> ocean_time_buffer;

			std::unique_ptr<EWEBuffer> spectrum_parameter_buffer;
			Spectrum_Settings spectrumSettings[2];

			std::unique_ptr<EWEBuffer> gauss_buffer;


		//RENDER BEGIN
			std::unique_ptr<EWEPipeline> renderPipeline;
			VkPipelineLayout renderPipeLayout;
			EWEDescriptorSetLayout* renderParamsDSL{nullptr};
			std::array<std::unique_ptr<EWEBuffer>, 2> renderParamsBuffer;

			std::vector<VkDescriptorSet> renderParamsDescriptorSets;
			std::array<std::vector<VkDescriptorSet>, 3> renderTextureDescriptorSets;

			Ocean_Draw_Push_Constant ocean_push{};
			Ocean_Ubo ocean_ubo{};
			Ocean_Material ocean_material{};

			std::unique_ptr<EWEModel> oceanModel;

			TextureID foam = 0;
		//RENDER END

			float time = 0.f;

			void initializeDSLs();

			void getGaussNoise();
			float GaussianRandom(std::default_random_engine& randE, std::uniform_real_distribution<float>& distribution) {

				return std::cos(2 * 3.14159265358979323846f * distribution(randE)) * std::sqrt(-2.f * std::log(distribution(randE)));
			}

		public:
			Ocean(EWEDevice& device);
			~Ocean();

			void InitializeTwiddle(VkCommandBuffer cmdBuf);
			void InitialiseCascades(VkCommandBuffer cmdBuf);

			//maybe construct the command buffer here?
			void ComputeUpdate(std::array<VkCommandBuffer, 5> oceanBuffers, float dt);

			void RenderUpdate(FrameInfo frameInfo);

			void createBuffers();
			void createRenderPipeline(VkPipelineRenderingCreateInfo const& pipeRenderInfo);

			void transferOceanToGraphics(VkCommandBuffer cmdBuffer) {
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
			}

			void transferGraphicsToOcean(VkCommandBuffer cmdBuf) {
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
			}
		};
	}
}