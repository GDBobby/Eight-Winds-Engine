#include "EWEngine/Graphics/Texture/Cube_Texture.h"

namespace EWE {

#ifndef SKYBOX_DIR
#define SKYBOX_DIR "textures/skybox/"
#endif

    TextureDesc Cube_Texture::createCubeTexture(EWEDevice& device, std::string texPath) {
        auto tmPtr = Texture_Manager::getTextureManagerPtr();
        {
            auto foundImage = tmPtr->imageMap.find(texPath);
            if (foundImage != tmPtr->imageMap.end()) {
#ifdef _DEBUG
                if (foundImage->second->usedInTexture.size() != 1) {
                    //im trusting the user here to not cube images in multiple descriptors
                    //if the application has a need for cube images to be in multipel descriptors, this will need to be reworked
                    //the rework should be as simple as rewriting the return for this scope
                    throw std::runtime_error("cube image used in multiple descriptors");
                }
#endif
                return *foundImage->second->usedInTexture.begin();
            }
        }
        const std::array<std::string, 6> cubeNames = {
            "px", "nx", "py", "ny", "pz", "nz"
        };
        std::vector<PixelPeek> pixelPeeks{};
        pixelPeeks.reserve(6);

        for (int i = 0; i < 6; i++) {
            std::string individualPath = SKYBOX_DIR;
            individualPath += texPath;
            individualPath += cubeNames[i];
            individualPath += ".png";

            pixelPeeks.emplace_back(individualPath);

            if ((i > 0) && ((pixelPeeks[i].width != pixelPeeks[i - 1].width) || (pixelPeeks[i].height != pixelPeeks[i - 1].height))) {
                throw std::runtime_error("failed to load cube texture, bad dimensions");
            }
            
        }
        
        Texture_Manager::ImageTracker* cubeTracker = Texture_Manager::constructEmptyImageTracker(texPath);
        ImageInfo& cubeImage = cubeTracker->imageInfo;

        createCubeImage(cubeImage, device, pixelPeeks);
        createCubeImageView(cubeImage, device);
        createCubeSampler(cubeImage, device);

        cubeImage.descriptorImageInfo.sampler = cubeImage.sampler;
        cubeImage.descriptorImageInfo.imageView = cubeImage.imageView;
        cubeImage.descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        EWEDescriptorWriter descBuilder(*TextureDSLInfo::getSimpleDSL(tmPtr->device, VK_SHADER_STAGE_FRAGMENT_BIT), DescriptorPool_Global);

        descBuilder.writeImage(0, &cubeImage.descriptorImageInfo);
        TextureDesc retDesc = descBuilder.build();
        tmPtr->textureImages.try_emplace(retDesc, std::vector<Texture_Manager::ImageTracker*>{cubeTracker});
        tmPtr->imageMap.emplace(texPath, cubeTracker);

        //cubeVector.emplace_back(EWETexture(eweDevice, texPath, tType_cube));
        //tmPtr->textureMap.emplace(tmPtr->currentTextureCount, );
        
        tmPtr->skyboxID = retDesc;
        tmPtr->currentTextureCount++;
        return retDesc;
    }

    void Cube_Texture::createCubeImage(ImageInfo& cubeTexture, EWEDevice& device, std::vector<PixelPeek>& pixelPeek) {
        uint64_t layerSize = pixelPeek[0].width * pixelPeek[0].height * 4;
        VkDeviceSize imageSize = layerSize * 6;

        void* data;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        vkMapMemory(device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
        uint64_t memAddress = reinterpret_cast<uint64_t>(data);
        cubeTexture.mipLevels = 1;
        for (int i = 0; i < 6; i++) {
            memcpy(reinterpret_cast<void*>(memAddress), pixelPeek[i].pixels, static_cast<size_t>(layerSize)); //static_cast<void*> unnecessary>?
            stbi_image_free(pixelPeek[i].pixels);
            memAddress += layerSize;
        }
        vkUnmapMemory(device.device(), stagingBufferMemory);

        VkImageCreateInfo imageInfo;
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = pixelPeek[0].width;
        imageInfo.extent.height = pixelPeek[0].height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 6;

        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, cubeTexture.image, cubeTexture.imageMemory);

        device.transitionImageLayout(cubeTexture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cubeTexture.mipLevels, 6);
        device.copyBufferToImage(stagingBuffer, cubeTexture.image, pixelPeek[0].width, pixelPeek[0].height, 6);



        vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
        vkFreeMemory(device.device(), stagingBufferMemory, nullptr);

        //i gotta do this a 2nd time i guess
        device.transitionImageLayout(cubeTexture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cubeTexture.mipLevels, 6, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }

    void Cube_Texture::createCubeImageView(ImageInfo& cubeTexture, EWEDevice& device) {

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = cubeTexture.image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = cubeTexture.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 6;
        viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
            
        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &cubeTexture.imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
        
    }

    void Cube_Texture::createCubeSampler(ImageInfo& cubeTexture, EWEDevice& device) {

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        //if(tType == tType_2d){
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        samplerInfo.addressModeV = samplerInfo.addressModeU;
        samplerInfo.addressModeW = samplerInfo.addressModeU;

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = device.getProperties().limits.maxSamplerAnisotropy;

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

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &cubeTexture.sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
        
    }
}