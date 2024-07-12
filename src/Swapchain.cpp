#include "EWEngine/Graphics/Swapchain.hpp"

#include "EWEngine/Graphics/Texture/Image.h"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace EWE {

    EWESwapChain::EWESwapChain(VkExtent2D extent, bool fullscreen)
        : windowExtent{ extent }, syncHub{SyncHub::GetSyncHubInstance()} {

        Init(fullscreen);
        //deviceRef.receiveImageInFlightFences(&imagesInFlight);
    }
    EWESwapChain::EWESwapChain(VkExtent2D extent, bool fullscreen, std::shared_ptr<EWESwapChain> previous)
        : windowExtent{ extent }, oldSwapChain{ previous }, syncHub{ SyncHub::GetSyncHubInstance() } {
        Init(fullscreen);
        oldSwapChain.reset();
        //deviceRef.receiveImageInFlightFences(&imagesInFlight);
    }
    void EWESwapChain::Init(bool fullScreen) {
       // logFile.open("log.log");
       // logFile << "initializing swap chain \n";
        //printf("init swap chain \n");

        /*
        //structures before creating swapchain, need it in swapchaincreateinfo.pNext
        //the example also has VkPhysicalDeviceSurfaceInfo2KHR but that may be HDR and unrelated
        surfaceFullScreenExclusiveInfoEXT.sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT;
        surfaceFullScreenExclusiveInfoEXT.pNext = &surfaceFullScreenExclusiveWin32InfoEXT;
        surfaceFullScreenExclusiveInfoEXT.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;

        surfaceFullScreenExclusiveWin32InfoEXT.sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT;
        surfaceFullScreenExclusiveWin32InfoEXT.pNext = nullptr;
        surfaceFullScreenExclusiveWin32InfoEXT.hmonitor = MonitorFromWindow(MainWindow::GetHWND(), MONITOR_DEFAULTTONEAREST);
        FULL SCREEN SHIT */

        CreateSwapChain();
        CreateImageViews();

        //createRenderPass();
        CreateDepthResources();
        InitDynamicStruct();
        //createFramebuffers();
        //createSyncObjects();

        //acquire fullscreen here
        //acquireFullscreen();

        pipeline_rendering_create_info = VkPipelineRenderingCreateInfo{};
        pipeline_rendering_create_info.pNext = nullptr;
        pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipeline_rendering_create_info.colorAttachmentCount = 1;
        pipeline_rendering_create_info.pColorAttachmentFormats = &swapChainImageFormat;
        pipeline_rendering_create_info.depthAttachmentFormat = swapChainDepthFormat;
    }

    EWESwapChain::~EWESwapChain() {
        //device.removeImageInFlightFences(&imagesInFlight);
        //logFile.close();
        VkDevice const& vkDevice = EWEDevice::GetVkDevice();
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(vkDevice, imageView, nullptr);
        }
        swapChainImageViews.clear();

        if (swapChain != nullptr) {
            vkDestroySwapchainKHR(vkDevice, swapChain, nullptr);
            swapChain = nullptr;
        }

        for (int i = 0; i < depthImages.size(); i++) {
            vkDestroyImageView(vkDevice, depthImageViews[i], nullptr);
            vkDestroyImage(vkDevice, depthImages[i], nullptr);
            vkFreeMemory(vkDevice, depthImageMemorys[i], nullptr);
        }
        /*
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(EWEDevice::GetVkDevice(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(EWEDevice::GetVkDevice(), renderPass, nullptr);
        */
        // cleanup synchronization objects
        //for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        //    vkDestroySemaphore(EWEDevice::GetVkDevice(), renderFinishedSemaphores[i], nullptr);
        //    vkDestroySemaphore(EWEDevice::GetVkDevice(), imageAvailableSemaphores[i], nullptr);
        //    vkDestroyFence(EWEDevice::GetVkDevice(), inFlightFences[i], nullptr);
        //}
    }
    VkResult EWESwapChain::AcquireNextImage(uint32_t* imageIndex) {
       // printf("pre-wait for ANI inflightfences \n");
        printf("before waiting for in flight fence\n");
        EWE_VK_ASSERT(vkWaitForFences(
            EWEDevice::GetVkDevice(),
            1,
            syncHub->GetFlightFence(currentFrame),
            //&inFlightFences[currentFrame],
            VK_TRUE,
            std::numeric_limits<uint64_t>::max()
        ));
        //printf("after waiting for fence in ANI \n");
        printf("before acquiring image\n");
        VkResult result = vkAcquireNextImageKHR(
            EWEDevice::GetVkDevice(),
            swapChain,
            std::numeric_limits<uint64_t>::max(),
            syncHub->GetImageAvailableSemaphore(currentFrame),
            //imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
            VK_NULL_HANDLE,
            imageIndex
        );
        printf("after acquiring image\n");

        return result;
    }

    VkResult EWESwapChain::SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        syncHub->SubmitGraphics(submitInfo, currentFrame, imageIndex);

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain;
        presentInfo.pImageIndices = imageIndex;
        auto result = syncHub->PresentKHR(presentInfo, currentFrame);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        return result;
    }

    void EWESwapChain::CreateSwapChain() {
        //logFile << "creating swap chain \n";
        //printf("create swap chain \n");

        EWEDevice* eweDevice = EWEDevice::GetEWEDevice();
        SwapChainSupportDetails swapChainSupport = eweDevice->GetSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + (swapChainSupport.capabilities.minImageCount < swapChainSupport.capabilities.maxImageCount);
        syncHub->SetImageCount(imageCount);


        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        //createInfo.pNext = &surfaceFullScreenExclusiveInfoEXT;
        createInfo.surface = eweDevice->surface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueData[] = { eweDevice->GetGraphicsIndex(), eweDevice->GetPresentIndex() };

        /*
        if (queueData[0] != queueData[1]) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 1;      // Optional
        }
        */
        const bool differentFamilies = (queueData[0] != queueData[1]);
        createInfo.imageSharingMode = (VkSharingMode)differentFamilies; //1 is concurrent, 0 is exclusive
        createInfo.queueFamilyIndexCount = 1 + differentFamilies;

        createInfo.pQueueFamilyIndices = &queueData[0];  //if exclusive, only the first element is read from the array

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

        //logFile << "values initialized, vkcreateswapchain now \n";
        EWE_VK_ASSERT(vkCreateSwapchainKHR(EWEDevice::GetVkDevice(), &createInfo, nullptr, &swapChain));
        //logFile << "afterr vkswapchain \n";
        // we only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        EWE_VK_ASSERT(vkGetSwapchainImagesKHR(EWEDevice::GetVkDevice(), swapChain, &imageCount, nullptr));
        assert(imageCount > 0 && "failed to get swap chain images\n");

        swapChainImages.resize(imageCount);
        EWE_VK_ASSERT(vkGetSwapchainImagesKHR(EWEDevice::GetVkDevice(), swapChain, &imageCount, swapChainImages.data()));
        //logFile << "after vkgetswapchain images \n";

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
     }

    void EWESwapChain::CreateImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for (std::size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = swapChainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = swapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            EWE_VK_ASSERT(vkCreateImageView(EWEDevice::GetVkDevice(), &viewInfo, nullptr, &swapChainImageViews[i]));
        }
    }

    
    void EWESwapChain::InitDynamicStruct() {

        dynamicStructs.reserve(swapChainImages.size());
        for (uint8_t i = 0; i < swapChainImages.size(); i++) {
			dynamicStructs.emplace_back(swapChainImageViews[i], depthImageViews[i], swapChainExtent.width, swapChainExtent.height);
		}
    }
    
    /*
    void EWESwapChain::createFramebuffers() {
        swapChainFramebuffers.resize(imageCount());
        for (size_t i = 0; i < imageCount(); i++) {
            std::array<VkImageView, 2> attachments = {swapChainImageViews[i], depthImageViews[i]};

            VkExtent2D swapChainExtent = getSwapChainExtent();
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(
                    EWEDevice::GetVkDevice(),
                    &framebufferInfo,
                    nullptr,
                    &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }
    */

    void EWESwapChain::CreateDepthResources() {
        VkFormat depthFormat = FindDepthFormat();
        swapChainDepthFormat = depthFormat;
        VkExtent2D swapChainExtent = GetSwapChainExtent();

        const std::size_t imageCount = swapChainImages.size();
        depthImages.resize(imageCount);
        depthImageMemorys.resize(imageCount);
        depthImageViews.resize(imageCount);

        for (int i = 0; i < depthImages.size(); i++) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = swapChainExtent.width;
            imageInfo.extent.height = swapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            Image::CreateImageWithInfo(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                depthImages[i],
                depthImageMemorys[i]
            );

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = depthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            EWE_VK_ASSERT(vkCreateImageView(EWEDevice::GetVkDevice(), &viewInfo, nullptr, &depthImageViews[i]));
        }
    }
    /*
    bool EWESwapChain::acquireFullscreen() {
        if (vkAcquireFullScreenExclusiveModeEXT(EWEDevice::GetVkDevice(), swapChain) != VK_SUCCESS) {
            printf("failed to acquire full screen \n");
            return false;
        }
        return true;
    }
    bool EWESwapChain::releaseFullscreen() {
        if (vkReleaseFullScreenExclusiveModeEXT(EWEDevice::GetVkDevice(), swapChain) != VK_SUCCESS) {
            printf("failed to release full screen \n");
            return false;
        }
        return true;
    }
    */

    VkSurfaceFormatKHR EWESwapChain::ChooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR EWESwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                std::cout << "Present mode: Mailbox" << std::endl;
                return availablePresentMode;
            }
            
        }
