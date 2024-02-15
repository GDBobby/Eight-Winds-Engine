#pragma once

#include "EWEngine/Graphics/Texture/Texture.h"


#include <cmath>
#include <random>
#include <array>

namespace EWE {

	class Compute_Texture {
    public:
        Compute_Texture(EWEDevice& device, uint16_t width, uint16_t height, bool mipmaps, std::vector<std::array<float, 4>> pixels, VkFormat texel_format = VK_FORMAT_R8G8B8A8_UNORM) : device{ device }, width{ width }, height{ height }, texel_format{ texel_format } {
            //std::cout << "GENERATING WITH PIXELS : " << width << ":" << height << std::endl;
            createTextureImage(pixels, mipmaps);
           // printf("after create image PIXELS \n");
            createTextureImageView();
            //printf("after image view PIXELS \n");
            createTextureSampler();
            //printf("before descriptors PIXELS \n");

            descriptor.sampler = sampler;
            descriptor.imageView = imageView;
            descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            graphicsDescriptor.sampler = sampler;
            graphicsDescriptor.imageView = imageView;
            graphicsDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            //std::cout << "FINISHED GENERATION WITH PIXELS" << std::endl;
        }
        Compute_Texture(EWEDevice& device, uint16_t width, uint16_t height, bool mipmaps = false) : device{ device }, width{ width }, height{ height } {
            //std::cout << " width : height - " << width << " : " << height << std::endl;
            //imageLayout{ descriptorCount }, image{ descriptorCount }, imageMemory{ descriptorCount }, texPath{ texPath }
            //mipLevels.resize(pixelPeek.size(), 1);
#if GPU_LOGGING
            {
                std::ofstream textureLogger{ GPU_LOG_FILE, std::ios::app };
                textureLogger << "creating compute texture : " << "\n";
                textureLogger.close();
            }
#endif
            if (mipmaps) {
                mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))) + 1);
            }
            else {
                mipLevels = 1;
            }

            createTextureImage();
           // printf("after create image \n");
            createTextureImageView();
           // printf("after image view \n");
            createTextureSampler();
            //printf("before descriptors \n");

            //descriptor.resize(1);
            descriptor.sampler = sampler;
            descriptor.imageView = imageView;
            
            descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        //VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        Compute_Texture(EWEDevice& device, uint16_t width, uint16_t height, VkPipelineStageFlags stageFlags, bool mipmaps = false) : device{ device }, width{ width }, height{ height } {

            if (mipmaps) {
                mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))) + 1);
            }
            else {
                mipLevels = 1;
            }
            createGraphicsImage(stageFlags);
            createGraphicsImageView();
            createTextureSampler(true);

            graphicsDescriptor.sampler = graphicsSampler;
            graphicsDescriptor.imageView = graphicsImageView;
            graphicsDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        VkDescriptorImageInfo* getGraphicsDescriptor() {
            return &graphicsDescriptor;
        }
        VkDescriptorImageInfo* getDescriptor() {
            return &descriptor;
        }
        VkImage getImage() {
            return image;
        }

    private:
        void createTextureImage();
        void createTextureImage(std::vector<std::array<float, 4>>& pixels, bool mipmaps);

        void generateMipmaps(VkFormat imageFormat);
        void createTextureImageView();
        void createTextureSampler(bool settingGraphics = false);

        void createGraphicsImage(VkPipelineStageFlags stageFlags);
        void createGraphicsImageView();



        VkImageLayout imageLayout;


        VkImage image{ VK_NULL_HANDLE };
        VkDeviceMemory imageMemory;
        VkImageView imageView;
        VkSampler sampler;

        VkImage graphicsImage;
        VkDeviceMemory graphicsImageMemory;
        VkImageView graphicsImageView;
        VkSampler graphicsSampler;

        uint32_t width;
        uint32_t height;
        uint32_t mipLevels{ 1 };

        EWEDevice& device;


        VkDescriptorImageInfo descriptor;
        VkDescriptorImageInfo graphicsDescriptor;

        VkFormat texel_format = VK_FORMAT_R8G8B8A8_UNORM;
	};
}