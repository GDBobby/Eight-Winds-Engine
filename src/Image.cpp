#include "EWEngine/Graphics/Texture/Image.h"

#include "EWEngine/Graphics/Texture/Sampler.h"
#include "EWEngine/Graphics/TransferCommandManager.h"

#include <string>
#include <iostream>
#include <filesystem>

#include <functional>

#ifndef TEXTURE_DIR
#define TEXTURE_DIR "textures/"
#endif

#define MIPMAP_ENABLED true

namespace EWE {

    namespace Image {
        void CreateImageWithInfo(const VkImageCreateInfo& imageInfo, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
            VkDevice vkDevice = EWEDevice::GetVkDevice();

            EWE_VK_ASSERT(vkCreateImage(vkDevice, &imageInfo, nullptr, &image));

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(vkDevice, image, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = EWEDevice::FindMemoryType(memRequirements.memoryTypeBits, properties);

            EWE_VK_ASSERT(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &imageMemory));

            EWE_VK_ASSERT(vkBindImageMemory(vkDevice, image, imageMemory, 0));
        }
    }


    PixelPeek::PixelPeek(std::string const& path) {
        pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
#if _DEBUG
        assert(pixels && ((width * height) > 0) && path.c_str());
#endif
    }

    std::unordered_map<TextureDSLInfo, EWEDescriptorSetLayout*> TextureDSLInfo::descSetLayouts;


