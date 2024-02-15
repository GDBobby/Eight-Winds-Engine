#include "EWEngine/Systems/Ocean/WaveCascades.h"

namespace EWE {
	namespace Ocean {
		WaveCascades::WaveCascades(EWEDevice& device, OceanFFT& oceanFFT, uint16_t ocean_resolution, VkDescriptorBufferInfo* gaussianNoise, std::shared_ptr<EWEDescriptorPool> oceanPool, std::array<EWEDescriptorSetLayout*, 6>& cascadeDSLs, VkDescriptorBufferInfo* spectrumBufferInfo, VkDescriptorBufferInfo* timeBufferInfo) :
			device{ device },
			oceanFFT{ oceanFFT },
			work_group_size{ ocean_resolution / compute_local_work_groups_size },
			H0_Texture{device, ocean_resolution, ocean_resolution},
			WaveData_Texture{ device, ocean_resolution, ocean_resolution },
			displacement_Texture{ device, ocean_resolution, ocean_resolution },//, true },
			derivative_Texture{ device, ocean_resolution, ocean_resolution },
			turbulence_Texture{ device, ocean_resolution, ocean_resolution }
		{
			separateGraphicsComputeQueue = device.getComputeGraphicsIndex().size() > 1;
			cascade_params.size = ocean_resolution;
			//fft descriptor

			constructBuffers(ocean_resolution);
			constructDescriptors(oceanPool, cascadeDSLs, spectrumBufferInfo, gaussianNoise, timeBufferInfo);

			//buildPipeBarriers();
		}
		void WaveCascades::constructBuffers(uint16_t ocean_resolution) {

			size_t derivSize = sizeof(float) * 2 * ocean_resolution * ocean_resolution;
			for (uint8_t i = 0; i < 5; i++) {
				derivative_buffer[i] = std::make_unique<EWEBuffer>(
					device, derivSize, 1,
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
					);
				derivative_buffer[i]->map();
				derivativeVkBuffers[i] = derivative_buffer[i]->getBuffer();
			}

			cascade_parameter_buffer = std::make_unique<EWEBuffer>(
				device, sizeof(Cascade_Parameters), 1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			cascade_parameter_buffer->map();

		}
		void WaveCascades::constructDescriptors(std::shared_ptr<EWEDescriptorPool> oceanPool, std::array<EWEDescriptorSetLayout*, 6>& cascadeDSLs, VkDescriptorBufferInfo* spectrumBufferInfo, VkDescriptorBufferInfo* gaussianNoise, VkDescriptorBufferInfo* timeBufferInfo) {

			//permute descriptor
			permute_descriptor = EWEDescriptorWriter(*oceanFFT.fftDSLs[0], *oceanPool)
				.writeBuffer(0, oceanFFT.parameterBuffer->descriptorInfo())
				.writeBuffer(1, derivative_buffer[0]->descriptorInfo())
				.writeBuffer(2, derivative_buffer[1]->descriptorInfo())
				.writeBuffer(3, derivative_buffer[2]->descriptorInfo())
				.writeBuffer(4, derivative_buffer[3]->descriptorInfo())
				//.writeBuffer(1, &buffers[i][2]->descriptorInfo())
				.build();

			//inverse fft descriptor
			std::cout << "INVERSE FFT DESCRIPTOR" << std::endl;
			
			inverse_fft_descriptor = EWEDescriptorWriter(*oceanFFT.fftDSLs[1], *oceanPool)
				.writeImage(0, oceanFFT.precomputedData.getDescriptor())
				.writeBuffer(1, oceanFFT.parameterBuffer->descriptorInfo())
				//.writeBuffer(1, &buffers[i][2]->descriptorInfo())
				.build();

			for (uint8_t i = 0; i < 4; i++) {
				inverse_fft_deriv_descriptor[i] = EWEDescriptorWriter(*oceanFFT.fftDSLs[2], *oceanPool)
					.writeBuffer(0, derivative_buffer[i]->descriptorInfo())
					.writeBuffer(1, derivative_buffer[4]->descriptorInfo())
					//.writeBuffer(1, &buffers[i][2]->descriptorInfo())
					.build();
				
				inverse_fft_deriv_descriptor[i + 4] = EWEDescriptorWriter(*oceanFFT.fftDSLs[2], *oceanPool)
					.writeBuffer(0, derivative_buffer[4]->descriptorInfo())
					.writeBuffer(1, derivative_buffer[i]->descriptorInfo())
					//.writeBuffer(1, &buffers[i][2]->descriptorInfo())
					.build();
				
			}
			
			//initial spectrum descriptor

			std::cout << "INITIAL SPECTRUM DESCRIPTOR" << std::endl;
			initial_spectrum_descriptor = EWEDescriptorWriter(*cascadeDSLs[0], *oceanPool)
				.writeImage(0, WaveData_Texture.getDescriptor())
				.writeBuffer(1, gaussianNoise)
				.writeBuffer(2, cascade_parameter_buffer->descriptorInfo())
				.writeBuffer(3, spectrumBufferInfo)
				.writeBuffer(4, derivative_buffer[0]->descriptorInfo())
				.build();
			//time dependent

			std::cout << "TIME DEPENDENT DESCRIPTOR" << std::endl;
			time_dependent_descriptor = EWEDescriptorWriter(*cascadeDSLs[1], *oceanPool)
				.writeImage(0, H0_Texture.getDescriptor())
				.writeImage(1, WaveData_Texture.getDescriptor())
				.writeBuffer(2, derivative_buffer[0]->descriptorInfo())
				.writeBuffer(3, derivative_buffer[1]->descriptorInfo())
				.writeBuffer(4, derivative_buffer[2]->descriptorInfo())
				.writeBuffer(5, derivative_buffer[3]->descriptorInfo())
				.writeBuffer(6, timeBufferInfo)
				.build();
			
			//wave texture merger

			std::cout << "TEXTURE MERGE DESCRIPTOR" << std::endl;
			
			texture_merger_descriptor = EWEDescriptorWriter(*cascadeDSLs[2], *oceanPool)
				.writeBuffer(0, derivative_buffer[0]->descriptorInfo())
				.writeBuffer(1, derivative_buffer[1]->descriptorInfo())
				.writeBuffer(2, derivative_buffer[2]->descriptorInfo())
				.writeBuffer(3, derivative_buffer[3]->descriptorInfo())
				.writeBuffer(4, timeBufferInfo)
				.build();
			//combined texture binding
			merged_texture_descriptor = EWEDescriptorWriter(*cascadeDSLs[4], *oceanPool)
				.writeImage(0, displacement_Texture.getDescriptor())
				.writeImage(1, derivative_Texture.getDescriptor())
				.writeImage(2, turbulence_Texture.getDescriptor())
				.build();

			//conjugated spectrum
			std::cout << "CONJUGATED SPECTRUM DESCRIPTOR" << std::endl;
			
			conjugated_spectrum_descriptor = EWEDescriptorWriter(*cascadeDSLs[3], *oceanPool)
				.writeImage(0, H0_Texture.getDescriptor())
				.writeImage(1, WaveData_Texture.getDescriptor())
				.writeBuffer(2, cascade_parameter_buffer->descriptorInfo())
				.writeBuffer(3, derivative_buffer[4]->descriptorInfo())
				.build();
			std::cout << "after descriptors " << std::endl;
		}

		void WaveCascades::calculate_Horizontal_IFFTS(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout) {
			//bind the descriptor here

			//I NEED A DESCRIPTOR SET FOR EACH PARAMETER PAIR
			vkCmdBindDescriptorSets(
				cmdBuf,
				VK_PIPELINE_BIND_POINT_COMPUTE,
				pipeLayout,
				0, 1,
				&inverse_fft_descriptor,
				0,
				nullptr
			);

			oceanFFT.inverse_FFT(cmdBuf, pipeLayout, derivativeVkBuffers, inverse_fft_deriv_descriptor);
		}
		void WaveCascades::calculate_Vertical_IFFTS(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout) {
			//bind the descriptor here
			vkCmdBindDescriptorSets(
				cmdBuf,
				VK_PIPELINE_BIND_POINT_COMPUTE,
				pipeLayout,
				0, 1,
				&inverse_fft_descriptor,
				0,
				nullptr
			);

			oceanFFT.inverse_FFT(cmdBuf, pipeLayout, derivativeVkBuffers, inverse_fft_deriv_descriptor);
		}
		void WaveCascades::calculate_permutes(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout) {
			//bind permute descriptor here
			vkCmdBindDescriptorSets(
				cmdBuf,
				VK_PIPELINE_BIND_POINT_COMPUTE,
				pipeLayout,
				0, 1,
				&permute_descriptor,
				0,
				nullptr
			);
			vkCmdDispatch(cmdBuf, work_group_size, work_group_size, 1);
			//oceanFFT.permute_IFFTS(cmdBuf);

		}

		void WaveCascades::calculateInitial(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout, float lengthScale, float cutoffLow, float cutoffHigh) {
			cascade_params.cutoffHigh = cutoffHigh;
			cascade_params.cutoffLow = cutoffLow;
			cascade_params.lengthScale = lengthScale;

			cascade_parameter_buffer->writeToBuffer(&cascade_params);

			std::cout << "before initial desc \n";
			vkCmdBindDescriptorSets(
				cmdBuf,
				VK_PIPELINE_BIND_POINT_COMPUTE,
				pipeLayout, 
				0, 1,
				&initial_spectrum_descriptor,
				0,
				nullptr
			);
			vkCmdDispatch(cmdBuf, work_group_size, work_group_size, 1);
			std::cout << "after initial dispatch \n";
		}
		void WaveCascades::calculateInitialConjugate(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout) {
			std::cout << "before conjugated desc \n";
			vkCmdBindDescriptorSets(
				cmdBuf,
				VK_PIPELINE_BIND_POINT_COMPUTE,
				pipeLayout,
				0, 1,
				&conjugated_spectrum_descriptor,
				0,
				nullptr
			);
			std::cout << "after initial desc \n";
			vkCmdDispatch(cmdBuf, work_group_size, work_group_size, 1);
		}
		void WaveCascades::calculateTimeDependentSpectrum(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout) {
			vkCmdBindDescriptorSets(
				cmdBuf,
				VK_PIPELINE_BIND_POINT_COMPUTE,
				pipeLayout,
				0, 1,
				&time_dependent_descriptor,
				0,
				nullptr
			);

			//std::cout << "work group size : " << work_group_size << std::endl;
			vkCmdDispatch(cmdBuf, work_group_size, work_group_size, 1);
		}
		
		void WaveCascades::bindMergedTextures(VkCommandBuffer commandBuffer, VkPipelineLayout pipeLayout, uint8_t bindIndex) {
			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipeLayout,
				bindIndex, 1,
				&merged_texture_descriptor,
				//&renderTextureDescriptorSets[i][frameInfo.index],
				0, nullptr
			);
		}

