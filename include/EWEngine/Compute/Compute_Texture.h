#pragma once

#include "EWEngine/Graphics/Texture/Texture.h"


#include <cmath>
#include <random>
#include <array>

namespace EWE {

	class Compute_Texture {
    public:
        Compute_Texture(uint16_t width, uint16_t height, bool mipmaps = false) : width{ width }, height{ height } {
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
            descriptor.sampler = VK_NULL_HANDLE;
            descriptor.imageView = imageView;
            
            descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        //VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        Compute_Texture(uint16_t width, uint16_t height, VkPipelineStageFlags stageFlags, bool mipmaps = false) : width{ width }, height{ height } {

            if (mipmaps) {
                mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))) + 1);
            }
            else {
                mipLevels = 1;
            }
            createGraphicsImage(stageFlags);
            createGraphicsImageView();
            createTextureSampler(true);

            graphicsDescriptor.sampler = VK_NULL_HANDLE;
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



        VkImageLayout imageLayout;


        VkImage image{ VK_NULL_HANDLE };
        VkDeviceMemory imageMemory;
        VkImageView imageView;

        VkImage graphicsImage;
        VkDeviceMemory graphicsImageMemory;
        VkImageView graphicsImageView;
        VkSampler graphicsSampler;

        uint32_t width;
        uint32_t height;
        uint32_t mipLevels{ 1 };


        VkDescriptorImageInfo descriptor;
        VkDescriptorImageInfo graphicsDescriptor;

        VkFormat texel_format = VK_FORMAT_R8G8B8A8_UNORM;
	};
}
