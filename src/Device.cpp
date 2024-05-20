#include "EWEngine/Graphics/Device.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


// std headers
#include <cstring>
#include <iostream>

#include <unordered_set>
#include <stack>
#include <set>

#define ENGINE_DIR "../"

namespace EWE {
    EWEDevice* EWEDevice::eweDevice = nullptr;


#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    //const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const bool enableValidationLayers = true;
#endif

    // local callback functions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        switch (messageSeverity) {

            //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001,
            //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010,
            //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100,
            //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000,
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				std::cout << "validation verbose: " << messageType << ":" << pCallbackData->pMessage << '\n' << std::endl;
				break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                std::cout << "validation info: " << messageType << ":" << pCallbackData->pMessage << '\n' << std::endl;
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                std::cout << "validation warning: " << messageType << ":" << pCallbackData->pMessage << '\n' << std::endl;
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                std::cout << "validation error: " << messageType << ":" << pCallbackData->pMessage << '\n' << std::endl;
                assert(0 && "validation layer error");
				break;
            default:{
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
                break;
            }
        }
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


    void QueueData::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface_) {

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        
        queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        printf("queue family count : %d:%lu \n", queueFamilyCount, queueFamilies.size());

        //i want a designated graphics/present queue, or throw an error
        //i want a dedicated async compute queue
        //i want a dedicated async transfer queue
        //if i dont get two separate dedicated queues for transfer and compute, attempt to combine those 2
        //otherwise, flop them in the graphics queue

        bool foundDedicatedGraphicsPresent = false;
#if 1 //queue debugging
        for (const auto& queueFamily : queueFamilies) {
            printf("queue properties - %d:%d:%d\n", queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT, queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT, queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT);
        }
#endif

        //fidning graphics/present queue
        int currentIndex = 0;
        for (const auto& queueFamily : queueFamilies) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, currentIndex, surface_, &presentSupport);
            bool graphicsSupport = queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool computeSupport = queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT;
            if ((presentSupport && graphicsSupport && computeSupport) == true) {
                //im pretty sure compute and graphics in a queue is a vulkan requirement, but not 100%
                foundDedicatedGraphicsPresent = true;
                index[QueueData::q_graphics] = currentIndex;
                index[QueueData::q_present] = currentIndex;
                found[QueueData::q_graphics] = true;
                found[QueueData::q_present] = true;
                break;
            }
            currentIndex++;
        }
        assert(foundDedicatedGraphicsPresent && "failed to find a graphics/present queue that could also do compute");

        //re-searching for compute and transfer queues
        std::stack<int> dedicatedComputeFamilies{};
        std::stack<int> dedicatedTransferFamilies{};
        std::stack<int> combinedTransferComputeFamilies{};

        currentIndex = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (currentIndex == index[QueueData::q_graphics]) {
                currentIndex++;
                continue;
            }
            bool computeSupport = queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT;
            bool transferSupport = queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT;
            printf("queue support[%d] - %d:%d \n", currentIndex, computeSupport, transferSupport);
            if (computeSupport && transferSupport) {
                combinedTransferComputeFamilies.push(currentIndex);
            }
            else if (computeSupport) {
                dedicatedComputeFamilies.push(currentIndex);
            }
            else if (transferSupport) {
                dedicatedTransferFamilies.push(currentIndex);
            }
            currentIndex++;
        }
        printf("after the queue family \n");
        if (dedicatedComputeFamilies.size() > 0) {
            index[QueueData::q_compute] = dedicatedComputeFamilies.top();
            found[QueueData::q_compute] = true;
        }
        if (dedicatedTransferFamilies.size() > 0) {
            index[QueueData::q_transfer] = dedicatedTransferFamilies.top();
            found[QueueData::q_transfer] = true;
        }
        if (combinedTransferComputeFamilies.size() > 0) {
            if ((!found[QueueData::q_compute]) && (!found[QueueData::q_transfer])) {
                assert(combinedTransferComputeFamilies.size() >= 2 && "not enough queues for transfer and compute");

                index[QueueData::q_compute] = combinedTransferComputeFamilies.top();
                found[QueueData::q_compute] = true;
                combinedTransferComputeFamilies.pop();

                index[QueueData::q_transfer] = combinedTransferComputeFamilies.top();
                found[QueueData::q_transfer] = true;
            }
            else if (!found[QueueData::q_compute]) {
                index[QueueData::q_compute] = combinedTransferComputeFamilies.top();
                found[QueueData::q_compute] = true;
            }
            else if (!found[QueueData::q_transfer]) {
                index[QueueData::q_transfer] = combinedTransferComputeFamilies.top();
                found[QueueData::q_transfer] = true;
            }
        }
        assert(found[QueueData::q_compute] && found[QueueData::q_transfer] && "did not find a dedicated transfer or compute queue");
        assert(index[QueueData::q_compute] != index[QueueData::q_transfer] && "compute queue and transfer q should not be the same");
    }

    // class member functions
    EWEDevice::EWEDevice(MainWindow& window) : window{ window } {
        //printf("device constructor \n");
        assert(eweDevice == nullptr && "EWEDevice already exists");
        eweDevice = this;
#if GPU_LOGGING
        {
            std::ofstream logFile{ GPU_LOG_FILE, std::ofstream::trunc };
            logFile << "testing output \n";
            //initialize log file (reset it)

            logFile.close();
        }
#endif

        CreateInstance();
        //printf("after creating device instance \n");
        SetupDebugMessenger();
        //printf("after setup debug messenger \n");
        CreateSurface();
        //printf("after creating device surface \n");
        PickPhysicalDevice();
        //printf("after picking physical device \n");
        CreateLogicalDevice();
        //printf("after creating logical device \n");
        CreateCommandPool();
        CreateComputeCommandPool();
#if GPU_LOGGING
        //printf("opening file? \n");
        {
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "after creating command pool " << std::endl;
            logFile.close();
        }
#endif
        //printf("after creating command pool, end of device constructor \n");
        CreateTransferCommandPool();
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
        SyncHub::Initialize(device_, graphicsQueue_, presentQueue_, computeQueue_, transferQueue_, commandPool, computeCommandPool, transferCommandPool, queueData.index[QueueData::Queue_Enum::q_transfer]);
        syncHub = SyncHub::GetSyncHubInstance();
        
        /*
        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        */
    }

    EWEDevice::~EWEDevice() {
        syncHub->Destroy(commandPool, computeCommandPool, transferCommandPool);
#if DECONSTRUCTION_DEBUG
        printf("beginning EWEdevice deconstruction \n");
#endif


        vkDestroyCommandPool(device_, commandPool, nullptr);
        if (computeCommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_, computeCommandPool, nullptr);
        }
        if (transferCommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_, transferCommandPool, nullptr);
        }
        eweDevice = nullptr;
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

    void EWEDevice::CreateInstance() {
        if (enableValidationLayers && !CheckValidationLayerSupport()) {

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

        std::vector<const char*> extensions = GetRequiredExtensions();
        //extensions.push_back("VK_KHR_get_physical_Device_properties2");
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        EWE_VK_ASSERT(vkCreateInstance(&createInfo, nullptr, &instance));

        HasGflwRequiredInstanceExtensions();
    }

    void EWEDevice::PickPhysicalDevice() {

        uint32_t deviceCount = 16;
        //deviceCount = 2;
        //printf("enumerate devices result : %lld \n", result);
        std::vector<VkPhysicalDevice> devices(deviceCount);

                            //score     //device iter in the vector
        std::list<std::pair<uint32_t, uint32_t>> deviceScores{};

        EWE_VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));
        std::cout << "Device count: " << deviceCount << std::endl;

        //printf("enumerate devices2 result : %u \n", deviceCount);
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
            std::string deviceNameTemp = properties.deviceName;
            score += ((deviceNameTemp.find("AMD") != deviceNameTemp.npos) && (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)) * 100000;
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
            if (IsDeviceSuitable(devices[iter->second])) {
                physicalDevice = devices[iter->second];
                break;
            }
            else {
                printf("device unsuitable \n");
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

    void EWEDevice::CreateLogicalDevice() {

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::vector<std::vector<float>> queuePriorities{};

        queueData.index[QueueData::q_present] = queueData.index[QueueData::q_graphics];

        queuePriorities.emplace_back().push_back(1.f);
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueData.index[QueueData::q_graphics];

        queueCreateInfo.queueCount = 1;
        queueCreateInfos.push_back(queueCreateInfo);

        queuePriorities.emplace_back().push_back(0.9f);
        queueCreateInfo.queueFamilyIndex = queueData.index[QueueData::q_compute];
        queueCreateInfos.push_back(queueCreateInfo);

        queuePriorities.emplace_back().push_back(0.8f);
        queueCreateInfo.queueFamilyIndex = queueData.index[QueueData::q_transfer];
        queueCreateInfos.push_back(queueCreateInfo);
        //not currently doing a separate queue for present. its currently combined with graphics
        //not sure how much wrok it'll be to fix that, i'll come back to it later


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
        EWE_VK_ASSERT(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_));
#if GPU_LOGGING
        {
            //printf("opening file? \n");
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "after creating logic device " << std::endl;
            logFile.close();
        }
#endif
        
        std::cout << "getting device queues \n";
        std::cout << "\t graphics family:queue index - " << queueData.index[QueueData::q_graphics] << std::endl;
        std::cout << "\t present family:queue index - " << queueData.index[QueueData::q_present] << std::endl;
        std::cout << "\t compute family:queue index - " << queueData.index[QueueData::q_compute] << std::endl;
        std::cout << "\t transfer family:queue index - " << queueData.index[QueueData::q_transfer] << std::endl;
        //printf("before graphics queue \n");
        vkGetDeviceQueue(device_, queueData.index[QueueData::q_graphics], 0, &graphicsQueue_);
        //printf("after graphics queue \n");
        if (queueData.index[QueueData::q_graphics] != queueData.index[QueueData::q_present]) {
            vkGetDeviceQueue(device_, queueData.index[QueueData::q_present], 0, &presentQueue_);
        }

        vkGetDeviceQueue(device_, queueData.index[QueueData::q_compute], 0, &computeQueue_);

        vkGetDeviceQueue(device_, queueData.index[QueueData::q_transfer], 0, &transferQueue_);
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
    void EWEDevice::CreateComputeCommandPool() {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueData.index[QueueData::q_compute];

        //sascha doesnt use TRANSIENT_BIT
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        EWE_VK_ASSERT(vkCreateCommandPool(device_, &poolInfo, nullptr, &computeCommandPool));
    }

    void EWEDevice::CreateCommandPool() {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueData.index[QueueData::q_graphics];
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        EWE_VK_ASSERT(vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool));
    }
    void EWEDevice::CreateTransferCommandPool() {

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        if (queueData.found[QueueData::q_transfer]) {
            printf("transfer command pool created with transfer queue family \n");
            poolInfo.queueFamilyIndex = queueData.index[QueueData::q_transfer];
        }
        else {
            printf("transfer command pool created with graphics queue family \n");
            poolInfo.queueFamilyIndex = queueData.index[QueueData::q_graphics];
        }
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        EWE_VK_ASSERT(vkCreateCommandPool(device_, &poolInfo, nullptr, &transferCommandPool));
    }

    void EWEDevice::CreateSurface() { window.createWindowSurface(instance, &surface_, GPU_LOGGING); }

    bool EWEDevice::IsDeviceSuitable(VkPhysicalDevice device) {
        queueData.FindQueueFamilies(device, surface_);

        bool extensionsSupported = CheckDeviceExtensionSupport(device);


        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return queueData.isComplete() && extensionsSupported && swapChainAdequate &&
            supportedFeatures.samplerAnisotropy;
    }

    void EWEDevice::PopulateDebugMessengerCreateInfo(
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

    void EWEDevice::SetupDebugMessenger() {
        if (!enableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {

            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    bool EWEDevice::CheckValidationLayerSupport() {
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

    std::vector<const char*> EWEDevice::GetRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
#if GPU_LOGGING
        if (glfwExtensions == nullptr) {
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

    void EWEDevice::HasGflwRequiredInstanceExtensions() {
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
        auto requiredExtensions = GetRequiredExtensions();
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

    bool EWEDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
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
            std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
            logFile << "REQUIRED EXTENSIONOS NOT SUPPORTED : " << *iter << std::endl;
            logFile.close();
#endif
            printf("REQUIRED EXTENSIONOS NOT SUPPORTED : %s \n", iter->c_str());
        }

        return requiredExtensions.empty();
    }


    SwapChainSupportDetails EWEDevice::QuerySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        EWE_VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities));

        uint32_t formatCount;
        EWE_VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr));

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        EWE_VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr));

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    VkFormat EWEDevice::FindSupportedFormat(
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
            std::ofstream file{ GPU_LOG_FILE };
            file << "failed to find supported format \n";
        }
