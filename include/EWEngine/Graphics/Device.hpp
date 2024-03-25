#pragma once

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



#define GPU_LOGGING true
#define GPU_LOG_FILE "GPULog.log"

namespace EWE {

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        enum Queue_Enum {
            q_graphics,
            q_present,
            q_compute,
            q_transfer,
        };

        std::array<uint32_t, 4> familyIndices;
        std::array<uint32_t, 4> familyHasIndex = {false, false, false, false};
        /*
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        uint32_t computeFamily;
        uint32_t transferFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool computeFamilyHasValue = false;
        bool transferFamilyHasValue = false;
        */
        bool isComplete() { return familyHasIndex[q_graphics] && familyHasIndex[q_present] && familyHasIndex[q_compute] && familyHasIndex[q_transfer]; }
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

        VkCommandPool getCommandPool() { return commandPool; }
        VkCommandPool getTransferCommandPool() { return transferCommandPool; }
        VkCommandPool getComputeCommandPool() {
            return computeCommandPool;
        }
        VkDevice device() { return device_; }
        VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
        VkSurfaceKHR surface() { return surface_; }
        VkQueue graphicsQueue() { return graphicsQueue_; }
        uint32_t getGraphicsIndex() { return graphicsIndex; }

        VkQueue presentQueue() { return presentQueue_; }
        uint32_t getPresentIndex() { return presentIndex; }

        //VkQueue computeQueue() { return computeQueue_; }
        //uint32_t getComputeIndex() { return computeIndex; }

        uint32_t getTransferIndex() { return transferIndex; }
        VkQueue transferQueue() { return transferQueue_; }

        uint32_t getComputeIndex() { return computeIndex; }
        VkQueue computeQueue() { return computeQueue_; }

        std::vector<uint32_t> getComputeGraphicsIndex() {
            if (computeIndex == graphicsIndex) {
                return { graphicsIndex };
            }
            else {
                return { graphicsIndex, computeIndex };
            }
        }

        std::string deviceName;

        SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        const QueueFamilyIndices& getPhysicalQueueFamilies() { return queueFamilyIndices; }

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        // Buffer Helper Functions
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        //void endSingleTimeCommandsSecondThread(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void copySecondaryBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmdBuf);

        void transitionImageLayout(VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t& mipLevels, uint8_t layerCount = 1);
        void transferImageStage(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, std::vector<VkImage> const& images);
        void transitionImageLayout(VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t& mipLevels, uint8_t layerCount, VkPipelineStageFlags destinationStage);

        void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange);

        void copyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height, uint32_t layerCount);
        void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

        VkInstance getInstance() { return instance; }
        const VkPhysicalDeviceProperties& getProperties() { return properties; }

        VkDeviceSize GetMemoryRemaining();



    private:
        VkPhysicalDeviceProperties properties{};
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();

        void createCommandPool();
        void createComputeCommandPool();
        void createTransferCommandPool();

        QueueFamilyIndices queueFamilyIndices;

        // helper functions
        bool isDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char*> getRequiredExtensions(); //glfw
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
        MainWindow& window;
        VkCommandPool commandPool{ VK_NULL_HANDLE };
        VkCommandPool transferCommandPool{ VK_NULL_HANDLE };
        VkCommandPool computeCommandPool{VK_NULL_HANDLE};

        VkDevice device_;
        VkSurfaceKHR surface_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;
        VkQueue computeQueue_;
        VkQueue transferQueue_;
        uint32_t graphicsIndex;
        uint32_t presentIndex;
        uint32_t computeIndex;
        uint32_t transferIndex;

        //std::thread::id mainThreadID;
        std::shared_ptr<SyncHub> syncHub;

        const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        // if doing full screen                                                              VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME
    };

}  // namespace EWE