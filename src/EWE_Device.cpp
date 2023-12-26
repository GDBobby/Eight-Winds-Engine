#include "EWEngine/graphics/EWE_Device.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>
#include <map>

#define ENGINE_DIR "../"

namespace EWE {

    // local callback functions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
        //throw std::exception("validition layer \n");
        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    // class member functions
    EWEDevice::EWEDevice(MainWindow& window) : window{ window } {
        //printf("device constructor \n");
#if GPU_LOGGING
        {
            std::fstream logFile{ GPU_LOG_FILE };
            logFile << "testing output \n";
            //initialize log file (reset it)

            logFile.close();
        }
#endif

        createInstance();
        //printf("after creating device instance \n");
        setupDebugMessenger();
        //printf("after setup debug messenger \n");
        createSurface();
        //printf("after creating device surface \n");
        pickPhysicalDevice();
        //printf("after picking physical device \n");
        createLogicalDevice();
        //printf("after creating logical device \n");
        createCommandPool();
        createComputeCommandPool();
#if GPU_LOGGING
        //printf("opening file? \n");
        {
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "after creating command pool " << std::endl;
            logFile.close();
        }
#endif
        //printf("after creating command pool, end of device constructor \n");
        createTransferCommandPool();
#if GPU_LOGGING
        //printf("opening file? \n");
        {
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "after creating transfer comand pool " << std::endl;
            logFile.close();
        }
#endif
        //printf("command pool, transfer CP - %lld:%lld \n", commandPool, transferCommandPool);
        //std::cout << "command pool, transfer CP - " << std::hex << commandPool << ":" << transferCommandPool << std::endl;
        //printf("after creating transfer command pool \n");


        //mainThreadID = std::this_thread::get_id();
        syncHub = SyncHub::getSyncHubInstance();
        syncHub->initialize(device_, graphicsQueue_, presentQueue_, computeQueue_, transferQueue_, commandPool, computeCommandPool, transferCommandPool);
        /*
        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        */
    }

    EWEDevice::~EWEDevice() {
        syncHub->destroy(commandPool, computeCommandPool, transferCommandPool);
#if DECONSTRUCTION_DEBUG
        printf("beginning EWEdevice deconstruction \n");
#endif


        vkDestroyCommandPool(device_, commandPool, nullptr);
        vkDestroyCommandPool(device_, computeCommandPool, nullptr);
        vkDestroyCommandPool(device_, transferCommandPool, nullptr);

        vkDestroyDevice(device_, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface_, nullptr);
        vkDestroyInstance(instance, nullptr);

#if DECONSTRUCTION_DEBUG
        printf("end EWEdevice deconstruction \n");
#endif
    }

    void EWEDevice::createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {

#if GPU_LOGGING
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "validation layers requested, but not available!" << std::endl;
            logFile.close();
#endif
            printf("validation layers not available \n");
            throw std::runtime_error("validation layers requested, but not available!");
        }
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Eight Winds";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Eight Winds Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        if (!glfwVulkanSupported()) {
#if GPU_LOGGING
            //printf("opening file? \n");
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "glfw Vulkan is not suppoted! " << std::endl;
            logFile.close();

#endif
        }

        std::vector<const char*> extensions = getRequiredExtensions();
        //extensions.push_back("VK_KHR_get_physical_Device_properties2");
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            printf("failed to create instance \n");
#if GPU_LOGGING
            //printf("opening file? \n");
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "failed to create instance " << std::endl;
            logFile.close();

