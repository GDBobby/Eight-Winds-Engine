#include "EWEngine/systems/Graphics/OceanFFT.h"

namespace EWE {
	namespace Ocean {
		OceanFFT::OceanFFT(uint16_t ocean_resolution, EWEDevice& device) 
			: work_group_size{ ocean_resolution / compute_local_work_groups_size }, log_size{ static_cast<uint32_t>(std::log2(ocean_resolution)) }, precomputedData{ device, ocean_resolution, ocean_resolution, false }
		{
			fft_parameters.size = ocean_resolution;
			/*
						ocean_time_buffer = std::make_unique<EWEBuffer>(
				device, sizeof(Ocean_Time_Struct), 1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				);
			*/

			parameterBuffer = std::make_unique<EWEBuffer>(device, sizeof(FFT_Parameters), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			parameterBuffer->map();
			parameterBuffer->writeToBuffer(&fft_parameters);

			
		}
		void OceanFFT::createDSLs(EWEDevice& device, std::shared_ptr<EWEDescriptorPool> oceanPool) {
			
			std::cout << "PRECOMPUTE DESCRIPTOR" << std::endl;
			fftDSLs[0] = EWEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();
			fft_pipelines.emplace(Pipe_permute, EWE_Compute_Pipeline::createPipeline(device, { fftDSLs[0]->getDescriptorSetLayout() }, "permute.comp.spv"));

			std::cout << "after permute pipe " << std::endl;

			std::cout << "INVERSE FFT DESCRIPTOR" << std::endl;
			fftDSLs[1] = EWEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();


			//fft parameters, separated because they get updated separately from the derivative buffers, and precomputed data
			fftDSLs[2] = EWEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();

			fft_pipelines.emplace(Pipe_horizontal_inverse_fft, EWE_Compute_Pipeline::createPipeline(device, { fftDSLs[1]->getDescriptorSetLayout(), fftDSLs[2]->getDescriptorSetLayout() }, "horizontal_step_inverse_FFT.comp.spv"));
			fft_pipelines.emplace(Pipe_vertical_inverse_fft, EWE_Compute_Pipeline::createPipeline(device, { fftDSLs[1]->getDescriptorSetLayout(), fftDSLs[2]->getDescriptorSetLayout()}, "vertical_step_inverse_FFT.comp.spv"));
			std::cout << "after inverse pipes" << std::endl;

			{
				fftDSLs[3] = EWEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				 .build();
				fft_pipelines.emplace(Pipe_precompute_twiddle, EWE_Compute_Pipeline::createPipeline(device, { fftDSLs[3]->getDescriptorSetLayout() }, "twiddle_and_indices.comp.spv"));
			}

			if (!
				EWEDescriptorWriter(*fftDSLs[3].get(), *oceanPool)
				.writeImage(0, precomputedData.getDescriptor())
				.writeBuffer(1, parameterBuffer->descriptorInfo())
				.build(twiddleDescriptorSet)
				) {
				printf("twiddle desc failure \n");
				throw std::runtime_error("failed to create twiddle descriptor set");
			}
			
		}
		void OceanFFT::precompute(VkCommandBuffer cmdBuf) {
			fft_pipelines.at(Pipe_precompute_twiddle).bind(cmdBuf);
			vkCmdBindDescriptorSets(cmdBuf,
				VK_PIPELINE_BIND_POINT_COMPUTE,
				fft_pipelines[Pipe_precompute_twiddle].pipe_layout,
				0, 1,
				&twiddleDescriptorSet,
				0, nullptr
			);

			vkCmdDispatch(cmdBuf, log_size, fft_parameters.size / 2 / compute_local_work_groups_size, 1);
		}

		void OceanFFT::inverse_FFT(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout, std::array<VkBuffer, 5>& derivativeVkBuffers, std::array<VkDescriptorSet, 8>& deriv_descriptors) {

			VkBufferMemoryBarrier bufferBarrier = {};
			bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access type from previous operation (shader write)
			bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access type for the next operation (shader read)
			bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.offset = 0;
			bufferBarrier.size = VK_WHOLE_SIZE; // The entire size of the buffer

			// Insert the pipeline barrier into the command buffer

			uint8_t outIndex[8] = {
				4, 4, 4, 4,
				0, 1, 2, 3
			};

			//bind descriptor??
			for (uint8_t descIndex = 0; descIndex < 4; descIndex++) {
				bool pingPong = false;
				for (uint32_t i = 0; i < log_size; i++) {
					pingPong = !pingPong;
					//vkCmdBindDescriptorSet(fft

					//change this to derivIn derivOut
					vkCmdBindDescriptorSets(
						cmdBuf,
						VK_PIPELINE_BIND_POINT_COMPUTE,
						pipeLayout,
						1, 1,
						&deriv_descriptors[descIndex + (pingPong * 4)],
						0, nullptr
					);
					//std::cout << "work_group_size : " << work_group_size << std::endl;
					vkCmdDispatch(cmdBuf, work_group_size, work_group_size, 1);
					if ((descIndex < 3) && i < (log_size - 1)) {
						bufferBarrier.buffer = derivativeVkBuffers[outIndex[descIndex + (pingPong * 4)]]; // The buffer you want to synchronize
						vkCmdPipelineBarrier(
							cmdBuf,
							VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Source pipeline stage (compute shader)
							VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Destination pipeline stage (compute shader)
							0, // Dependency flags (0 means no special behavior)
							0, nullptr, // Memory barriers (none in this case)
							1, &bufferBarrier, // Buffer memory barriers (synchronize access to storageBufferB)
							0, nullptr // Image memory barriers (none in this case)
						);
					}
					
				}
			}
		}
		//void OceanFFT::permute_IFFTS(VkCommandBuffer cmdBuf) {
			//descriptor already binded, i dont think i need to update anything?
			//calling vkCmdDispatch without even coming in here

			//vkCmdDispatch()
			//compute_structs[Pipe_scale].dispatch(cmdBuf, work_group_size);
		//}
	}
}