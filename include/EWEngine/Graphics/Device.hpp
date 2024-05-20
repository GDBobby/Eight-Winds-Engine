#pragma once

#include "EWEngine/Global_Macros.h"

#include "EWEngine/MainWindow.h"
#include "EWEngine/Systems/SyncHub.h"

// std lib headers

#include <string>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <list>
#include <utility>




namespace EWE {

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueData {
        enum Queue_Enum : uint16_t {
            q_graphics,
            q_present,
            q_compute,
            q_transfer,
        };

        std::array<int, 4> index = { -1, -1, -1, -1 };
        std::array<bool, 4> found = {false, false, false, false};
        std::vector<VkQueueFamilyProperties> queueFamilies{};
        void FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface_);

        bool isComplete() { return found[q_graphics] && found[q_present] && found[q_transfer] && found[q_compute]; }
    };

    //std::vector<VkDeviceQueueCreateInfo> queueInfo;

    class EWEDevice {
        static EWEDevice* eweDevice; //singleton

    public:

        EWEDevice(MainWindow& window);
        ~EWEDevice();

        // Not copyable or movable
        EWEDevice(const EWEDevice&) = delete;
        EWEDevice& operator=(const EWEDevice&) = delete;
        EWEDevice(EWEDevice&&) = delete;
        EWEDevice& operator=(EWEDevice&&) = delete;

        static EWEDevice* GetEWEDevice() {
#ifdef _DEBUG
            assert(eweDevice && "device is nullptr ??");
#endif
            return eweDevice;
        }
        static VkDevice GetVkDevice() {
            return eweDevice->Device();
        }

        VkFormatProperties GetVkFormatProperties(VkFormat imageFormat) {
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);
            return formatProperties;
        }

        VkCommandPool getCommandPool() { return commandPool; }
        VkCommandPool getTransferCommandPool() { return transferCommandPool; }
        VkCommandPool getComputeCommandPool() {
            return computeCommandPool;
        }
        VkDevice Device() { return device_; }
        VkPhysicalDevice GetPhysicalDevice() { return physicalDevice; }
        VkSurfaceKHR surface() { return surface_; }
        VkQueue GetGraphicsQueue() { return graphicsQueue_; }
        uint32_t GetGraphicsIndex() { return queueData.index[QueueData::q_graphics]; }

        VkQueue GetPresentQueue() { return presentQueue_; }
        uint32_t GetPresentIndex() { return queueData.index[QueueData::q_present]; }

        //VkQueue computeQueue() { return computeQueue_; }
        //uint32_t getComputeIndex() { return computeIndex; }

        uint32_t GetTransferIndex() { return queueData.index[QueueData::q_transfer]; }
        VkQueue GetTransferQueue() { return transferQueue_; }

        uint32_t GetComputeIndex() { return queueData.index[QueueData::q_compute]; }
        VkQueue GetComputeQueue() { return computeQueue_; }


        std::string deviceName;

        SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(physicalDevice); }
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        QueueData const& GetPhysicalQueueFamilies() { return queueData; }

        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        // Buffer Helper Functions
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        //void endSingleTimeCommandsSecondThread(VkCommandBuffer commandBuffer);
        //void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CopyBuffer(VkCommandBuffer cmdBuf, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        VkImageMemoryBarrier TransitionImageLayout(VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint8_t layerCount = 1);
        void TransitionImageLayoutWithBarrier(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint8_t layerCount = 1);

        void TransferImageStage(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, std::vector<VkImage> const& images);
        void TransferImageStage(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImage const& image);
        void TransitionFromTransfer(VkCommandBuffer cmdBuf, QueueData::Queue_Enum dstQueueIndex, VkImage const& image, VkImageLayout finalLayout);
        void TransitionFromTransferToGraphics(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImage const& image);

        QueueTransitionContainer* PostTransitionsToGraphics(VkCommandBuffer cmdBuf, uint8_t frameIndex);

        void SetImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange);

        void CopyBufferToImage(VkCommandBuffer cmdBuf, VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height, uint32_t layerCount);
        void CreateImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

        VkInstance GetInstance() { return instance; }
        const VkPhysicalDeviceProperties& GetProperties() { return properties; }

        VkDeviceSize GetMemoryRemaining();



    private:
        VkPhysicalDeviceProperties properties{};
        QueueData queueData;

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
        MainWindow& window;
        VkCommandPool commandPool{ VK_NULL_HANDLE };
        VkCommandPool transferCommandPool{ VK_NULL_HANDLE };
        bool asyncTransferCapable = false;
        VkCommandPool computeCommandPool{ VK_NULL_HANDLE };
        bool asyncComputeCapable = false;

        VkDevice device_;
        VkSurfaceKHR surface_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;
        VkQueue computeQueue_;
        VkQueue transferQueue_;

        SyncHub* syncHub;


        const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME };
        // if doing full screen                                                              VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME

        void CreateInstance();
        void SetupDebugMessenger();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLogicalDevice();

        void CreateCommandPool();
        void CreateComputeCommandPool();
        void CreateTransferCommandPool();


        // helper functions
        bool IsDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char*> GetRequiredExtensions(); //glfw
        bool CheckValidationLayerSupport();
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void HasGflwRequiredInstanceExtensions();
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);



    };

}  // namespace EWE