#endif
            throw std::runtime_error("failed to create instance!");
        }

        hasGflwRequiredInstanceExtensions();
    }

    void EWEDevice::pickPhysicalDevice() {

        uint32_t deviceCount = 16;
        //VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        //deviceCount = 2;
        //printf("enumerate devices result : %lld \n", result);
        std::vector<VkPhysicalDevice> devices(deviceCount);

                            //score     //device iter in the vector
        std::list<std::pair<uint32_t, uint32_t>> deviceScores{};

        VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        std::cout << "Device count: " << deviceCount << std::endl;
        if (result != VK_SUCCESS) {
#if GPU_LOGGING
            //printf("opening file? \n");
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "failed to enumerate physical devices : " << result << '\n' << std::endl;
            logFile.close();
#endif
            throw std::runtime_error("failed to enumerate devices");
        }

        printf("enumerate devices2 result : %d - %u \n", result, deviceCount);
        if (deviceCount == 0) {
            std::cout << "failed to find GPUs with Vulkan support!" << std::endl;
#if GPU_LOGGING
            //printf("opening file? \n");
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "failed to find device with vulkan support\n" << std::endl;
            logFile.close();

#endif
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        for (uint32_t i = 0; i < deviceCount; i++) {

            vkGetPhysicalDeviceProperties(devices[i], &properties);

            uint32_t score = 0;
            score += (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) * 1000;
            score += properties.limits.maxImageDimension2D;
            //properties.limits.maxFramebufferWidth;

            printf("Device Name:Score %s:%d \n", properties.deviceName, score);
            for (auto iter = deviceScores.begin(); iter != deviceScores.end(); iter++) {

                //big to little
                if (iter->first < score) {
                    deviceScores.insert(iter, { score, i });
                }
            }
            if (deviceScores.size() == 0) {
                deviceScores.push_back({ score, i });
            }

            //std::cout << "minimum alignment " << properties.limits.minUniformBufferOffsetAlignment << std::endl;
            //std::cout << "max sampler anisotropy : " << properties.limits.maxSamplerAnisotropy << std::endl;
        }
        //bigger score == gooder device
        //printf("after getting scores \n");
        for (auto iter = deviceScores.begin(); iter != deviceScores.end(); iter++) {
            if (isDeviceSuitable(devices[iter->second])) {
                physicalDevice = devices[iter->second];
                break;
            }
        }
        //printf("before physical device null handle \n");
        if (physicalDevice == VK_NULL_HANDLE) {
            printf("failed to find a suitable GPU! \n");
#if GPU_LOGGING
            //printf("opening file? \n");
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "Failed to find a suitable GPU! " << std::endl;
            logFile.close();
#endif
            throw std::runtime_error("failed to find a suitable GPU!");
        }
        //printf("before get physical device properties \n");
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        std::cout << "Physical Device: " << properties.deviceName << std::endl;
        deviceName = properties.deviceName;
        std::cout << "max ubo, storage : " << properties.limits.maxUniformBufferRange << ":" << properties.limits.maxStorageBufferRange << std::endl;
        std::cout << "minimum alignment " << properties.limits.minUniformBufferOffsetAlignment << std::endl;
        std::cout << "max sampler anisotropy : " << properties.limits.maxSamplerAnisotropy << std::endl;

#if GPU_LOGGING
        //printf("opening file? \n");
        std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
        logFile << "Device Name: " << properties.deviceName << std::endl;
        logFile << "max sampelr allocation : " << properties.limits.maxSamplerAllocationCount << std::endl;
        logFile << "max samplers : " << properties.limits.maxDescriptorSetSamplers << std::endl;
        logFile << "max sampled images : " << properties.limits.maxDescriptorSetSampledImages << std::endl;
        logFile << "max image dimension 2d : " << properties.limits.maxImageDimension2D << std::endl;
        logFile.close();

        //printf("remaining memory : %d \n", GetMemoryRemaining());

#endif

        //printf("max draw vertex count : %d \n", properties.limits.maxDrawVertexCount);
    }

    void EWEDevice::createLogicalDevice() {
        //queueFamilyIndices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        //std::vector<uint32_t> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily, queueFamilyIndices.presentFamily, queueFamilyIndices.transferFamily, queueFamilyIndices.computeFamily };
        if (queueFamilyIndices.familyIndices[QueueFamilyIndices::q_graphics] == queueFamilyIndices.familyIndices[QueueFamilyIndices::q_transfer]) {
            printf("SAME QUEUE FAMILY INDEX ON GRAPHIC AND TRANSFER \n");
        }


        uint32_t graphicsQueueIndex = 0;
        uint32_t presentQueueIndex = 0;
        uint32_t transferQueueIndex = 0;
        uint32_t computeQueueIndex = 0;

        std::vector<std::vector<float>> queuePriorities;
        if (queueFamilyIndices.familyHasIndex[QueueFamilyIndices::q_graphics]) {
            queuePriorities.push_back({});
            queuePriorities[0].push_back(1.0f);
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamilyIndices.familyIndices[QueueFamilyIndices::q_graphics];

            queueCreateInfo.queueCount = 1;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        if (queueFamilyIndices.familyHasIndex[QueueFamilyIndices::q_present]) {
            if (queueFamilyIndices.familyIndices[QueueFamilyIndices::q_present] == queueFamilyIndices.familyIndices[QueueFamilyIndices::q_graphics]) {
                //queueCreateInfos[0].queueCount++;
                presentQueueIndex = 0;

                queuePriorities[0].push_back(0.9f);
            }
            else {
                presentQueueIndex = 0;
                queuePriorities.push_back({});
                queuePriorities.back().push_back(0.9f);
                VkDeviceQueueCreateInfo queueCreateInfo = {};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamilyIndices.familyIndices[QueueFamilyIndices::q_present];

               // queueCreateInfo.queueCount = 1;
                queueCreateInfos.push_back(queueCreateInfo);
            }
        }
        if (queueFamilyIndices.familyHasIndex[QueueFamilyIndices::q_transfer]) {
            if (queueFamilyIndices.familyIndices[QueueFamilyIndices::q_transfer] == queueFamilyIndices.familyIndices[QueueFamilyIndices::q_graphics]) {
                transferQueueIndex = queueCreateInfos[0].queueCount;
                queueCreateInfos[0].queueCount++;
                queuePriorities[0].push_back(0.8f);
            }
            else {
                printf("no queue transfer support \n");
#if GPU_LOGGING
                //printf("opening file? \n");
                std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
                logFile << "no queue transfer support, currently throwing an exception\n" << std::endl;
                logFile.close();

#endif
                throw std::runtime_error("please");
            }

        }
        if (queueFamilyIndices.familyHasIndex[QueueFamilyIndices::q_compute]) {
            computeQueueIndex = queueCreateInfos[0].queueCount;
            queueCreateInfos[0].queueCount++;
            queuePriorities[0].push_back(0.7f);
        }

        for (int i = 0; i < queueCreateInfos.size(); i++) {
            queueCreateInfos[i].pQueuePriorities = queuePriorities[i].data();
        }

        VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_feature{};
        dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamic_rendering_feature.pNext = nullptr;
        dynamic_rendering_feature.dynamicRendering = VK_TRUE;
        VkPhysicalDeviceSynchronization2FeaturesKHR synchronization_2_feature{};
        synchronization_2_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
        synchronization_2_feature.pNext = &dynamic_rendering_feature;

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &synchronization_2_feature;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        /*
        printf("printing active device extensions \n");
        for (int i = 0; i < deviceExtensions.size(); i++) {
            printf("\t %d : %s \n", i, deviceExtensions[i]);
        }
        */
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // might not really be necessary anymore because device specific validation layers
        // have been deprecated
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }
#if GPU_LOGGING
        //printf("opening file? \n");
        {
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "before creating logic device " << std::endl;
            logFile.close();
        }
#endif
        VkResult vkResult = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_);
        if (vkResult != VK_SUCCESS) {
#if GPU_LOGGING
            {
                std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
                logFile << "Device Name: " << properties.deviceName << " - FAILED TO LOAD LOGICAL DEVICE : " << vkResult << std::endl;
                logFile.close();
            }
#endif
            printf("failed to create logical device \n");
            throw std::runtime_error("failed to create logical device!");
        }
#if GPU_LOGGING
        {
            //printf("opening file? \n");
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "after creating logic device " << std::endl;
            logFile.close();
        }
#endif
        graphicsIndex = queueFamilyIndices.familyIndices[QueueFamilyIndices::q_graphics];
        presentIndex = queueFamilyIndices.familyIndices[QueueFamilyIndices::q_present];
        computeIndex = queueFamilyIndices.familyIndices[QueueFamilyIndices::q_compute];
        transferIndex = queueFamilyIndices.familyIndices[QueueFamilyIndices::q_transfer];
        
        std::cout << "getting device queues \n";
        std::cout << "\t graphics family:queue index - " << graphicsIndex << ":" << graphicsQueueIndex << std::endl;
        std::cout << "\t present family:queue index - " << presentIndex << ":" << presentQueueIndex << std::endl;
        std::cout << "\t compute family:queue index - " << computeIndex << ":" << computeQueueIndex << std::endl;
        std::cout << "\t transfer family:queue index - " << transferIndex << ":" << transferQueueIndex << std::endl;
        //printf("before graphics queue \n");
        vkGetDeviceQueue(device_, graphicsIndex, graphicsQueueIndex, &graphicsQueue_);
        //printf("after graphics queue \n");
        vkGetDeviceQueue(device_, presentIndex, presentQueueIndex, &presentQueue_);
        //printf("after present queue \n");
        //vkGetDeviceQueue(device_, indices.computeFamily, 0, &computeQueue_);

        vkGetDeviceQueue(device_, computeIndex, computeQueueIndex, &computeQueue_);

        vkGetDeviceQueue(device_, transferIndex, transferQueueIndex, &transferQueue_);
        printf("after transfer qeuue \n");

#if GPU_LOGGING
        {
            //printf("opening file? \n");
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "after getting device queues " << std::endl;
            logFile.close();
        }
#endif
    }
    void EWEDevice::createComputeCommandPool() {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = computeIndex;

        //sascha doesnt use TRANSIENT_BIT
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(device_, &poolInfo, nullptr, &computeCommandPool) != VK_SUCCESS) {
#if GPU_LOGGING
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "FAILED TO CREATE COMPUTE COMMAND POOL" << std::endl;
            logFile.close();
#endif
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void EWEDevice::createCommandPool() {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = graphicsIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
#if GPU_LOGGING
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "FAILED TO CREATE COMMAND POOL" << std::endl;
            logFile.close();
#endif
            throw std::runtime_error("failed to create command pool!");
        }
    }
    void EWEDevice::createTransferCommandPool() {

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        if (queueFamilyIndices.familyHasIndex[QueueFamilyIndices::q_transfer]) {
            printf("transfer command pool created with transfer queue family \n");
            poolInfo.queueFamilyIndex = transferIndex;
        }
        else {
            printf("transfer command pool created with graphics queue family \n");
            poolInfo.queueFamilyIndex = graphicsIndex;
        }
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkResult vkResult = vkCreateCommandPool(device_, &poolInfo, nullptr, &transferCommandPool);
        if (vkResult != VK_SUCCESS) {
            printf("failed to create transfer command pool : %d \n", vkResult);
#if GPU_LOGGING
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "FAILED TO CREATE TRANSFER COMMAND POOL" << vkResult << std::endl;
            logFile.close();
#endif
            throw std::runtime_error("failed to create transfer command pool!");
        }
    }

    void EWEDevice::createSurface() { window.createWindowSurface(instance, &surface_, GPU_LOGGING); }

    bool EWEDevice::isDeviceSuitable(VkPhysicalDevice device) {
        queueFamilyIndices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);


        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return queueFamilyIndices.isComplete() && extensionsSupported && swapChainAdequate &&
            supportedFeatures.samplerAnisotropy;
    }

    void EWEDevice::populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;  // Optional
    }

    void EWEDevice::setupDebugMessenger() {
        if (!enableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {

            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    bool EWEDevice::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> EWEDevice::getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
#if GPU_LOGGING
        if (glfwExtensions == NULL) {
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "Failed to get required extensions! " << std::endl;
            logFile.close();
        }
#endif

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        //extensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
        //extensions.push_back(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);

        //extension.push_back()
        //extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void EWEDevice::hasGflwRequiredInstanceExtensions() {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        //std::cout << "available extensions:" << std::endl;
        std::unordered_set<std::string> available;
        for (const auto& extension : extensions) {
            //std::cout << "\t" << extension.extensionName << std::endl;
            available.insert(extension.extensionName);
        }

        //std::cout << "required extensions:" << std::endl;
        auto requiredExtensions = getRequiredExtensions();
        for (const auto& required : requiredExtensions) {
            //std::cout << "\t" << required << std::endl;
            if (available.find(required) == available.end()) {
#if GPU_LOGGING
                std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
                logFile << "Extension is not available! : " << required << std::endl;
                logFile.close();
#endif
                throw std::runtime_error("Missing required glfw extension");
            }
        }
    }

    bool EWEDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(
            device,
            nullptr,
            &extensionCount,
            availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            //printf("device extension name available : %s \n", extension.extensionName);
            requiredExtensions.erase(extension.extensionName);
        }
        for (auto iter = requiredExtensions.begin(); iter != requiredExtensions.end(); iter++) {
#if GPU_LOGGING
            std::fstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "REQUIRED EXTENSIONOS NOT SUPPORTED : " << *iter << std::endl;
            logFile.close();
#endif
            printf("REQUIRED EXTENSIONOS NOT SUPPORTED : %s \n", iter->c_str());
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices EWEDevice::findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        printf("queue family count : %d \n", queueFamilyCount);

        std::vector<int> availablePresent{};
        std::vector<int> availableGraphics{};
        std::vector<int> availableTransfer{};
        std::vector<int> availableCompute{};

        std::vector<int> availableCombinedGraphicsPresent{};
        //std::vector<int> availableCombinedComputeGraphicsPresent{}; //dont currently need

        //attempt to get everything in the same queue family
        //if not possible, try to keep graphics and present in the same family
        //transfer and compute may not need graphics support? not really sure
        //currently, set transfer to have graphics support preferably

        bool suitableGraphicsFamily = false;
        bool suitablePresentFamily = false;
        bool suitableComputeFamily = false;
        bool suitableTransferFamily = false;

        int index = 0;
        for (const auto& queueFamily : queueFamilies) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface_, &presentSupport);
            bool graphicsSupport = queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool transferSupport = queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT;
            bool computeSupport = queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT;

            if (presentSupport && graphicsSupport && transferSupport && computeSupport && queueFamily.queueCount >= 3) {
                std::cout << "can stuff all 4 queues into one family \n";
                indices.familyIndices[QueueFamilyIndices::q_graphics] = index;
                indices.familyIndices[QueueFamilyIndices::q_present] = index;
                indices.familyIndices[QueueFamilyIndices::q_compute] = index;
                indices.familyIndices[QueueFamilyIndices::q_transfer] = index;

                indices.familyHasIndex[QueueFamilyIndices::q_graphics] = true;
                indices.familyHasIndex[QueueFamilyIndices::q_present] = true;
                indices.familyHasIndex[QueueFamilyIndices::q_compute] = true;
                indices.familyHasIndex[QueueFamilyIndices::q_transfer] = true;
                return indices;
            }
            index++;
        }
        return indices;

        /*
        index = 0;
        for (const auto& queueFamily : queueFamilies) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface_, &presentSupport);
            printf("queuefamily[%d] : queueCount : %d - queueFlags \n", index, queueFamily.queueCount);
            printf("\t queueFlags - %d:%d:%d:%d \n", (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT), presentSupport, (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT), (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT));
            if (presentSupport) {
                availablePresent.push_back(index);
                if ((queueFamily.queueCount > 0) && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                    availableCombinedGraphicsPresent.push_back(index);
                    *
                    if ((queueFamily.queueCount > 0) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                        availableCombinedComputeGraphicsPresent.push_back(i);
                    }
                    *
                }
            }
            if ((queueFamily.queueCount > 0) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                availableCompute.push_back(index);
            }
            if ((queueFamily.queueCount > 0) && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                availableGraphics.push_back(index);
            }

            if ((queueFamily.queueCount > 0) && ((queueFamily.queueFlags & (VK_QUEUE_GRAPHICS_BIT + VK_QUEUE_TRANSFER_BIT)) == (VK_QUEUE_GRAPHICS_BIT + VK_QUEUE_TRANSFER_BIT))) {
                availableTransfer.push_back(index);
            }
            index++;
        }
        if (availableGraphics.size() == 0) {
            std::cout << "available graphics is 0 \n";
            throw std::exception("0 graphics queues?");
        }
        if (availablePresent.size() == 0) {
            std::cout << "available present is 0 \n";
            throw std::exception("i dont know how to handle this currently");
        }
        if (availableTransfer.size() == 0) {
            std::cout << "available transfer is 0 \n";
            throw std::exception("0 transfer queues????");
        }
        if (availableCompute.size() == 0) {
            std::cout << "available compute is 0 \n";
            throw std::exception("compute queues required");
        }

        int graphicsQueue = availableGraphics[0];
        int transferQueue = availableTransfer[0];
        int presentQueue = availablePresent[0];
        int computeQueue = availableCompute[0];

        bool combinedGraphicsPresent = false;
        bool foundASyncCompute = false;
        bool foundASyncTransfer = false;

        if (availableCombinedGraphicsPresent.size() > 0) {
            graphicsQueue = availableCombinedGraphicsPresent[0];
            presentQueue = availableCombinedGraphicsPresent[0];
            combinedGraphicsPresent = true;

            for (uint16_t i = 0; (i < availableCombinedGraphicsPresent.size()) && !foundASyncCompute; i++) {
                bool notEqual = true;
                for (uint16_t j = 0; j < availableCompute.size() && !foundASyncCompute; j++) {
                    if (availableCombinedGraphicsPresent[i] != availableCompute[j]) {
                        //available combined graphics present that can perform async compute
                        graphicsQueue = availableCombinedGraphicsPresent[i];
                        presentQueue = availableCombinedGraphicsPresent[i];
                        computeQueue = availableCompute[j];
                        foundASyncCompute = true;
                    }
                }
            }
            for (uint16_t i = 0; (i < availableTransfer.size()) && !foundASyncTransfer; i++) {
                if (availableTransfer[i] != graphicsQueue) {
                    transferQueue = i;
                    foundASyncTransfer = true;
                }
            }

        }
        else {
            throw std::exception("not currently supported \n");
        }
        indices.graphicsFamily = graphicsQueue;
        indices.presentFamily = presentQueue;
        indices.computeFamily = computeQueue;
        indices.transferFamily = transferQueue;

        indices.graphicsFamilyHasValue = true;
        indices.presentFamilyHasValue = true;
        indices.computeFamilyHasValue = true;
        indices.transferFamilyHasValue = true;

        std::cout << "selected indices - graphics:" << graphicsQueue << " - present:" << presentQueue << " - compute:" << computeQueue << " - transferQueue:" << transferQueue << std::endl;

        return indices;
        */





        /*
        uint32_t graphicsQueueCount = 0;
        uint32_t presentQueueCount = 0;
        uint32_t computeQueueCount = 0;
        uint32_t transferQueueCount = 0;

        int32_t desiredPresentQueue = -1; //doesnt need graphics capability?
        int32_t desiredGraphicsQueue = -1; //give present queue a higher priority
        int32_t desiredComputeQueue = -1; //not necessary right now
        int32_t desiredTransferQueue = -1; //needs graphic capability, or i need to split images and non-images singletimecommands into different parts

        bool presentEqualGraphics = true;
        bool transferEqualGraphics = true;

        bool presentAndGraphics = false;

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
            printf("queuefamily[%d] : queueCount : %d - queueFlags \n", i, queueFamily.queueCount);
            printf("\t queueFlags - %d:%d:%d:%d \n", (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT), presentSupport, (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT), (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT));
            

            if ((queueCounts[i] > 0) && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                    if (desiredGraphicsQueue == -1) {
                        queueCounts[i]--;
                        desiredGraphicsQueue = i;
                    }
                }
                graphicsQueueCount++;
            }
            if ((queueCounts[i] > 0) && (presentSupport)) { //highest priority
                if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                    if (presentEqualGraphics) {
                        if (desiredPresentQueue != -1) {
                            queueCounts[desiredPresentQueue]++;
                        }
                        else {
                            queueCounts[i]--;
                        }
                        desiredPresentQueue = i;
                        if (i != desiredGraphicsQueue) {
                            desiredPresentQueue = i;
                            presentEqualGraphics = false;
                        }
                    }
                }
                presentQueueCount++;
            }
            if ((queueCounts[i] > 0) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                if (desiredComputeQueue == -1) {
					desiredComputeQueue = i;
				}
                computeQueueCount++;
            }
            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                transferQueueCount++;
                if (((queueFamily.queueFlags & (VK_QUEUE_GRAPHICS_BIT + VK_QUEUE_TRANSFER_BIT)) == (VK_QUEUE_GRAPHICS_BIT + VK_QUEUE_TRANSFER_BIT))) {
                    if (transferEqualGraphics) {
                        if (desiredTransferQueue != -1) {
                            queueCounts[desiredTransferQueue]++;
                        }
                        else {
                            queueCounts[i]--;
                        }

                        desiredTransferQueue = i;                    
                        if (i != desiredGraphicsQueue) {
                            desiredTransferQueue = i;
                            transferEqualGraphics = false;
                        }
                    }

                }
                else {
                    printf("transfer queue flags? : %d \n", queueFamily.queueFlags);
                }
            }
             i++;
        }

        if (desiredPresentQueue == -1) {
            for (const auto& queueFamily : queueFamilies) {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
                if (presentSupport) { //highest priority
                    if (presentEqualGraphics) {
                        desiredPresentQueue = i;
                        if (i != desiredGraphicsQueue) {
                            desiredPresentQueue = i;
                            presentEqualGraphics = false;
                        }
                    }
                    presentQueueCount++;
                }
            }

        }
        i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if ((queueCounts[i] > 0) && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {

            }
        }
        

        
        //if ((desiredTransferQueue >= 0) && (desiredGraphicsQueue >= 0) && (desiredPresentQueue >= 0)) {
        printf("desired queues - %d:%d:%d \n", desiredGraphicsQueue, desiredPresentQueue, desiredTransferQueue);
        indices.graphicsFamily = desiredGraphicsQueue;
        indices.presentFamily = desiredPresentQueue;
        indices.transferFamily = desiredTransferQueue;
        indices.graphicsFamilyHasValue = desiredGraphicsQueue >= 0;
        indices.presentFamilyHasValue = desiredPresentQueue >= 0;
        indices.transferFamilyHasValue = desiredTransferQueue >= 0;
        indices.computeFamily = desiredComputeQueue;
        indices.computeFamilyHasValue = desiredComputeQueue >= 0;
        //return indices;
        //}
        if (!((desiredTransferQueue >= 0) && (desiredGraphicsQueue >= 0) && (desiredPresentQueue >= 0))) {

            printf("did not find all desired queues \n");
            printf("desired queues - %d:%d:%d \n", desiredGraphicsQueue, desiredPresentQueue, desiredTransferQueue);
        }

        printf("queue counts - %d:%d:%d:%d \n", graphicsQueueCount, presentQueueCount, computeQueueCount, transferQueueCount);

        if (!indices.isComplete()) {
            std::cout << "couldnt pull all queues!" << std::endl;
            if (!indices.computeFamilyHasValue) {
#if GPU_LOGGING
                std::fstream logFile{ GPU_LOG_FILE, std::ios::app };
                logFile << "GPU doesn't have compute queue family \n";
                logFile.close();
#endif
                std::cout << "compute family has no value!" << std::endl;
            }
            if (!indices.graphicsFamilyHasValue) {
#if GPU_LOGGING
                std::fstream logFile{ GPU_LOG_FILE, std::ios::app };
                logFile << "GPU doesn't have graphics queue family \n";
                logFile.close();
#endif
                std::cout << "graphics family has no value!" << std::endl;
            }
            if (!indices.presentFamilyHasValue) {
#if GPU_LOGGING
                std::fstream logFile{ GPU_LOG_FILE, std::ios::app };
                logFile << "GPU doesn't have present queue family, testing present=graphics \n";
                logFile.close();
#endif
                presentQueue_ = graphicsQueue_;
                std::cout << "present family has no value!" << std::endl;
            }
            if (!indices.transferFamilyHasValue) {
                printf("GPU doesn't have transfer family, using a second graphics queue instead \n");
#if GPU_LOGGING
                std::fstream logFile{ GPU_LOG_FILE, std::ios::app };
                logFile << "GPU doesn't have transfer queue family, testing trasfer=graphics \n";
                logFile.close();
#endif
                transferQueue_ = graphicsQueue_;

            }
        }
        return indices;
        */
    }

    SwapChainSupportDetails EWEDevice::querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        VkResult vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);
        if (vkResult != VK_SUCCESS) {
#if GPU_LOGGING
            {
                std::fstream file{ GPU_LOG_FILE, std::ios::app };
                file << "failed to get physical device surface capabilities KHR : " << vkResult << "\n";
                file.close();
            }
#endif
        }

        uint32_t formatCount;
        vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);
        if (vkResult != VK_SUCCESS) {
#if GPU_LOGGING
            {
                std::fstream file{ GPU_LOG_FILE, std::ios::app };
                file << "failed to get physical device surface formats : " << vkResult << "\n";
                file.close();
            }
#endif
        }

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);
        if (vkResult != VK_SUCCESS) {
#if GPU_LOGGING
            {
                std::fstream file{ GPU_LOG_FILE, std::ios::app };
                file << "failed to get physical device surface capabilities KHR : " << vkResult << "\n";
                file.close();
            }
#endif
        }

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    VkFormat EWEDevice::findSupportedFormat(
        const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
#if GPU_LOGGING
        {
            std::fstream file{ GPU_LOG_FILE };
            file << "failed to find supported format \n";
        }
#endif
        throw std::runtime_error("failed to find supported format!");
    }

    uint32_t EWEDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        /*
        printf("memory heap count : %d \n", memProperties.memoryHeapCount);
        printf("memory type count : %d \n", memProperties.memoryTypeCount);

        for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++) {
            printf("heap[%d] size : %llu \n", i, memProperties.memoryHeaps->size);
            printf("heap flags : %d \n", memProperties.memoryHeaps->flags);
        }
        */

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            //printf("memory type[%d] heap index : %d \n", i, memProperties.memoryTypes->heapIndex);
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        printf("failed to find suitable memory type \n");
        throw std::runtime_error("failed to find suitable memory type!");
    }

    void EWEDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer);
        if (result != VK_SUCCESS) {
#if GPU_LOGGING
        {
            std::fstream file{ GPU_LOG_FILE };
            file << "failed to create buffer : " << result << "\n";
        }
#endif
        printf("failed to create buffer \n");
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory);
        if (result != VK_SUCCESS) {
#if GPU_LOGGING
        {
            std::fstream file{ GPU_LOG_FILE };
            file << "failed to create image : " << result << "\n";
        }
#endif
        printf("failed to allocate vertex buffer memoy \n");
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(device_, buffer, bufferMemory, 0);
    }

    void EWEDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {

        vkEndCommandBuffer(commandBuffer);

        syncHub->prepTransferSubmission(commandBuffer);
    }
    void EWEDevice::copySecondaryBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmdBuf) {
        //printf("COPY SECONDARY BUFFER, thread ID: %d \n", std::this_thread::get_id());
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(cmdBuf, srcBuffer, dstBuffer, 1, &copyRegion);
    }
    void EWEDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = syncHub->beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void EWEDevice::transitionImageLayout(VkImage &image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t &mipLevels, uint8_t layerCount) {

        VkCommandBuffer commandBuffer = syncHub->beginSingleTimeCommands();
        
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layerCount;

        VkPipelineStageFlags sourceStage, destinationStage;

        if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_GENERAL)) {
            //barrier.srcAccessMask = 0;
            //??? THE BOSS SASCHA WILLEMS DOESNT HAVE AN EXAMPLE
            //std::cout << "landed here, PLEASE " << std::endl;

            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            //copied from vkguide.dev

        }
        else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        endSingleTimeCommands(commandBuffer);
    }
    void EWEDevice::transferImageStage(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, std::vector<VkImage> const& images) {
        assert(images.size() > 0);
        uint32_t imageCount = static_cast<uint32_t>(images.size());

        std::vector<VkImageMemoryBarrier> imageBarriers{};
        imageBarriers.resize(imageCount);
        imageBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarriers[0].pNext = nullptr;
        imageBarriers[0].image = images[0];
        if (imageBarriers[0].image == VK_NULL_HANDLE) {
            printf("transfer image stage image handle is null \n");
            throw std::runtime_error("EMPTY???");
        }

        imageBarriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // or VK_IMAGE_ASPECT_DEPTH_BIT for depth images
        imageBarriers[0].subresourceRange.baseMipLevel = 0;
        imageBarriers[0].subresourceRange.levelCount = 1;
        imageBarriers[0].subresourceRange.baseArrayLayer = 0;
        imageBarriers[0].subresourceRange.layerCount = 1;
        if (computeIndex != graphicsIndex) {
            imageBarriers[0].srcQueueFamilyIndex = computeIndex;
            imageBarriers[0].dstQueueFamilyIndex = graphicsIndex;
        }
        else {
            imageBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        }

        if (srcStage == VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT &&
            ((dstStage == VK_PIPELINE_STAGE_VERTEX_SHADER_BIT) || (dstStage == VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT))
        ) {

            std::cout << " COMPUTE TO GRAPHICS IMAGE TRANSFER \n";
            imageBarriers[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageBarriers[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageBarriers[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for compute shader writes
            imageBarriers[0].dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for transfer read operation
        }
        else if (((srcStage == VK_PIPELINE_STAGE_VERTEX_SHADER_BIT) || (srcStage == VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)) &&
            dstStage == VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
        {
            std::cout << " GRAPHICS TO COMPUTE IMAGE TRANSFER \n";
            imageBarriers[0].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageBarriers[0].newLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageBarriers[0].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for compute shader writes
            imageBarriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for transfer read operation
        }

        for (uint8_t i = 1; i < imageCount; i++) {
            imageBarriers[i] = imageBarriers[0];
            imageBarriers[i].image = images[i];
            if (imageBarriers[i].image == VK_NULL_HANDLE) {
                std::cout << "trying to transfer an invalid image \n";
                throw std::runtime_error("EMPTY???");
            }
        }

        vkCmdPipelineBarrier(
            cmdBuf,
            srcStage,  // pipeline stage
            dstStage, 
            0,
            0, nullptr,
            0, nullptr,
            imageCount, &imageBarriers[0]
        );
    }

    void EWEDevice::transitionImageLayout(VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t& mipLevels, uint8_t layerCount, VkPipelineStageFlags destinationStage) {
        VkCommandBuffer commandBuffer = syncHub->beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layerCount;

        VkPipelineStageFlags sourceStage;
        if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }
        else {
            printf("unsupported image layout transition 2 \n");
            throw std::invalid_argument("unsupported layout transition 2!");
        }

        vkCmdPipelineBarrier(commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        endSingleTimeCommands(commandBuffer);
    }

    void EWEDevice::setImageLayout(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange)
    {
        // Create an image barrier object
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;


        VkPipelineStageFlags sourceStage, destinationStage;

        if ((oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            printf("unsupported image layout transition \n");
            throw std::invalid_argument("unsupported layout transition!");
        }
        //VkPipelineStageFlags srcStageMask{}, dstStageMask{};
        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(
            cmdbuffer,
            sourceStage,
            destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
    }

    void EWEDevice::copyBufferToImage(VkBuffer &buffer, VkImage &image, uint32_t width, uint32_t height, uint32_t layerCount) {
        VkCommandBuffer commandBuffer = syncHub->beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);
        endSingleTimeCommands(commandBuffer);
    }

    void EWEDevice::createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        VkResult result = vkCreateImage(device_, &imageInfo, nullptr, &image);
        if (result != VK_SUCCESS) {
#if GPU_LOGGING
            {
                std::fstream file{ GPU_LOG_FILE, std::ios::app };
                file << "failed to create image : " << result << "\n";
                file.close();
            }
#endif
            printf("failed to create image \n");
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device_, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory);
        if (result != VK_SUCCESS) {
#if GPU_LOGGING
            {
                std::fstream file{ GPU_LOG_FILE, std::ios::app };
                file << "failed to allocate memory : " << result << "\n";
            }
#endif
            printf("failed to allocate memory \n");
            throw std::runtime_error("failed to allocate image memory!");
        }

        result = vkBindImageMemory(device_, image, imageMemory, 0);
        if (result != VK_SUCCESS) {
#if GPU_LOGGING
            {
                std::fstream file{ GPU_LOG_FILE, std::ios::app };
                file << "failed to bind image : " << result << "\n";
            }
#endif
            printf("failed to bind image memory \n");
            throw std::runtime_error("failed to bind image memory!");
        }
    }

    VkDeviceSize EWEDevice::GetMemoryRemaining() {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        uint32_t memoryHeapCount = memoryProperties.memoryHeapCount;
        VkDeviceSize deviceMemoryRemaining = 0;
        for (uint32_t i = 0; i < memoryHeapCount; ++i) {
            VkMemoryHeap memoryHeap = memoryProperties.memoryHeaps[i];
            VkDeviceSize heapSize = memoryHeap.size;
            VkMemoryHeapFlags heapFlags = memoryHeap.flags;

            // Check if the heap is device local and not a host-visible heap
            if (heapFlags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                VkDeviceSize heapUsed = 0;

                // Sum up the memory used on the heap by iterating through memory types
                for (uint32_t j = 0; j < memoryProperties.memoryTypeCount; ++j) {
                    VkMemoryType memoryType = memoryProperties.memoryTypes[j];

                    // Check if the memory type belongs to the current heap
                    if (memoryType.heapIndex == i) {
                        VkDeviceSize memorySize = memoryProperties.memoryHeaps[memoryType.heapIndex].size;
                        heapUsed += memorySize;
                    }
                }

                VkDeviceSize heapRemaining = heapSize - heapUsed;
                std::cout << "Heap " << i << " Remaining Memory: " << heapRemaining << " bytes" << std::endl;
                deviceMemoryRemaining += heapRemaining;
            }
        }
        return deviceMemoryRemaining;
    }
}  // namespace EWE