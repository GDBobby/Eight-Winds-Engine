#include "EWEngine/Graphics/Textures/Texture.h"

#include <string>
#include <iostream>
#include <filesystem>

#ifndef TEXTURE_DIR
#define TEXTURE_DIR "textures/"
#endif

#define MIPMAP_ENABLED true

namespace EWE {
    std::unordered_map<TextureDSLInfo, std::unique_ptr<EWEDescriptorSetLayout>> TextureDSLInfo::descSetLayouts;


    void TextureDSLInfo::setStageTextureCount(VkShaderStageFlags stageFlag, uint8_t textureCount) {
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
            //im pretty sure that _ALL_GRAPHICS and _ALL are both noob traps, but ill put them in anyways
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

    std::unique_ptr<EWEDescriptorSetLayout> TextureDSLInfo::buildDSL(EWEDevice& device) {
        uint32_t currentBinding = 0;
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //this is 1
        EWEDescriptorSetLayout::Builder dslBuilder{ device };
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

    EWEDescriptorSetLayout* TextureDSLInfo::getDescSetLayout(EWEDevice& device) {
        if (descSetLayouts.contains(*this)) {
            return descSetLayouts.find(*this)->second.get();
        }
        auto emplaceRet = descSetLayouts.try_emplace(*this, buildDSL(device));
        if (!emplaceRet.second) {
            printf("failed to create dynamic desc set layout \n");
            throw std::runtime_error("failed to create dynamic desc set layout");
        }

        return emplaceRet.first->second.get();

    }

    ImageInfo::ImageInfo(EWEDevice& device, PixelPeek& pixelPeek, bool mipmap) {

        createTextureImage(device, pixelPeek, mipmap); //strange to pass in the first, btu whatever
        //printf("after create image \n");
        createTextureImageView(device);
        //printf("after image view \n");
        createTextureSampler(device);

        descriptorImageInfo.sampler = sampler;
        descriptorImageInfo.imageView = imageView;
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    void ImageInfo::destroy(EWEDevice& device) {
        vkDestroySampler(device.device(), sampler, nullptr);
        
        vkDestroyImageView(device.device(), imageView, nullptr);
        
        //printf("after image view destruction \n");
        vkDestroyImage(device.device(), image, nullptr);
        
        //printf("after image destruction \n");
        vkFreeMemory(device.device(), imageMemory, nullptr);
    }

    void ImageInfo::createTextureImage(EWEDevice& device, PixelPeek& pixelPeek, bool mipmapping) {
        int width = pixelPeek.width;
        int height = pixelPeek.height;

        VkDeviceSize imageSize = width * height * 4;
        //printf("image dimensions : %d:%d \n", width[i], height[i]);
        //printf("beginning of create image, dimensions - %d : %d : %d \n", width[i], height[i], pixelPeek[i].channels);
        if (MIPMAP_ENABLED && mipmapping) {
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))) + 1);
        }
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        //printf("before creating buffer \n");

        device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        //printf("before memory mapping \n");
        void* data;
        vkMapMemory(device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
        //printf("memcpy \n");
        memcpy(data, pixelPeek.pixels, static_cast<size_t>(imageSize));
        //printf("unmapping \n");
        vkUnmapMemory(device.device(), stagingBufferMemory);
        //printf("freeing pixels \n");
        stbi_image_free(pixelPeek.pixels);
        //printf("after memory mapping \n");

        VkImageCreateInfo imageInfo;
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;

        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        //printf("before image info \n");
        device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
        //printf("before transition \n");
        device.transitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
        //printf("before copy buffer to image \n");
        device.copyBufferToImage(stagingBuffer, image, width, height, 1);
        //printf("after copy buffer to image \n");
        //i gotta do this a 2nd time i guess
        //eweDevice.transitionImageLayout(image[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels[i]);

        vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
        vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
        //printf("end of create texture image loop %d \n", i);
        
        //printf("before generate mip maps \n");
        if (MIPMAP_ENABLED && mipmapping) {
            generateMipmaps(device, VK_FORMAT_R8G8B8A8_SRGB, width, height);
        }
        //printf("after generate mip maps \n");
    }

    void ImageInfo::createTextureImageView(EWEDevice& device) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void ImageInfo::createTextureSampler(EWEDevice& device) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        //if(tType == tType_2d){
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.addressModeV = samplerInfo.addressModeU;
        samplerInfo.addressModeW = samplerInfo.addressModeU;

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = device.getProperties().limits.maxSamplerAnisotropy;

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

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
        
    }

    void ImageInfo::generateMipmaps(EWEDevice& device, VkFormat imageFormat, int width, int height) {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(device.getPhysicalDevice(), imageFormat, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }
        //printf("before mip map loop? size of image : %d \n", image.size());

        VkCommandBuffer commandBuffer = SyncHub::getSyncHubInstance()->beginSingleTimeCommands();
        //printf("after beginning single time command \n");

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
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
            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
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
            vkCmdBlitImage(commandBuffer,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);
            //printf("after blit image \n");
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
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
        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        //printf("after pipeline barrier 3 \n");
        device.endSingleTimeCommands(commandBuffer);
        //printf("after end single time commands \n");
        
        //printf("end of mip maps \n");
    }
}