#include "EWEngine/Systems/Ocean/Ocean.h"
#include "EWEngine/Data/TransformInclude.h"
#include "EWEngine/Graphics/Texture/ImageFunctions.h"

#include "EWEngine/Graphics/PipelineBarrier.h"

namespace EWE {
	namespace Ocean {
		Ocean::Ocean(VkDescriptorImageInfo* skyboxImage) {
			PrepareStorageImage();
			CreateBuffers();
			CreateDescriptor(skyboxImage);

			ifsGPUData.pushData.mDepth = depth;
			tdfsGPUData.pushData.CopyFromIFS(ifsGPUData.pushData);
		}
		Ocean::~Ocean() {
			Sampler::RemoveSampler(oceanOutputImageInfoDescriptorCompute.sampler);
			EWE_VK(vkDestroyImageView, VK::Object->vkDevice, oceanOutputImageInfoDescriptorCompute.imageView, nullptr);
			//vkDestroySampler(EWEDevice::GetVkDevice(), oceanOutputImageInfoDescriptorCompute.sampler, nullptr); //this is a copy
			//vkDestroyImageView(EWEDevice::GetVkDevice(), oceanOutputImageInfoDescriptorCompute.imageView, nullptr); //this is a copy
			EWE_VK(vkDestroyImage, VK::Object->vkDevice, oceanOutputImages, nullptr);

			Sampler::RemoveSampler(oceanFreqImageInfoDescriptor.sampler);
			EWE_VK(vkDestroyImage, VK::Object->vkDevice, oceanFreqImages, nullptr);
			EWE_VK(vkDestroyImageView, VK::Object->vkDevice, oceanFreqImageInfoDescriptor.imageView, nullptr);
		}


		void Ocean::CreateDescriptor(VkDescriptorImageInfo* skyboxImage) {
			tdfsGPUData.pushData.mLengthScale = ifsGPUData.pushData.mLengthScale;
			graphicsGPUData.oceanRenderParameters.oceanFragmentData.mLengthScales = ifsGPUData.pushData.mLengthScale;
			graphicsGPUData.oceanRenderParameters.oceanFragmentData.mWindSpeed = ifsGPUData.jonswapParams.mWindSpeed;

			ifsGPUData.CreateDescriptorSet(&oceanFreqImageInfoDescriptor);
			tdfsGPUData.CreateDescriptorSet(&oceanFreqImageInfoDescriptor, &oceanOutputImageInfoDescriptorCompute);
			fftGPUData.CreateDescriptorSet(&oceanOutputImageInfoDescriptorCompute);
			fftGPUData.SetVkImage(oceanOutputImages);
			graphicsGPUData.CreateDescriptorSet(&oceanOutputImageInfoDescriptorGraphics, skyboxImage);
			
		}
		void Ocean::TransferComputeToGraphics() {
			Barrier::TransferImageStage(VK::Object->GetFrameBuffer(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, oceanOutputImages);
		}

		void Ocean::TransferGraphicsToCompute() {
			Barrier::TransferImageStage(VK::Object->GetFrameBuffer(), VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, oceanOutputImages);
		}
		//void Ocean::ComputeBarrier(CommandBuffer cmdBuf) {
		//}

		void Ocean::CreateBuffers() {
		
			frequencyBuffer = Construct<EWEBuffer>({ cascade_count * OCEAN_WAVE_COUNT * OCEAN_WAVE_COUNT * sizeof(float) * 4, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT });
			
		}

		void Ocean::PrepareStorageImage() {
			const VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;

			VkFormatProperties formatProperties;
			// Get device properties for the requested texture format
			EWE_VK(vkGetPhysicalDeviceFormatProperties, VK::Object->physicalDevice, format, &formatProperties);
			// Check if requested image format supports image storage operations required for storing pixel from the compute shader
			assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);


			VkImageCreateInfo imageCreateInfo{};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.pNext = nullptr;

			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = format;
			imageCreateInfo.extent = { OCEAN_WAVE_COUNT, OCEAN_WAVE_COUNT, 1 };
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = cascade_count * 3;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			imageCreateInfo.flags = 0;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			
			uint32_t queueData[] = { static_cast<uint32_t>(VK::Object->queueIndex[Queue::graphics]), static_cast<uint32_t>(VK::Object->queueIndex[Queue::present])};
			const bool differentFamilies = (queueData[0] != queueData[1]);
			imageCreateInfo.sharingMode = (VkSharingMode)differentFamilies;
			imageCreateInfo.queueFamilyIndexCount = 1 + differentFamilies;
			imageCreateInfo.pQueueFamilyIndices = queueData;
			Image::CreateImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, oceanOutputImages, oceanOutputImageMemory);

			imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT;
			imageCreateInfo.arrayLayers = cascade_count;
			Image::CreateImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, oceanFreqImages, oceanFreqImageMemory);


			SyncHub* syncHub = SyncHub::GetSyncHubInstance();
			//directly to graphics because no data is being uploaded
			CommandBuffer& cmdBuf = syncHub->BeginSingleTimeCommand(Queue::graphics);

			VkImageMemoryBarrier imageBarriers[2];
			imageBarriers[0] = Barrier::TransitionImageLayout(oceanOutputImages,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
				1, cascade_count * 3
			);			
			imageBarriers[1] = Barrier::TransitionImageLayout(oceanFreqImages,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
				1, cascade_count
			);
			EWE_VK(vkCmdPipelineBarrier, cmdBuf,
            	VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, //i get the feeling this is suboptimal, but this is what sascha does and i haven't found an alternative
            	0,
            	0, nullptr,
            	0, nullptr,
            	2, imageBarriers
			);

			//directly to graphics because no data is being uploaded
			syncHub->EndSingleTimeCommandGraphics(cmdBuf);

			// Create sampler
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.maxAnisotropy = 1.0f;
			samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 1.0f;
			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			Sampler::GetSampler(samplerInfo);
			oceanOutputImageInfoDescriptorGraphics.sampler = oceanOutputImageInfoDescriptorCompute.sampler;

			// Create image view
			VkImageViewCreateInfo view{};
			view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view.pNext = nullptr;
			view.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			view.format = format;
			view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view.subresourceRange.baseMipLevel = 0;
			view.subresourceRange.levelCount = 1;
			view.subresourceRange.baseArrayLayer = 0;
			view.subresourceRange.layerCount = cascade_count * 3;
			view.image = oceanOutputImages;
			EWE_VK(vkCreateImageView, VK::Object->vkDevice, &view, nullptr, &oceanOutputImageInfoDescriptorCompute.imageView);
			oceanOutputImageInfoDescriptorGraphics.imageView = oceanOutputImageInfoDescriptorCompute.imageView;
			view.image = oceanFreqImages;
			view.subresourceRange.layerCount = cascade_count;
			EWE_VK(vkCreateImageView, VK::Object->vkDevice, &view, nullptr, &oceanFreqImageInfoDescriptor.imageView);


			// Initialize a descriptor for later use
			oceanOutputImageInfoDescriptorCompute.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			oceanOutputImageInfoDescriptorGraphics.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			oceanFreqImageInfoDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			//ifsGPUData.CreateDescriptorSet(&oceanImageInfoDescriptor);
		}

		void Ocean::InitializeSpectrum() {
			ifsGPUData.Compute();

		}
		void Ocean::UpdateSpectrum(float dt) {
			tdfsGPUData.Compute(dt);
		}

		void Ocean::ComputeFFT(float deltaTime) {
			fftGPUData.Compute(deltaTime);
		}

		void Ocean::RenderOcean() {
			graphicsGPUData.Render();
		}

		void Ocean::UpdateNoInit(float dt) {
			tdfsGPUData.Compute(dt);
			fftGPUData.Compute(dt);
		}
		void Ocean::ReinitUpdate(float dt) {

			ifsGPUData.Compute();
			Barrier::TransferImageStage(VK::Object->GetFrameBuffer(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, oceanFreqImages);
			tdfsGPUData.Compute(dt);
			Barrier::TransferImageStage(VK::Object->GetFrameBuffer(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, oceanOutputImages);
			fftGPUData.Compute(dt);

			TransferComputeToGraphics();
		}

	}//ocean namespace
}