    void TextureDSLInfo::SetStageTextureCount(VkShaderStageFlags stageFlag, uint8_t textureCount) {
        switch (stageFlag) {
        case VK_SHADER_STAGE_VERTEX_BIT: {
            stageCounts[0] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: {
            stageCounts[1] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: {
            stageCounts[2] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_GEOMETRY_BIT: {
            stageCounts[3] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_FRAGMENT_BIT: {
            stageCounts[4] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_COMPUTE_BIT: {
            stageCounts[5] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_ALL_GRAPHICS: {
            //_ALL_GRAPHICS and _ALL feel like noob traps, but ill put them in anyways
            stageCounts[6] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_ALL: {
            stageCounts[7] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT: {
            stageCounts[8] = textureCount;
            break;
        }
        default: {
            printf("unsupported texture shader stage flag \n");
            throw std::runtime_error("invalid shader stage flag for textures");
            break;
        }
        }
    }

    EWEDescriptorSetLayout* TextureDSLInfo::BuildDSL() {
        uint32_t currentBinding = 0;
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //this is 1
        EWEDescriptorSetLayout::Builder dslBuilder{};
        for (uint8_t j = 0; j < 6; j++) {
            
            for (uint8_t i = 0; i < stageCounts[j]; i++) {
                dslBuilder.addBinding(currentBinding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stageFlags);
                currentBinding++;
            }
            stageFlags <<= 1;
        }
        for (uint8_t i = 0; i < stageCounts[6]; i++) {
            dslBuilder.addBinding(currentBinding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS);
            currentBinding++;
        }

        for (uint8_t i = 0; i < stageCounts[7]; i++) {
            dslBuilder.addBinding(currentBinding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL);
            currentBinding++;
        }

        for (uint8_t i = 0; i < stageCounts[8]; i++) {
            dslBuilder.addBinding(currentBinding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
            currentBinding++;
        }
        return dslBuilder.build();
    }

    EWEDescriptorSetLayout* TextureDSLInfo::GetSimpleDSL(VkShaderStageFlags stageFlag) {
        TextureDSLInfo dslInfo{};
        dslInfo.SetStageTextureCount(stageFlag, 1);

        auto dslIter = descSetLayouts.find(dslInfo);
        if (dslIter != descSetLayouts.end()) {
            return dslIter->second;
        }
        
        EWEDescriptorSetLayout::Builder dslBuilder{};
        dslBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stageFlag);
        return descSetLayouts.emplace(dslInfo, dslBuilder.build()).first->second;
    }

    EWEDescriptorSetLayout* TextureDSLInfo::GetDescSetLayout() {
        auto dslIter = descSetLayouts.find(*this);
        if (dslIter != descSetLayouts.end()) {
			return dslIter->second;
		}

#ifdef _DEBUG
        auto emplaceRet = descSetLayouts.try_emplace(*this, BuildDSL());
        assert(emplaceRet.second && "failed to create dynamic desc set layout");
        return emplaceRet.first->second;
#else
        return descSetLayouts.try_emplace(*this, BuildDSL()).first->second;
#endif
    }

    ImageInfo::ImageInfo(PixelPeek& pixelPeek, bool mipmap, Queue::Enum queue) {

        CreateTextureImage(queue, pixelPeek, mipmap); //strange to pass in the first, btu whatever
        //printf("after create image \n");
        CreateTextureImageView();
        //printf("after image view \n");
        CreateTextureSampler();

        descriptorImageInfo.sampler = sampler;
        descriptorImageInfo.imageView = imageView;
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    void ImageInfo::Initialize(std::string const& path, bool mipmap, Queue::Enum queue) {
        PixelPeek pixelPeek{ path };
        Initialize(pixelPeek, mipmap, queue);
    }
    void ImageInfo::Initialize(PixelPeek& pixelPeek, bool mipmap, Queue::Enum queue) {

        CreateTextureImage(queue, pixelPeek, mipmap);
        CreateTextureImageView();
        CreateTextureSampler();

        descriptorImageInfo.sampler = sampler;
        descriptorImageInfo.imageView = imageView;
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    ImageInfo::ImageInfo(std::string const& path, bool mipmap, Queue::Enum queue) {
        PixelPeek pixelPeek{ path };

        CreateTextureImage(queue, pixelPeek, mipmap); //strange to pass in the first, btu whatever
        //printf("after create image \n");
        CreateTextureImageView();
        //printf("after image view \n");
        CreateTextureSampler();

        descriptorImageInfo.sampler = sampler;
        descriptorImageInfo.imageView = imageView;
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }


    void ImageInfo::Destroy() {
        VkDevice const& vkDevice = EWEDevice::GetVkDevice();
        Sampler::RemoveSampler(sampler);
        
        vkDestroyImageView(vkDevice, imageView, nullptr);
        
        //printf("after image view destruction \n");
        vkDestroyImage(vkDevice, image, nullptr);
        
        //printf("after image destruction \n");
        vkFreeMemory(vkDevice, imageMemory, nullptr);
    }


    void ImageInfo::CreateTextureImage(Queue::Enum queue, PixelPeek& pixelPeek, bool mipmapping) {

        StagingBuffer stagingBuffer = StageImage(pixelPeek);
        //printf("image dimensions : %d:%d \n", width[i], height[i]);
        //printf("beginning of create image, dimensions - %d : %d : %d \n", width[i], height[i], pixelPeek[i].channels);
        if (MIPMAP_ENABLED && mipmapping) {
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(pixelPeek.width, pixelPeek.height))) + 1);
        }
        //printf("before creating buffer \n");

        EWEDevice* const eweDevice = EWEDevice::GetEWEDevice();

        VkImageCreateInfo imageInfo;
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = pixelPeek.width;
        imageInfo.extent.height = pixelPeek.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = arrayLayers;

        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        if(MIPMAP_ENABLED && mipmapping){
            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        //printf("before image info \n");
        Image::CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
        //printf("before transition \n");
        
        SyncHub* syncHub = SyncHub::GetSyncHubInstance();
        SyncedCommandQueue* commandQueue = nullptr;
        {
            VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommand(queue);

            EWEDevice::TransitionImageLayoutWithBarrier(cmdBuf,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                image,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                mipLevels
            );
            if (queue == Queue::graphics) {
                syncHub->EndSingleTimeCommandGraphics(cmdBuf);
            }
            else if (queue == Queue::transfer) {
                //this doesnt want a transition, need to fix that
                //syncHub::EndSingleTimeCommandTransfer(cmdBuf);
                commandQueue = TransferCommandManager::BeginCommandQueue();
                commandQueue->Push(cmdBuf);
            }
        }
        //printf("before copy buffer to image \n");
        {
            VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommand(queue);
            eweDevice->CopyBufferToImage(cmdBuf, stagingBuffer.buffer, image, pixelPeek.width, pixelPeek.height, arrayLayers);
            if (queue == Queue::graphics) {
                syncHub->EndSingleTimeCommandGraphics(cmdBuf);
                stagingBuffer.Free(eweDevice->Device());
                if (mipmapping && MIPMAP_ENABLED) {
                    GenerateMipmaps(imageInfo.format, imageInfo.extent.width, imageInfo.extent.height, Queue::graphics);
                }
            }
            else if (queue == Queue::transfer) {
                //this doesnt want a transition, need to fix that
                //syncHub::EndSingleTimeCommandTransfer(cmdBuf);
                commandQueue->Push(cmdBuf);

                commandQueue->stagingBuffer = stagingBuffer;
                {

                    VkCommandBuffer transitionCmdBuf = syncHub->BeginSingleTimeCommandTransfer();
                    VkImageMemoryBarrier imageBarrier{};
                    imageBarrier.pNext = nullptr;
                    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageBarrier.image = image;
                    imageBarrier.srcQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetTransferIndex();
                    imageBarrier.dstQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetGraphicsIndex();
                    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    imageBarrier.subresourceRange.baseArrayLayer = 0;
                    imageBarrier.subresourceRange.layerCount = 1;
                    imageBarrier.subresourceRange.levelCount = 1;
                    imageBarrier.subresourceRange.baseMipLevel = 0;
                    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                    //cmdBuf, 
                    PipelineBarrier pipeBarrier;
                    pipeBarrier.AddBarrier(imageBarrier);
                    pipeBarrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    pipeBarrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    pipeBarrier.SubmitBarrier(transitionCmdBuf);

                    commandQueue->Push(transitionCmdBuf);
                    commandQueue->SetBarrier(pipeBarrier);
                    commandQueue->stagingBuffer = stagingBuffer;

                    if (mipmapping && MIPMAP_ENABLED) {
                        commandQueue->graphicsCallback = [this, format = imageInfo.format, width = imageInfo.extent.width, height = imageInfo.extent.height, queue = Queue::graphics] {
                            this->GenerateMipmaps(format, width, height, queue);
                        };
                    }
                    TransferCommandManager::EndCommandQueue(commandQueue);
                }
            }
        }
    }

    StagingBuffer ImageInfo::StageImage(PixelPeek& pixelPeek) {
        const int width = pixelPeek.width;
        const int height = pixelPeek.height;

        const VkDeviceSize imageSize = width * height * 4;

        StagingBuffer stagingBuffer;

        EWEDevice* const eweDevice = EWEDevice::GetEWEDevice();
        eweDevice->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.buffer, stagingBuffer.memory);
        void* data;
        vkMapMemory(eweDevice->Device(), stagingBuffer.memory, 0, imageSize, 0, &data);
        //printf("memcpy \n");
        memcpy(data, pixelPeek.pixels, static_cast<std::size_t>(imageSize));
        //printf("unmapping \n");
        vkUnmapMemory(eweDevice->Device(), stagingBuffer.memory);
        //printf("freeing pixels \n");
        stbi_image_free(pixelPeek.pixels);

        return stagingBuffer;
    }
    StagingBuffer ImageInfo::StageImage(std::vector<PixelPeek>& pixelPeek) {
        const int width = pixelPeek[0].width;
        const int height = pixelPeek[0].height;
        const uint64_t layerSize = width * height * 4;
        const VkDeviceSize imageSize = layerSize * pixelPeek.size();
        void* data;

        StagingBuffer stagingBuffer{};

        EWEDevice::GetEWEDevice()->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.buffer, stagingBuffer.memory);
        vkMapMemory(EWEDevice::GetVkDevice(), stagingBuffer.memory, 0, imageSize, 0, &data);
        uint64_t memAddress = reinterpret_cast<uint64_t>(data);

        for (int i = 0; i < pixelPeek.size(); i++) {
            memcpy(reinterpret_cast<void*>(memAddress), pixelPeek[i].pixels, static_cast<std::size_t>(layerSize)); //static_cast<void*> unnecessary>?
            stbi_image_free(pixelPeek[i].pixels);
            memAddress += layerSize;
        }
        vkUnmapMemory(EWEDevice::GetVkDevice(), stagingBuffer.memory);

        return stagingBuffer;
    }
    void ImageInfo::CreateTextureImageView() {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = arrayLayers;

        EWE_VK_ASSERT(vkCreateImageView(EWEDevice::GetVkDevice(), &viewInfo, nullptr, &imageView));
    }

    void ImageInfo::CreateTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.pNext = nullptr;

        //if(tType == tType_2d){
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = EWEDevice::GetEWEDevice()->GetProperties().limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;

        //force sampler to not use lowest level by changing this value
        // i.e. samplerInfo.minLod = static_cast<float>(mipLevels / 2);
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);

        sampler = Sampler::GetSampler(samplerInfo);
    }

    //this needs to happen in the graphics queue
    void ImageInfo::GenerateMipmaps(const VkFormat imageFormat, const int width, const int height, Queue::Enum srcQueue){
        SyncHub* syncHub = SyncHub::GetSyncHubInstance();
        VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommandGraphics();

        GenerateMipmaps(cmdBuf, imageFormat, width, height, srcQueue);

        syncHub->EndSingleTimeCommandGraphics(cmdBuf);
    }

    void ImageInfo::GenerateMipmaps(VkCommandBuffer cmdBuf, const VkFormat imageFormat, int width, int height, Queue::Enum srcQueue) {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties = EWEDevice::GetEWEDevice()->GetVkFormatProperties(imageFormat);

        assert((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) && "texture image format does not support linear blitting");
        //printf("before mip map loop? size of image : %d \n", image.size());

        //printf("after beginning single time command \n");

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image; 
        barrier.srcQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetQueueIndex(srcQueue);
        barrier.dstQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetGraphicsIndex(); //graphics queue is the only queue that can support vkCmdBlitImage
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            //printf("before cmd pipeline barrier \n");
            //this barrier right here needs a transfer queue partner
            vkCmdPipelineBarrier(cmdBuf,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
            //this is going to be set again for each mip level, extremely small performance hit, potentially optimized away
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            //printf("after cmd pipeline barreir \n");
            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { width, height, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { width > 1 ? width / 2 : 1, height > 1 ? height / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;
            //printf("before blit image \n");
            vkCmdBlitImage(cmdBuf,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);
            //printf("after blit image \n");
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(cmdBuf,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
            //printf("after pipeline barrier 2 \n");
            if (width > 1) { width /= 2; }
            if (height > 1) { height /= 2; }
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        //printf("before pipeline barrier 3 \n");
        vkCmdPipelineBarrier(
            cmdBuf,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        //printf("after pipeline barrier 3 \n");
        //printf("after end single time commands \n");
        
        //printf("end of mip maps \n");
    }
    void ImageInfo::GenerateMipmaps(VkImage image, uint8_t mipLevels, const VkFormat imageFormat, int width, int height, Queue::Enum srcQueue) {
        SyncHub* syncHub = SyncHub::GetSyncHubInstance();
        VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommandGraphics();
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties = EWEDevice::GetEWEDevice()->GetVkFormatProperties(imageFormat);

        assert((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) && "texture image format does not support linear blitting");
        //printf("before mip map loop? size of image : %d \n", image.size());

        //printf("after beginning single time command \n");

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetQueueIndex(srcQueue);
        barrier.dstQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetGraphicsIndex(); //graphics queue is the only queue that can support vkCmdBlitImage
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            //printf("before cmd pipeline barrier \n");
            //this barrier right here needs a transfer queue partner
            vkCmdPipelineBarrier(cmdBuf,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
            //this is going to be set again for each mip level, extremely small performance hit, potentially optimized away
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            //printf("after cmd pipeline barreir \n");
            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { width, height, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { width > 1 ? width / 2 : 1, height > 1 ? height / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;
            //printf("before blit image \n");
            vkCmdBlitImage(cmdBuf,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);
            //printf("after blit image \n");
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(cmdBuf,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
            //printf("after pipeline barrier 2 \n");
            if (width > 1) { width /= 2; }
            if (height > 1) { height /= 2; }
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        //printf("before pipeline barrier 3 \n");
        vkCmdPipelineBarrier(
            cmdBuf,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        syncHub->EndSingleTimeCommandGraphics(cmdBuf);
    }
}