#include "EWEngine/Graphics/Texture/UI_Texture.h"
#include "EWEngine/Graphics/Texture/Sampler.h"

namespace EWE {
    namespace UI_Texture {

        //namespace internal {
        void CreateUIImage(ImageInfo& uiImageInfo, std::vector<PixelPeek> const& pixelPeek, Queue::Enum queue) {
            std::size_t layerSize = pixelPeek[0].width * pixelPeek[0].height * 4;
            uiImageInfo.arrayLayers = pixelPeek.size();
            const VkDeviceSize imageSize = layerSize * uiImageInfo.arrayLayers;
#if EWE_DEBUG
            assert(pixelPeek.size() > 1 && "creating an array without an array of images?");
            const VkDeviceSize assertionSize = pixelPeek[0].width * pixelPeek[0].height * 4;
            for (uint16_t i = 1; i < pixelPeek.size(); i++) {
                assert(assertionSize == (pixelPeek[i].width * pixelPeek[i].height * 4) && "size is not equal");
            }
#endif

            void* data;
#if USING_VMA
            StagingBuffer* stagingBuffer = Construct<StagingBuffer>({ imageSize, EWEDevice::GetAllocator() });
            vmaMapMemory(EWEDevice::GetAllocator(), stagingBuffer->vmaAlloc, &data);
#else
            StagingBuffer* stagingBuffer = Construct<StagingBuffer>({ imageSize, EWEDevice::GetEWEDevice()->GetPhysicalDevice(), EWEDevice::GetVkDevice() });
            EWE_VK(vkMapMemory, EWEDevice::GetVkDevice(), stagingBuffer->memory, 0, imageSize, 0, &data);
#endif
            uint64_t memAddress = reinterpret_cast<uint64_t>(data);
            uiImageInfo.mipLevels = 1;

            for (uint16_t i = 0; i < pixelPeek.size(); i++) {
                memcpy(reinterpret_cast<void*>(memAddress), pixelPeek[i].pixels, layerSize); //static_cast<void*> unnecessary>?
                stbi_image_free(pixelPeek[i].pixels);
                memAddress += layerSize;
            }
#if USING_VMA
            vmaUnmapMemory(EWEDevice::GetAllocator(), stagingBuffer->vmaAlloc);
#else
            EWE_VK(vkUnmapMemory, EWEDevice::GetVkDevice(), stagingBuffer->memory);
#endif

            VkImageCreateInfo imageCreateInfo;
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageCreateInfo.pNext = nullptr;
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.extent.width = pixelPeek[0].width;
            imageCreateInfo.extent.height = pixelPeek[0].height;
            imageCreateInfo.extent.depth = 1;
            imageCreateInfo.mipLevels = uiImageInfo.mipLevels;
            imageCreateInfo.arrayLayers = uiImageInfo.arrayLayers;

            imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.flags = 0;// VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

            Image::CreateImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uiImageInfo.image, uiImageInfo.memory);
#if DEBUG_NAMING
            DebugNaming::SetObjectName(EWEDevice::GetVkDevice(), uiImageInfo.image, VK_OBJECT_TYPE_IMAGE, pixelPeek[0].debugName.c_str());
#endif
#if IMAGE_DEBUGGING
            uiImageInfo.CreateImageCommands(imageCreateInfo, stagingBuffer, queue, false, pixelPeek[0].debugName);
#else
            uiImageInfo.CreateImageCommands(imageCreateInfo, stagingBuffer, queue, false);
#endif
        }

        void CreateUIImageView(ImageInfo& uiImageInfo) {

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.image = uiImageInfo.image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = uiImageInfo.mipLevels;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = uiImageInfo.arrayLayers;
            viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

            EWE_VK(vkCreateImageView, EWEDevice::GetVkDevice(), &viewInfo, nullptr, &uiImageInfo.imageView);
        }

        void CreateUISampler(ImageInfo& uiImageInfo) {

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.pNext = nullptr;

            //if(tType == tType_2d){
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.minFilter = VK_FILTER_NEAREST;

            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

            samplerInfo.addressModeV = samplerInfo.addressModeU;
            samplerInfo.addressModeW = samplerInfo.addressModeU;

            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = EWEDevice::GetEWEDevice()->GetProperties().limits.maxSamplerAnisotropy;

            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;

            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.mipLodBias = 0.f;

            //force sampler to not use lowest level by changing this value
            // i.e. samplerInfo.minLod = static_cast<float>(mipLevels / 2);
            samplerInfo.minLod = 0.f;
            samplerInfo.maxLod = 1.f;

            uiImageInfo.sampler = Sampler::GetSampler(samplerInfo);
        }
        //} //namespace internal
    } //namespace UI_Texture
} //namespace EWE