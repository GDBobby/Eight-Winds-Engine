#pragma once

#include "Device.hpp"

//#if !defined (_WIN32)
#include <vulkan/vulkan.h>
//#endif

// std lib headers
#include <memory>
#include <string>
#include <vector>

#include <format>
#include <iostream>
//#include <fstream>

namespace EWE {

class EWESwapChain {
    public:

        EWESwapChain(VkExtent2D windowExtent, bool fullscreen);
        EWESwapChain(VkExtent2D windowExtent, bool fullscreen, std::shared_ptr<EWESwapChain> previous);
        ~EWESwapChain();

        EWESwapChain(const EWESwapChain &) = delete;
        EWESwapChain &operator=(const EWESwapChain &) = delete;
        /*
        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        std::vector<VkFramebuffer> getFrameBuffers() { return swapChainFramebuffers; }
        VkRenderPass getRenderPass() { return renderPass; }
        */
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }
        size_t imageCount() { return swapChainImages.size(); }
        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }
        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }

        float extentAspectRatio() {
        return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }
        std::pair<uint32_t, uint32_t> getExtent() { return { swapChainExtent.width, swapChainExtent.height }; }
        VkFormat findDepthFormat();

        VkResult acquireNextImage(uint32_t *imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

        bool compareSwapFormats(const EWESwapChain& swapChain) const {
            return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat;
        }

        //VkPipelineRenderingCreateInfo const& pipeRenderInfo
        VkPipelineRenderingCreateInfo* getPipelineInfo() {
            return &pipeline_rendering_create_info;
        }
        VkImage getImage(uint8_t imageIndex) {
            return swapChainImages[imageIndex];
        }
        void beginRender(VkCommandBuffer commandBuffer, uint8_t imageIndex) {
            //std::cout << "before vkCmdBeginRendering : " << std::endl;
            vkCmdBeginRendering(commandBuffer, &dynamicStructs[imageIndex].render_info); //might need to use the frameIndex from renderer, not sure
            //std::cout << "after vkCmdBeginRendering : " << std::endl;
        }

 private:
    void init(bool fullScreen);
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void initDynamicStruct();
    //void createFramebuffers();
    //void createSyncObjects();
    //bool acquireFullscreen(); FULL SCREEN SHIT
    //bool releaseFullscreen();

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat swapChainImageFormat{};
    VkFormat swapChainDepthFormat{};
    VkExtent2D swapChainExtent{};

    //std::vector<VkFramebuffer> swapChainFramebuffers;
    //VkPipelineRenderingCreateInfo const& pipeRenderInfo;

    struct DynamicStructs {
        VkRenderingInfo render_info{};
        VkRenderingAttachmentInfo color_attachment_info{};
        VkRenderingAttachmentInfo depth_attachment_info{};

        DynamicStructs(VkImageView swapImageView, VkImageView depthImageView, uint32_t width, uint32_t height) {

            color_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            color_attachment_info.pNext = NULL;
            color_attachment_info.imageView = swapImageView;
            color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
            color_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment_info.clearValue = { 0.f, 0.f, 0.f, 1.0f };
            
            depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depth_attachment_info.pNext = NULL;
            depth_attachment_info.imageView = depthImageView;
            depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
            depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth_attachment_info.clearValue = { 1.0f, 0 };
            

            render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
            render_info.pNext = nullptr;
            render_info.renderArea = { 0, 0, width, height };
            render_info.layerCount = 1;
            render_info.colorAttachmentCount = 1;
            render_info.pStencilAttachment = nullptr;
            render_info.pColorAttachments = &color_attachment_info;
            render_info.pDepthAttachment = &depth_attachment_info;
        }
    };
    std::vector<DynamicStructs> dynamicStructs{};

    std::vector<VkImage> depthImages{};
    std::vector<VkDeviceMemory> depthImageMemorys{};
    std::vector<VkImageView> depthImageViews{};
    std::vector<VkImage> swapChainImages{};
    std::vector<VkImageView> swapChainImageViews{};

    VkExtent2D windowExtent{};

    VkSwapchainKHR swapChain{};
    std::shared_ptr<EWESwapChain> oldSwapChain{};

    uint8_t currentFrame = 0;


    std::shared_ptr<SyncHub> syncHub{};


    //VkSurfaceFullScreenExclusiveWin32InfoEXT surfaceFullScreenExclusiveWin32InfoEXT{};
    //VkSurfaceFullScreenExclusiveInfoEXT surfaceFullScreenExclusiveInfoEXT{};


    VkPipelineRenderingCreateInfo pipeline_rendering_create_info{};
};

}  // namespace EWE