#if GPU_LOGGING
        {
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "could not use present mode: mailbox" << std::endl;
            logFile.close();
        }
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
                std::cout << "Present mode: V-Sync" << std::endl;
                std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
                logFile << "could not use present mode: mailbox, but found VSYNC support" << std::endl;
                logFile.close();

                return availablePresentMode;
            }

        }
        {
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "could not use present mode: mailbox OR VSYNC" << std::endl;
            logFile << "available present mode count : " << availablePresentModes.size() << std::endl;
            for (const auto& availablePresentMode : availablePresentModes) {
                logFile << "available present mode : " << availablePresentMode << std::endl;
            }
            logFile.close();
        }
#endif



      // for (const auto &availablePresentMode : availablePresentModes) {
      //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      //     std::cout << "Present mode: Immediate" << std::endl;
      //     return availablePresentMode;
      //   }
      // }

        std::cout << "Present mode: V-Sync" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D EWESwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } 
        else {
            VkExtent2D actualExtent = windowExtent;
            actualExtent.width = std::max(
                capabilities.minImageExtent.width,
                std::min(capabilities.maxImageExtent.width, actualExtent.width)
            );
            actualExtent.height = std::max(
                capabilities.minImageExtent.height,
                std::min(capabilities.maxImageExtent.height, actualExtent.height)
            );

            return actualExtent;
        }
    }

    VkFormat EWESwapChain::FindDepthFormat() {
        return EWEDevice::GetEWEDevice()->FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }


    EWESwapChain::DynamicStructs::DynamicStructs(VkImageView swapImageView, VkImageView depthImageView, uint32_t width, uint32_t height) {

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

}  // namespace EWE