		void WaveCascades::mergeTextures(VkCommandBuffer cmdBuf, VkPipelineLayout pipeLayout) {
			vkCmdBindDescriptorSets(
				cmdBuf,
				VK_PIPELINE_BIND_POINT_COMPUTE,
				pipeLayout,
				0, 1,
				&texture_merger_descriptor,
				0,
				nullptr
			);
			vkCmdBindDescriptorSets(
				cmdBuf, 
				VK_PIPELINE_BIND_POINT_COMPUTE,
				pipeLayout,
				1, 1,
				&merged_texture_descriptor,
				0,
				nullptr
			);

			vkCmdDispatch(cmdBuf, work_group_size, work_group_size, 1);

		}
		void WaveCascades::addVertTransferImages(std::vector<VkImage>& imageVector) {

			imageVector.push_back(displacement_Texture.getImage());
		}
		void WaveCascades::addFragTransferImages(std::vector<VkImage>& imageVector) {

			imageVector.push_back(derivative_Texture.getImage());
			imageVector.push_back(turbulence_Texture.getImage());
		}
		/*
		void WaveCascades::transferTexturesComputeToGraphics(VkCommandBuffer cmdBuf) {
			//maybe try vkcmdcopyimage from a compute specific image to a graphics specific image

			std::cout << " COMPUTE TO GRAPHICS \n";
			vkCmdPipelineBarrier(
				cmdBuf,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // Source pipeline stage
				VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,        // Destination pipeline stage
				0,
				0, nullptr,
				0, nullptr,
				1, &CTGimageBarrier[0]
			);
			vkCmdPipelineBarrier(
				cmdBuf,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // Source pipeline stage
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,        // Destination pipeline stage
				0,
				0, nullptr,
				0, nullptr,
				2, &CTGimageBarrier[1]
			);
		}
		void WaveCascades::transferTexturesGraphicsToCompute(VkCommandBuffer cmdBuf) {

			std::cout << "GRAPHICS TO COMPUTE \n";
			vkCmdPipelineBarrier(
				cmdBuf,
				VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,    // src pipeline stage
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // dst pipeline stage
				0,
				0, nullptr,
				0, nullptr,
				1, &GTCimageBarrier[0]
			);
			vkCmdPipelineBarrier(
				cmdBuf,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,  // src pipeline stage
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // dst pipeline stage
				0,
				0, nullptr,
				0, nullptr,
				2, &GTCimageBarrier[1]
			);
		}
		void WaveCascades::buildPipeBarriers() {
			CTGimageBarrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			CTGimageBarrier[0].pNext = nullptr;
			CTGimageBarrier[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL; // Layout used for compute shader writes
			CTGimageBarrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // Transfer source optimal layout
			CTGimageBarrier[0].image = displacement_Texture.getImage();
			if (CTGimageBarrier[0].image == VK_NULL_HANDLE) {
				throw std::exception("EMPTY???");
			}

			CTGimageBarrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // or VK_IMAGE_ASPECT_DEPTH_BIT for depth images
			CTGimageBarrier[0].subresourceRange.baseMipLevel = 0;
			CTGimageBarrier[0].subresourceRange.levelCount = 1;
			CTGimageBarrier[0].subresourceRange.baseArrayLayer = 0;
			CTGimageBarrier[0].subresourceRange.layerCount = 1;
			CTGimageBarrier[0].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for compute shader writes
			CTGimageBarrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for transfer read operation
			if (separateGraphicsComputeQueue) {
				CTGimageBarrier[0].srcQueueFamilyIndex = device.getComputeIndex();
				CTGimageBarrier[1].dstQueueFamilyIndex = device.getGraphicsIndex();
			}
			else {
				CTGimageBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				CTGimageBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			}

			CTGimageBarrier[1] = CTGimageBarrier[0];
			CTGimageBarrier[1].image = derivative_Texture.getImage();
			if (CTGimageBarrier[1].image == VK_NULL_HANDLE) {
				throw std::exception("EMPTY???");
			}

			CTGimageBarrier[2] = CTGimageBarrier[0];
			CTGimageBarrier[2].image = turbulence_Texture.getImage();
			if (CTGimageBarrier[2].image == VK_NULL_HANDLE) {
				throw std::exception("EMPTY???");
			}

			GTCimageBarrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			GTCimageBarrier[0].pNext = nullptr;
			GTCimageBarrier[0].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // Layout used for compute shader writes
			GTCimageBarrier[0].newLayout = VK_IMAGE_LAYOUT_GENERAL; // Transfer source optimal layout
			GTCimageBarrier[0].image = displacement_Texture.getImage();
			GTCimageBarrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // or VK_IMAGE_ASPECT_DEPTH_BIT for depth images
			GTCimageBarrier[0].subresourceRange.baseMipLevel = 0;
			GTCimageBarrier[0].subresourceRange.levelCount = 1;
			GTCimageBarrier[0].subresourceRange.baseArrayLayer = 0;
			GTCimageBarrier[0].subresourceRange.layerCount = 1;

			GTCimageBarrier[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for compute shader writes
			GTCimageBarrier[0].dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for transfer read operation
			if (separateGraphicsComputeQueue) {
				GTCimageBarrier[0].srcQueueFamilyIndex = device.getComputeIndex();
				GTCimageBarrier[1].dstQueueFamilyIndex = device.getGraphicsIndex();
			}
			else {
				GTCimageBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				GTCimageBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			}

			GTCimageBarrier[1] = GTCimageBarrier[0];
			GTCimageBarrier[1].image = derivative_Texture.getImage();

			GTCimageBarrier[2] = GTCimageBarrier[0];
			GTCimageBarrier[2].image = turbulence_Texture.getImage();

			for (uint8_t i = 0; i < 3; i++) {

				if (GTCimageBarrier[i].image == VK_NULL_HANDLE) {
					throw std::exception("EMPTY???");
				}
			}
		}
		*/
	}
}