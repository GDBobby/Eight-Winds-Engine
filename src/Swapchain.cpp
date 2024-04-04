#include "EWEngine/Graphics/Swapchain.hpp"

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
        : windowExtent{ extent }, syncHub{SyncHub::getSyncHubInstance()} {
        init(fullscreen);
        //deviceRef.receiveImageInFlightFences(&imagesInFlight);
    }
    EWESwapChain::EWESwapChain(VkExtent2D extent, bool fullscreen, std::shared_ptr<EWESwapChain> previous)
        : windowExtent{ extent }, oldSwapChain{ previous }, syncHub{ SyncHub::getSyncHubInstance() } {
        init(fullscreen);
        oldSwapChain.reset();
        //deviceRef.receiveImageInFlightFences(&imagesInFlight);
    }
    void EWESwapChain::init(bool fullScreen) {
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

        createSwapChain();
        createImageViews();


        //EXPERIMENTAL
        //createTextureImageViews();
        //

        //createRenderPass();
        createDepthResources();
        initDynamicStruct();
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
    VkResult EWESwapChain::acquireNextImage(uint32_t* imageIndex) {
       // printf("pre-wait for ANI inflightfences \n");

        vkWaitForFences(
            EWEDevice::GetVkDevice(),
            1,
            syncHub->getFlightFence(currentFrame),
            //&inFlightFences[currentFrame],
            VK_TRUE,
            std::numeric_limits<uint64_t>::max()
        );
        //printf("after waiting for fence in ANI \n");
        VkResult result = vkAcquireNextImageKHR(
            EWEDevice::GetVkDevice(),
            swapChain,
            std::numeric_limits<uint64_t>::max(),
            syncHub->getImageAvailableSemaphore(currentFrame),
            //imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
            VK_NULL_HANDLE,
            imageIndex
        );

        return result;
    }

    VkResult EWESwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        syncHub->submitGraphics(submitInfo, currentFrame, imageIndex);

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain;
        presentInfo.pImageIndices = imageIndex;
        auto result = syncHub->presentKHR(presentInfo, currentFrame);
        if (result != VK_SUCCESS) {
            printf("failed to present KHR \n");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        return result;
    }

    void EWESwapChain::createSwapChain() {
        //logFile << "creating swap chain \n";
        //printf("create swap chain \n");

        EWEDevice* const& eweDevice = EWEDevice::GetEWEDevice();
        SwapChainSupportDetails swapChainSupport = eweDevice->GetSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        syncHub->setImageCount(imageCount);


        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        //createInfo.pNext = &surfaceFullScreenExclusiveInfoEXT;
        createInfo.surface = eweDevice->surface(); //im assuming this automatically creates a VkWin32SurfaceCreateInfoKHR if in WIN32

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = { eweDevice->GetGraphicsIndex(), eweDevice->GetPresentIndex() };

        /*
        if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 1;      // Optional
        }
        */
        const bool differentFamilies = (queueFamilyIndices[0] != queueFamilyIndices[1]);
        createInfo.imageSharingMode = (VkSharingMode)differentFamilies; //1 is concurrent, 0 is exclusive
        createInfo.queueFamilyIndexCount = 1 + differentFamilies;

        createInfo.pQueueFamilyIndices = &queueFamilyIndices[0];  //if exclusive, only the first element is read from the array

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

        //logFile << "values initialized, vkcreateswapchain now \n";
        if (vkCreateSwapchainKHR(EWEDevice::GetVkDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            printf("failed to create swap chain \n");
            throw std::runtime_error("failed to create swap chain!");
        }
        //logFile << "afterr vkswapchain \n";
        // we only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        vkGetSwapchainImagesKHR(EWEDevice::GetVkDevice(), swapChain, &imageCount, nullptr);
        if (imageCount <= 0) {
            printf("failed to get swap chain images \n");
		    throw std::runtime_error("failed to get swap chain images!");
        }
            swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(EWEDevice::GetVkDevice(), swapChain, &imageCount, swapChainImages.data());
        //logFile << "after vkgetswapchain images \n";

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
     }

    void EWESwapChain::createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
            for (size_t i = 0; i < swapChainImages.size(); i++) {
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

            if (vkCreateImageView(EWEDevice::GetVkDevice(), &viewInfo, nullptr, &swapChainImageViews[i]) !=
                VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }
        }
    }

    
    void EWESwapChain::initDynamicStruct() {

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

    void EWESwapChain::createDepthResources() {
        VkFormat depthFormat = findDepthFormat();
        swapChainDepthFormat = depthFormat;
        VkExtent2D swapChainExtent = getSwapChainExtent();

        depthImages.resize(imageCount());
        depthImageMemorys.resize(imageCount());
        depthImageViews.resize(imageCount());

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

            EWEDevice::GetEWEDevice()->CreateImageWithInfo(
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

            if (vkCreateImageView(EWEDevice::GetVkDevice(), &viewInfo, nullptr, &depthImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }
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

    VkSurfaceFormatKHR EWESwapChain::chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR EWESwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
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

    VkExtent2D EWESwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
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

    VkFormat EWESwapChain::findDepthFormat() {
        return EWEDevice::GetEWEDevice()->FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

}  // namespace EWE
