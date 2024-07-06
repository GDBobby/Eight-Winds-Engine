#include "EWEngine/Graphics/Texture/Cube_Texture.h"
#include "EWEngine/Graphics/Texture/Sampler.h"
#include "EWEngine/Graphics/TransferCommandManager.h"

namespace EWE {

#ifndef SKYBOX_DIR
#define SKYBOX_DIR "textures/skybox/"
#endif
    namespace Cube_Texture {
        void CreateCubeImage(ImageInfo& cubeTexture, std::vector<PixelPeek>& pixelPeek, Queue::Enum queue) {
            uint64_t layerSize = pixelPeek[0].width * pixelPeek[0].height * 4;
            cubeTexture.arrayLayers = 6;
            VkDeviceSize imageSize = layerSize * cubeTexture.arrayLayers;

            void* data;

            StagingBuffer stagingBuffer{};

            EWEDevice::GetEWEDevice()->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.buffer, stagingBuffer.memory);
            vkMapMemory(EWEDevice::GetVkDevice(), stagingBuffer.memory, 0, imageSize, 0, &data);
            uint64_t memAddress = reinterpret_cast<uint64_t>(data);
            cubeTexture.mipLevels = 1;
            for (int i = 0; i < 6; i++) {
                memcpy(reinterpret_cast<void*>(memAddress), pixelPeek[i].pixels, static_cast<std::size_t>(layerSize)); //static_cast<void*> unnecessary>?
                stbi_image_free(pixelPeek[i].pixels);
                memAddress += layerSize;
            }
            vkUnmapMemory(EWEDevice::GetVkDevice(), stagingBuffer.memory);

            VkImageCreateInfo imageInfo;
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.pNext = nullptr;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = pixelPeek[0].width;
            imageInfo.extent.height = pixelPeek[0].height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = cubeTexture.arrayLayers;

            imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

            EWEDevice* const& eweDevice = EWEDevice::GetEWEDevice();
            Image::CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, cubeTexture.image, cubeTexture.imageMemory);

            SyncHub* syncHub = SyncHub::GetSyncHubInstance();
            SyncedCommandQueue* commandQueue = nullptr;
            {
                VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommand(queue);
                eweDevice->TransitionImageLayoutWithBarrier(cmdBuf,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    cubeTexture.image,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    cubeTexture.mipLevels, cubeTexture.arrayLayers
                );
                if (queue == Queue::graphics) {
                    syncHub->EndSingleTimeCommandGraphics(cmdBuf);
                }
                else if (queue == Queue::transfer) {
                    commandQueue = TransferCommandManager::BeginCommandQueue();
                    commandQueue->Push(cmdBuf);
                }
            }
            {
                VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommandTransfer();
                eweDevice->CopyBufferToImage(cmdBuf, stagingBuffer.buffer, cubeTexture.image, pixelPeek[0].width, pixelPeek[0].height, cubeTexture.arrayLayers);
                if (queue == Queue::graphics) {
                    syncHub->EndSingleTimeCommandGraphics(cmdBuf);
                    stagingBuffer.Free(EWEDevice::GetVkDevice());
                }
                else if (queue == Queue::transfer) {
                    commandQueue->Push(cmdBuf);

                    {

                        VkCommandBuffer transitionCmdBuf = syncHub->BeginSingleTimeCommandTransfer();
                        VkImageMemoryBarrier imageBarrier = eweDevice->TransitionImageLayout(cubeTexture.image,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            cubeTexture.mipLevels, cubeTexture.arrayLayers
                        );
                        imageBarrier.srcQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetTransferIndex();
                        imageBarrier.dstQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetGraphicsIndex();
                        //cmdBuf, 
                        PipelineBarrier pipeBarrier;
                        pipeBarrier.AddBarrier(imageBarrier);
                        pipeBarrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                        pipeBarrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                        pipeBarrier.SubmitBarrier(transitionCmdBuf);

                        commandQueue->Push(transitionCmdBuf);
                        commandQueue->SetBarrier(pipeBarrier);
                        commandQueue->SetStagingBuffer(stagingBuffer);
                        TransferCommandManager::EndCommandQueue(commandQueue);
                    }

                }
            }
        }

        void CreateCubeImageView(ImageInfo& cubeTexture) {

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = cubeTexture.image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = cubeTexture.mipLevels;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = cubeTexture.arrayLayers;
            viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

            EWE_VK_ASSERT(vkCreateImageView(EWEDevice::GetVkDevice(), &viewInfo, nullptr, &cubeTexture.imageView));
        }

        void CreateCubeSampler(ImageInfo& cubeTexture) {

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

            //if(tType == tType_2d){
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.minFilter = VK_FILTER_NEAREST;

            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

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

            cubeTexture.sampler = Sampler::GetSampler(samplerInfo);
        }

        TextureDesc CreateCubeTexture(std::string texPath, Queue::Enum queue, std::string extension) {
            {
                ImageTracker* foundImage = Texture_Manager::FindByPath(texPath);
                if (foundImage != nullptr) {
#ifdef _DEBUG
                    assert(foundImage->usedInTexture.size() == 1 && "cube image used in multiple descriptors");
#endif
                    return *foundImage->usedInTexture.begin();
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
                individualPath += extension;

                pixelPeeks.emplace_back(individualPath);

                assert(!((i > 0) && ((pixelPeeks[i].width != pixelPeeks[i - 1].width) || (pixelPeeks[i].height != pixelPeeks[i - 1].height))) && "failed to load cube texture, bad dimensions");
            }

            ImageTracker* cubeTracker = Texture_Manager::ConstructEmptyImageTracker(texPath);
            ImageInfo& cubeImage = cubeTracker->imageInfo;

            CreateCubeImage(cubeImage, pixelPeeks, queue);
            CreateCubeImageView(cubeImage);
            CreateCubeSampler(cubeImage);

            cubeImage.descriptorImageInfo.sampler = cubeImage.sampler;
            cubeImage.descriptorImageInfo.imageView = cubeImage.imageView;
            cubeImage.descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            return Texture_Manager::EmplaceSkyboxImageTracker(cubeTracker, texPath);
        }
    }//namespace Cube_Texture
}//namespace EWE