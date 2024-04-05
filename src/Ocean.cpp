#include "EWEngine/Systems/Ocean/Ocean.h"
#include "EWEngine/Data/TransformInclude.h"

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
			vkDestroySampler(EWEDevice::GetVkDevice(), oceanOutputImageInfoDescriptorCompute.sampler, nullptr);
			vkDestroyImageView(EWEDevice::GetVkDevice(), oceanOutputImageInfoDescriptorCompute.imageView, nullptr);
			//vkDestroySampler(EWEDevice::GetVkDevice(), oceanOutputImageInfoDescriptorCompute.sampler, nullptr); //this is a copy
			//vkDestroyImageView(EWEDevice::GetVkDevice(), oceanOutputImageInfoDescriptorCompute.imageView, nullptr); //this is a copy
			vkDestroyImage(EWEDevice::GetVkDevice(), oceanOutputImages, nullptr);

			vkDestroySampler(EWEDevice::GetVkDevice(), oceanFreqImageInfoDescriptor.sampler, nullptr);
			vkDestroyImage(EWEDevice::GetVkDevice(), oceanFreqImages, nullptr);
			vkDestroyImageView(EWEDevice::GetVkDevice(), oceanFreqImageInfoDescriptor.imageView, nullptr);
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
		void Ocean::TransferComputeToGraphics(VkCommandBuffer cmdBuf) {
			EWEDevice::GetEWEDevice()->TransferImageStage(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, oceanOutputImages);
		}

		void Ocean::TransferGraphicsToCompute(VkCommandBuffer cmdBuf) {
			EWEDevice::GetEWEDevice()->TransferImageStage(cmdBuf, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, oceanOutputImages);
		}
		//void Ocean::ComputeBarrier(VkCommandBuffer cmdBuf) {
		//}

		void Ocean::CreateBuffers() {
		
			frequencyBuffer = ConstructSingular<EWEBuffer>(ewe_call_trace, cascade_count * OCEAN_WAVE_COUNT * OCEAN_WAVE_COUNT * sizeof(float) * 4, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); //this allows the GPU to make all the decisions, and doesn't allow CPU access, which we don't need
			
		}

		void Ocean::PrepareStorageImage() {
			const VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;

			EWEDevice* eweDevice = EWEDevice::GetEWEDevice();

			VkFormatProperties formatProperties;
			// Get device properties for the requested texture format
			vkGetPhysicalDeviceFormatProperties(eweDevice->GetPhysicalDevice(), format, &formatProperties);
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
			// Image will be sampled in the graphics shaders and used as storage target in the compute shader
			imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			imageCreateInfo.flags = 0;
			// If compute and graphics queue family indices differ, we create an image that can be shared between them
			// This can result in worse performance than exclusive sharing mode, but save some synchronization to keep the sample simple

			uint32_t queueData[] = { eweDevice->GetGraphicsIndex(), eweDevice->GetPresentIndex() };
			const bool differentFamilies = (queueData[0] != queueData[1]);
			imageCreateInfo.sharingMode = (VkSharingMode)differentFamilies;
			imageCreateInfo.queueFamilyIndexCount = 1 + differentFamilies;
			imageCreateInfo.pQueueFamilyIndices = queueData;
			eweDevice->CreateImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, oceanOutputImages, oceanOutputImageMemory);

			imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT;
			imageCreateInfo.arrayLayers = cascade_count;
			eweDevice->CreateImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, oceanFreqImages, oceanFreqImageMemory);


			eweDevice->TransitionImageLayout(oceanOutputImages,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, //i get the feeling this is suboptimal, but this is what sascha does and i haven't found an alternative
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
				1, cascade_count * 3
			);			
			eweDevice->TransitionImageLayout(oceanFreqImages,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, //i get the feeling this is suboptimal, but this is what sascha does and i haven't found an alternative
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
				1, cascade_count
			);

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
			vkCreateSampler(eweDevice->Device(), &samplerInfo, nullptr, &oceanOutputImageInfoDescriptorCompute.sampler);
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
			EWE_VK_ASSERT(vkCreateImageView(eweDevice->Device(), &view, nullptr, &oceanOutputImageInfoDescriptorCompute.imageView));
			oceanOutputImageInfoDescriptorGraphics.imageView = oceanOutputImageInfoDescriptorCompute.imageView;
			view.image = oceanFreqImages;
			view.subresourceRange.layerCount = cascade_count;
			EWE_VK_ASSERT(vkCreateImageView(eweDevice->Device(), &view, nullptr, &oceanFreqImageInfoDescriptor.imageView));


			// Initialize a descriptor for later use
			oceanOutputImageInfoDescriptorCompute.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			oceanOutputImageInfoDescriptorGraphics.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			oceanFreqImageInfoDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			//ifsGPUData.CreateDescriptorSet(&oceanImageInfoDescriptor);
		}

		void Ocean::InitializeSpectrum(FrameInfo const& frameInfo) {
			ifsGPUData.Compute(frameInfo);

		}
		void Ocean::UpdateSpectrum(FrameInfo const& frameInfo, float dt) {
			tdfsGPUData.Compute(frameInfo, dt);
		}

		void Ocean::ComputeFFT(FrameInfo const& frameInfo, float deltaTime) {
			fftGPUData.Compute(frameInfo, deltaTime);
		}

		void Ocean::RenderOcean(FrameInfo const& frameInfo) {
			graphicsGPUData.Render(frameInfo);
		}

		void Ocean::UpdateNoInit(FrameInfo const& frameInfo, float dt) {
			tdfsGPUData.Compute(frameInfo, dt);
			fftGPUData.Compute(frameInfo, dt);
		}
		void Ocean::ReinitUpdate(FrameInfo const& frameInfo, float dt) {

			ifsGPUData.Compute(frameInfo);
			EWEDevice::GetEWEDevice()->TransferImageStage(frameInfo.cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, oceanFreqImages);
			tdfsGPUData.Compute(frameInfo, dt);
			EWEDevice::GetEWEDevice()->TransferImageStage(frameInfo.cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, oceanOutputImages);
			fftGPUData.Compute(frameInfo, dt);

			TransferComputeToGraphics(frameInfo.cmdBuf);
		}

	}//ocean namespace
}