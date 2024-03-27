#include "EWEngine/Systems/Ocean/Ocean.h"
#include "EWEngine/Data/TransformInclude.h"

namespace EWE {
	namespace Ocean {
		Ocean::Ocean() {
			prepareStorageImage();
		}
		Ocean::~Ocean() {
			vkDestroySampler(EWEDevice::GetVkDevice(), oceanImageDescriptor.sampler, nullptr);
			vkDestroyImage(EWEDevice::GetVkDevice(), oceanImages, nullptr);
			vkDestroyImageView(EWEDevice::GetVkDevice(), oceanImageDescriptor.imageView, nullptr);
		}

		void Ocean::createRenderPipeline(VkPipelineRenderingCreateInfo const& pipeRenderInfo) {
			
		}

		void Ocean::createBuffers() {

		}

		void Ocean::RenderUpdate(FrameInfo frameInfo) {

		}

		void Ocean::prepareStorageImage()
		{
			const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

			EWEDevice* eweDevice = EWEDevice::GetEWEDevice();

			VkFormatProperties formatProperties;
			// Get device properties for the requested texture format
			vkGetPhysicalDeviceFormatProperties(eweDevice->getPhysicalDevice(), format, &formatProperties);
			// Check if requested image format supports image storage operations required for storing pixel from the compute shader
			assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);


			VkImageCreateInfo imageCreateInfo{};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.pNext = nullptr;

			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = format;
			imageCreateInfo.extent = { OCEAN_WAVE_COUNT, OCEAN_WAVE_COUNT, 1 };
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = CASCADE_COUNT * 3;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			// Image will be sampled in the fragment shader and used as storage target in the compute shader
			imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			imageCreateInfo.flags = 0;
			// If compute and graphics queue family indices differ, we create an image that can be shared between them
			// This can result in worse performance than exclusive sharing mode, but save some synchronization to keep the sample simple

			uint32_t queueFamilyIndices[] = { eweDevice->getGraphicsIndex(), eweDevice->getPresentIndex() };
			const bool differentFamilies = (queueFamilyIndices[0] != queueFamilyIndices[1]);
			imageCreateInfo.sharingMode = (VkSharingMode)differentFamilies;
			imageCreateInfo.queueFamilyIndexCount = 1 + differentFamilies;
			imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
			eweDevice->createImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, oceanImages, oceanImageMemory);

			eweDevice->transitionImageLayout(oceanImages, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);

			// Create sampler
			VkSamplerCreateInfo sampler{};
			sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler.magFilter = VK_FILTER_LINEAR;
			sampler.minFilter = VK_FILTER_LINEAR;
			sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			sampler.addressModeV = sampler.addressModeU;
			sampler.addressModeW = sampler.addressModeU;
			sampler.mipLodBias = 0.0f;
			sampler.maxAnisotropy = 1.0f;
			sampler.compareOp = VK_COMPARE_OP_NEVER;
			sampler.minLod = 0.0f;
			sampler.maxLod = 1.0f;
			sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			EWE_VK_ASSERT(vkCreateSampler(eweDevice->device(), &sampler, nullptr, &oceanImageDescriptor.sampler));

			// Create image view
			VkImageViewCreateInfo view{};
			view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view.image = VK_NULL_HANDLE;
			view.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view.format = format;
			view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			view.image = oceanImages;
			EWE_VK_ASSERT(vkCreateImageView(eweDevice->device(), &view, nullptr, &oceanImageDescriptor.imageView));

			// Initialize a descriptor for later use
			oceanImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}

	}//ocean namespace
}