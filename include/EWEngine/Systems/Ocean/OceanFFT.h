#pragma once
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Descriptors.h"
#include "EWEngine/Graphics/Device_Buffer.h"
#include "EWEngine/Graphics/Pipeline.h"
#include "EWEngine/Compute/Compute_Texture.h"

#include "OceanStructs.h"

#include <cmath>
#include <array>

namespace EWE {
	namespace Ocean {
		class OceanFFT {
			const uint32_t compute_local_work_groups_size = 8;
		public:

			struct FFT_Buffers {
				glm::vec2 buffers{ 0.f };
			};
			struct FFT_Parameters {
				uint32_t step{ 256 };
				uint32_t size{ 1024 };
			} fft_parameters;

			struct FFT_Data {
				/*
				layout(binding = 0, rgba8) uniform readonly image2D PrecomputeBuffer;
				layout(binding = 1, rgba8) uniform readonly image2D PrecomputedData;
				layout(binding = 2) uniform  Parameters {
					uint step;
					uint size;
					int outIter;
					int inIter;
				} parameters;

				struct Process_Data {
					vec2 data[2];
				};

				layout(std430, set = 0, binding = 2) buffer Please {
					Process_Data process_data[];
				} please;
				*/
			};
			
			uint32_t work_group_size;

			std::array<EWEDescriptorSetLayout*, 4> fftDSLs = {nullptr, nullptr, nullptr, nullptr};
			std::unique_ptr<EWEBuffer> parameterBuffer;
			//VkDescriptorSet parameterDescriptors;

			std::map<Pipe_Enum, EWE_Compute_Pipeline> fft_pipelines;

			uint32_t log_size;

			Compute_Texture precomputedData;
			VkDescriptorSet twiddleDescriptorSet;

			//need to run precomputeTwiddle here
			OceanFFT(uint16_t ocean_resolution, EWEDevice& device);
			~OceanFFT();
			void createDSLs(EWEDevice& device, std::shared_ptr<EWEDescriptorPool> oceanPool);

			//NEVER USED? WTF
			//void FFT2D(VkCommandBuffer cmdBuf, bool outputToInput = false);

			void inverse_FFT(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout, std::array<VkBuffer, 5>& derivativeBuffer, std::array<VkDescriptorSet, 8>& deriv_descriptors);

			void precompute(VkCommandBuffer cmdBuf);
			//void inverse_FFT(VkCommandBuffer cmdBuf, uint8_t inputIter);
			//void permute_IFFTS(VkCommandBuffer cmdBuf);

			//void inverse_FFT2d(VkCommandBuffer cmdBuf, TextureID inputID, TextureID outputID, bool outputToInput = false, bool scale = true, bool permute = false);

			//void PrecomputeTwiddleFactorsAndInputIndices(VkCommandBuffer cmdBuf);
		};
	}
}