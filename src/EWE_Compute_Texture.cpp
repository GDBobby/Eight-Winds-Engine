#include "Compute/EWE_Compute_Texture.h"

#include <fstream>

namespace EWE {
    void Compute_Texture::createTextureImage() {
        //width[i] = pixelPeek[i].width;
        //height[i] = pixelPeek[i].height;
        VkDeviceSize imageSize = width * height * 4;
        //printf("image dimensions : %d:%d \n", width[i], height[i]);
        //printf("beginning of create image, dimensions - %d : %d : %d \n", width[i], height[i], pixelPeek[i].channels);
        //printf("before creating buffer \n");

        VkImageCreateInfo imageInfo;
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;

        imageInfo.format = texel_format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        auto combinedIndex = device.getComputeGraphicsIndex();
        imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(combinedIndex.size());
        imageInfo.pQueueFamilyIndices = combinedIndex.data();
        if (imageInfo.queueFamilyIndexCount > 1) {
            imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        }
        else {
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(device.getPhysicalDevice(), texel_format, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
            printf("texture image format does not support STORAGE_IMAGE_BIT \n");
            throw std::runtime_error("texture image format does not support STORAGE_IMAGE_BIT!");
        }

       // printf("before image info \n");
        device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
        //printf("before transition \n");
        //device.transitionImageLayout(image, texel_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
        //std::cout << "before transitioning from UNDEFINED to general \n";
        device.transitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, mipLevels);
        //std::cout << "after transitioning from UNDEFINED to general \n";
        //printf("after transition \n");

        //printf("end of create texture image loop %d \n", i);
        
        //printf("before generate mip maps \n");            
        if (mipLevels > 1) {
            generateMipmaps(texel_format);
        }
        //printf("after generate mip maps \n");
    }

    void Compute_Texture::createGraphicsImage(VkPipelineStageFlags stageFlags) {
        VkDeviceSize imageSize = width * height * 4;

        VkImageCreateInfo graphicsImageInfo;
        graphicsImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        graphicsImageInfo.pNext = nullptr;
        graphicsImageInfo.imageType = VK_IMAGE_TYPE_2D;
        graphicsImageInfo.extent.width = width;
        graphicsImageInfo.extent.height = height;
        graphicsImageInfo.extent.depth = 1;
        graphicsImageInfo.mipLevels = mipLevels;
        graphicsImageInfo.arrayLayers = 1;
        //std::cout << "MIP LEVELS WHAT THE FUCK " << graphicsImageInfo.mipLevels << std::endl;

        graphicsImageInfo.format = texel_format;
        graphicsImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        graphicsImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        graphicsImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        graphicsImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        graphicsImageInfo.flags = 0; // Optional

        auto graphicsIndex = device.getGraphicsIndex();
        graphicsImageInfo.queueFamilyIndexCount = 1;
        graphicsImageInfo.pQueueFamilyIndices = &graphicsIndex;

        device.createImageWithInfo(graphicsImageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, graphicsImage, graphicsImageMemory);
        //printf("before transition \n");
        device.transitionImageLayout(graphicsImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels, stageFlags);

        if (mipLevels > 1) {
            //this isnt creating graphics mips currently
            //generateMipmaps(texel_format);
        }
    }

    void Compute_Texture::createTextureImage(std::vector<std::array<float, 4>>& pixels, bool mipmaps) {
        //width[i] = pixelPeek[i].width;
        //height[i] = pixelPeek[i].height;
        VkDeviceSize imageSize = width * height * 4;
        //printf("image dimensions : %d:%d \n", width[i], height[i]);
        //printf("beginning of create image, dimensions - %d : %d : %d \n", width[i], height[i], pixelPeek[i].channels);
        if (mipmaps) {
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
        memcpy(data, pixels.data(), static_cast<size_t>(imageSize));
        //printf("unmapping \n");
        vkUnmapMemory(device.device(), stagingBufferMemory);
        

        VkImageCreateInfo imageInfo;
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;

        imageInfo.format = texel_format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional
        auto combinedIndex = device.getComputeGraphicsIndex();
        imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(combinedIndex.size());
        imageInfo.pQueueFamilyIndices = combinedIndex.data();
        if (imageInfo.queueFamilyIndexCount > 1) {
            imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        }
        else {
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        //printf("before image info \n");
        device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
        //printf("before transition \n");
        device.transitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
        //device.transitionImageLayout(image, texel_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, mipLevels);



        device.copyBufferToImage(stagingBuffer, image, width, height, 1);
        //printf("after copy buffer to image \n");
        //i gotta do this a 2nd time i guess
        //eweDevice.transitionImageLayout(image, texel_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

        vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
        vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
        
        //printf("end of create texture image loop %d \n", i);

        //printf("before generate mip maps \n");            
        if (mipmaps) {
            generateMipmaps(texel_format);
        }
        //printf("after generate mip maps \n");
    }


    void Compute_Texture::generateMipmaps(VkFormat imageFormat) {
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

        int32_t mipWidth = width;
        int32_t mipHeight = height;

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
                1, &barrier);
            //printf("after cmd pipeline barreir \n");
            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
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
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
            //printf("after pipeline barrier 2 \n");
            if (mipWidth > 1) { mipWidth /= 2; }
            if (mipHeight > 1) { mipHeight /= 2; }
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
            1, &barrier);
        //printf("after pipeline barrier 3 \n");
        device.endSingleTimeCommands(commandBuffer);
        //printf("after end single time commands \n");
        
        //printf("end of mip maps \n");
    }

    void Compute_Texture::createTextureImageView() {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = texel_format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;


        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
        
    }
    void Compute_Texture::createGraphicsImageView() {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = graphicsImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = texel_format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;


        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &graphicsImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void Compute_Texture::createTextureSampler(bool settingGraphics) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        //if(tType == tType_2d){
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.addressModeV = samplerInfo.addressModeU;
        samplerInfo.addressModeW = samplerInfo.addressModeU;

        samplerInfo.anisotropyEnable = VK_TRUE;

        // idk why im using this value, but the other guy does// device.getProperties().limits.maxSamplerAnisotropy;
        //sascha willems uses 1.0f
        samplerInfo.maxAnisotropy = 6.f;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; //maybe change this
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
        if (settingGraphics) {
            if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &graphicsSampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }
        }
        
    }

}