#endif
        throw std::runtime_error("failed to find supported format!");
    }

    uint32_t EWEDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
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

    void EWEDevice::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        EWE_VK_ASSERT(vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        EWE_VK_ASSERT(vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory));

        vkBindBufferMemory(device_, buffer, bufferMemory, 0);
    }

    void EWEDevice::CopyBuffer(VkCommandBuffer cmdBuf, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        //printf("COPY SECONDARY BUFFER, thread ID: %d \n", std::this_thread::get_id());
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(cmdBuf, srcBuffer, dstBuffer, 1, &copyRegion);
    }

    //need to find the usage and have it create the single time command
    VkImageMemoryBarrier EWEDevice::TransitionImageLayout(VkImage &image, VkImageLayout srcLayout, VkImageLayout dstLayout, uint32_t mipLevels, uint8_t layerCount) {
        
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = srcLayout;
        barrier.newLayout = dstLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layerCount;

        switch (srcLayout)
        {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            // Image layout is undefined (or does not matter).
            // Only valid as initial layout. No flags required.
            barrier.srcAccessMask = 0;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            // Image is preinitialized.
            // Only valid as initial layout for linear images; preserves memory
            // contents. Make sure host writes have finished.
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image is a color attachment.
            // Make sure writes to the color buffer have finished
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image is a depth/stencil attachment.
            // Make sure any writes to the depth/stencil buffer have finished.
            barrier.srcAccessMask
                = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image is a transfer source.
            // Make sure any reads from the image have finished
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image is a transfer destination.
            // Make sure any writes to the image have finished.
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image is read by a shader.
            // Make sure any shader reads from the image have finished
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_GENERAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.srcAccessMask |= VK_ACCESS_SHADER_READ_BIT * (dstLayout == VK_IMAGE_LAYOUT_GENERAL);
        default:
            /* Value not used by callers, so not supported. */
            assert(false && "unsupported src layout transition");
        }

        // Target layouts (new)
        // The destination access mask controls the dependency for the new image
        // layout.
        switch (dstLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image will be used as a transfer destination.
            // Make sure any writes to the image have finished.
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image will be used as a transfer source.
            // Make sure any reads from and writes to the image have finished.
            barrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image will be used as a color attachment.
            // Make sure any writes to the color buffer have finished.
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image layout will be used as a depth/stencil attachment.
            // Make sure any writes to depth/stencil buffer have finished.
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image will be read in a shader (sampler, input attachment).
            // Make sure any writes to the image have finished.
            if (barrier.srcAccessMask == 0) {
                barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_GENERAL:
            barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            /* Value not used by callers, so not supported. */
#ifdef _DEBUG
            assert(false && "unsupported dst layout transition");
#else
    #if defined(_MSC_VER) && !defined(__clang__) // MSVC
        __assume(false);
    #else // GCC, Clang
        __builtin_unreachable();
    #endif
#endif
        }

        return barrier;
    }
    void EWEDevice::TransitionImageLayoutWithBarrier(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImage& image, VkImageLayout srcLayout, VkImageLayout dstLayout, uint32_t mipLevels, uint8_t layerCount){
        VkImageMemoryBarrier imageBarrier{TransitionImageLayout(image, srcLayout, dstLayout, mipLevels, layerCount)};
        vkCmdPipelineBarrier(cmdBuf,
            srcStageMask, dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageBarrier
        );
    }


#define BARRIER_DEBUGGING false
    void EWEDevice::TransferImageStage(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImage const& image) {
        VkImageMemoryBarrier imageBarrier{};
        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrier.pNext = nullptr;
        imageBarrier.image = image;
#ifdef _DEBUG
        assert(imageBarrier.image != VK_NULL_HANDLE && "transfering a null image?");
#endif
        imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // or VK_IMAGE_ASPECT_DEPTH_BIT for depth images
        imageBarrier.subresourceRange.baseMipLevel = 0;
        imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        if ((srcStage & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT) 
            && ((dstStage & (VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)))
            ) {
#if BARRIER_DEBUGGING
            printf(" COMPUTE TO GRAPHICS IMAGE TRANSFER \n");
#endif
            imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT; // Access mask for compute shader writes
            imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for transfer read operation

            vkCmdPipelineBarrier(
                cmdBuf,
                srcStage, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, // pipeline stage
                0, //dependency flags
                0, nullptr, //memory barrier
                0, nullptr, //buffer barrier
                1, &imageBarrier //image barrier
            );
        }
        else if (((srcStage & VK_PIPELINE_STAGE_VERTEX_SHADER_BIT) || (srcStage & VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)) &&
            (dstStage & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT))
        {
#if BARRIER_DEBUGGING
            printf(" GRAPHICS TO COMPUTE IMAGE TRANSFER \n");
#endif
            imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT; // Access mask for transfer read operation
            vkCmdPipelineBarrier(
                cmdBuf,
                srcStage, dstStage, // pipeline stage
                0, //dependency flags
                0, nullptr, //memory barrier
                0, nullptr, //buffer barrier
                1, &imageBarrier //image barrier
            );
        }
        else if (srcStage == dstStage && (srcStage & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)) {
#if BARRIER_DEBUGGING
            printf("COMPUTE TO COMPUTE image barrier \n");
#endif
            imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for compute shader writes
            imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT; // Access mask for transfer read operation
            vkCmdPipelineBarrier(
                cmdBuf,
                srcStage, dstStage, // pipeline stage
                0, //dependency flags
                0, nullptr, //memory barrier
                0, nullptr, //buffer barrier
                1, &imageBarrier //image barrier
            );
        }

    }
    void EWEDevice::TransferImageStage(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, std::vector<VkImage> const& images) {
        assert(images.size() > 0);
        const uint32_t imageCount = static_cast<uint32_t>(images.size());

        std::vector<VkImageMemoryBarrier> imageBarriers{};
        imageBarriers.resize(imageCount);
        imageBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarriers[0].pNext = nullptr;
        imageBarriers[0].image = images[0];
#ifdef _DEBUG
        assert(imageBarriers[0].image != VK_NULL_HANDLE && "transfering a null image?");
#endif

        imageBarriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // or VK_IMAGE_ASPECT_DEPTH_BIT for depth images
        imageBarriers[0].subresourceRange.baseMipLevel = 0;
        imageBarriers[0].subresourceRange.levelCount = 1;
        imageBarriers[0].subresourceRange.baseArrayLayer = 0;
        imageBarriers[0].subresourceRange.layerCount = 1;
        if (queueData.index[QueueData::q_compute] != queueData.index[QueueData::q_graphics]) {
            imageBarriers[0].srcQueueFamilyIndex = queueData.index[QueueData::q_compute];
            imageBarriers[0].dstQueueFamilyIndex = queueData.index[QueueData::q_graphics];
        }
        else {
            imageBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        }

        if ((srcStage & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT) &&
            ((dstStage & VK_PIPELINE_STAGE_VERTEX_SHADER_BIT) || (dstStage & VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT))
        ) {

            std::cout << " COMPUTE TO GRAPHICS IMAGE TRANSFER \n";
            imageBarriers[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageBarriers[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageBarriers[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for compute shader writes
            imageBarriers[0].dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for transfer read operation
        }
        else if (((srcStage & VK_PIPELINE_STAGE_VERTEX_SHADER_BIT) || (srcStage & VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)) &&
            (dstStage & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT))
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
#ifdef _DEBUG
            assert(imageBarriers[i].image != VK_NULL_HANDLE && "transfering a null image?");
#endif
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


    void EWEDevice::TransitionFromTransfer(VkCommandBuffer cmdBuf, QueueData::Queue_Enum dstQueueIndex, VkImage const& image, VkImageLayout finalLayout) {
        VkImageMemoryBarrier imageBarrier{};
        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrier.pNext = nullptr;
        imageBarrier.image = image;
        assert(imageBarrier.image != VK_NULL_HANDLE && "transfering a null image?");
        imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // or VK_IMAGE_ASPECT_DEPTH_BIT for depth images
        imageBarrier.subresourceRange.baseMipLevel = 0;
        imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        imageBarrier.srcQueueFamilyIndex = queueData.index[QueueData::q_transfer];

        imageBarrier.dstQueueFamilyIndex = queueData.index[dstQueueIndex];

        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrier.newLayout = finalLayout;
        assert((dstQueueIndex == QueueData::q_graphics) || (dstQueueIndex == QueueData::q_compute));

        imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Access mask for compute shader writes
        imageBarrier.dstAccessMask = 0; // Access mask for transfer read operation
        vkCmdPipelineBarrier(
            cmdBuf,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, // pipeline stage
            0, //dependency flags
            0, nullptr, //memory barrier
            0, nullptr, //buffer barrier
            1, &imageBarrier //image barrier
        );
    }
    void EWEDevice::TransitionFromTransferToGraphics(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImage const& image) {
        VkImageMemoryBarrier imageBarrier{};
        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrier.pNext = nullptr;
        imageBarrier.image = image;
        assert(imageBarrier.image != VK_NULL_HANDLE && "transfering a null image?");
        imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // or VK_IMAGE_ASPECT_DEPTH_BIT for depth images
        imageBarrier.subresourceRange.baseMipLevel = 0;
        imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        imageBarrier.srcQueueFamilyIndex = queueData.index[QueueData::q_transfer];

        imageBarrier.dstQueueFamilyIndex = queueData.index[QueueData::q_graphics];

        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        imageBarrier.srcAccessMask = 0; // Access mask for compute shader writes
        imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for transfer read operation
        vkCmdPipelineBarrier(
            cmdBuf,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // pipeline stage
            0, //dependency flags
            0, nullptr, //memory barrier
            0, nullptr, //buffer barrier
            1, &imageBarrier //image barrier
        );
    }

    void EWEDevice::SetImageLayout(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange)
    {
        // Create an image barrier object
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
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
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        }
        else if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
            imageMemoryBarrier.srcQueueFamilyIndex = queueData.q_transfer;
            imageMemoryBarrier.dstQueueFamilyIndex = queueData.q_graphics;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
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

    void EWEDevice::CopyBufferToImage(VkCommandBuffer cmdBuf, VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height, uint32_t layerCount) {

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
            cmdBuf,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);
    }

    void EWEDevice::CreateImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        EWE_VK_ASSERT(vkCreateImage(device_, &imageInfo, nullptr, &image));


        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device_, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        EWE_VK_ASSERT(vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory));

        EWE_VK_ASSERT(vkBindImageMemory(device_, image, imageMemory, 0));
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
    QueueTransitionContainer* EWEDevice::PostTransitionsToGraphics(VkCommandBuffer cmdBuf, uint8_t frameIndex){
		QueueTransitionContainer* transitionContainer = syncHub->transitionManager.PrepareGraphics(frameIndex);
        if(transitionContainer == nullptr){
            return nullptr;
        }

        VkImageLayout dstLayout;
        std::vector<VkImageMemoryBarrier> imageBarriers{};
        std::vector<VkBufferMemoryBarrier> bufferBarriers{};
        imageBarriers.reserve(transitionContainer->images.size());
        bufferBarriers.reserve(transitionContainer->buffers.size());

        for(auto& transitionInstance : transitionContainer->images){
            if(transitionInstance.mipLevels > 1){
                dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            }
            else{
                dstLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            imageBarriers.push_back(
                TransitionImageLayout(transitionInstance.image, 
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstLayout,
                    transitionInstance.mipLevels,
                    transitionInstance.arrayLayers
                )
            );
        }
         
        vkCmdPipelineBarrier(cmdBuf,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            0, nullptr,
            bufferBarriers.size(), bufferBarriers.data(),
            imageBarriers.size(), imageBarriers.data()
        );
        return transitionContainer;
    }
}  // namespace EWE