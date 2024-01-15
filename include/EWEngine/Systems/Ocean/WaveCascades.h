#pragma once
#include "OceanStructs.h"
#include "OceanFFT.h"

#include "EWEngine/Graphics/Texture.h"
#include <array>

namespace EWE {
	namespace Ocean {
		class WaveCascades {
			const uint32_t compute_local_work_groups_size = 8;

			enum Cascade_Pipe_Enum {
				Pipe_initial_spectrum = 0,
				Pipe_time_dependent_spectrum,
				Pipe_textures_merger,
			};

			OceanFFT& oceanFFT;

			//std::array<std::vector<glm::vec2>, 5> differentials;
			uint32_t work_group_size;
			Cascade_Parameters cascade_params;

			std::unique_ptr<EWEBuffer> cascade_parameter_buffer;
			std::array<VkBuffer, 5> derivativeVkBuffers;
			std::array<std::unique_ptr<EWEBuffer>, 5> derivative_buffer;

			VkDescriptorSet inverse_fft_descriptor;
			std::array<VkDescriptorSet, 8> inverse_fft_deriv_descriptor;
			VkDescriptorSet permute_descriptor;
			VkDescriptorSet initial_spectrum_descriptor;
			VkDescriptorSet time_dependent_descriptor;
			VkDescriptorSet conjugated_spectrum_descriptor;

			VkDescriptorSet texture_merger_descriptor;
			VkDescriptorSet merged_texture_descriptor;
			//VkDescriptorSet merged_texture_descriptor;

			Compute_Texture H0_Texture;
			Compute_Texture WaveData_Texture;

			Compute_Texture displacement_Texture;
			Compute_Texture derivative_Texture;
			Compute_Texture turbulence_Texture;

			EWEDevice& device;

			bool separateGraphicsComputeQueue = false;

			std::array<VkImageMemoryBarrier, 3> CTGimageBarrier{};

			std::array<VkImageMemoryBarrier, 3> GTCimageBarrier{};

			void buildPipeBarriers();


		public:
			WaveCascades(EWEDevice& device, OceanFFT& oceanFFT, uint16_t ocean_resolution, VkDescriptorBufferInfo* gaussianNoise, std::shared_ptr<EWEDescriptorPool> oceanPool, std::array<std::unique_ptr<EWEDescriptorSetLayout>, 6>& cascadeDSLs, VkDescriptorBufferInfo* spectrumBufferInfo, VkDescriptorBufferInfo* timeBufferInfo);

			void constructBuffers(uint16_t ocean_resolution);

			void constructDescriptors(std::shared_ptr<EWEDescriptorPool> oceanPool, std::array<std::unique_ptr<EWEDescriptorSetLayout>, 6>& cascadeDSLs, VkDescriptorBufferInfo* spectrumBufferInfo, VkDescriptorBufferInfo* gaussianNoise, VkDescriptorBufferInfo* timeBufferInfo);

			void Dispose() {
				//paramsBuffer.release();
			}

			void calculateTimeDependentSpectrum(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout);

			//do fft stuff, 4 each
			//MERGE THESE PIPELINES INTO 1????????
			void calculate_Horizontal_IFFTS(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout);
			void calculate_Vertical_IFFTS(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout);
			void calculate_permutes(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout);

			void bindMergedTextures(VkCommandBuffer commandBuffer, VkPipelineLayout pipeLayout, uint8_t bindIndex);
			void mergeTextures(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout);
			//void transferTexturesComputeToGraphics(VkCommandBuffer cmdBuf);
			//void transferTexturesGraphicsToCompute(VkCommandBuffer cmdBuf);
			void addVertTransferImages(std::vector<VkImage>& imageVector);
			void addFragTransferImages(std::vector<VkImage>& imageVector);

			void calculateInitial(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout, float lengthScale, float cutoffLow, float cutoffHigh);
			void calculateInitialConjugate(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout);

			void calculateWavesAtTime(float time);

			std::array<VkDescriptorImageInfo*, 3> getImageInfo() {
				return { 
					displacement_Texture.getDescriptor(),
					derivative_Texture.getDescriptor(),
					turbulence_Texture.getDescriptor()
				};
			}
			std::array<VkBuffer, 5> getDerivativeVkBuffers() {
				return derivativeVkBuffers;
			}
			
		};
	}//namespace Ocean
} //namespace EWE