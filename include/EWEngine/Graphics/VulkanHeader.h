#pragma once

#include "EWEngine/Graphics/Preprocessor.h"
#if USING_VMA
#include "EWEngine/Graphics/vk_mem_alloc.h"
#endif
#include "EWEngine/Data/EngineDataTypes.h"
#if DEBUGGING_DEVICE_LOST
#include "EWEngine/Graphics/VkDebugDeviceLost.h"
#endif
#if DEBUG_NAMING
#include "EWEngine/Graphics/DebugNaming.h"
#include <string>
#endif

#include <mutex>

#include <functional>

#include <type_traits>
#include <concepts>
#include <array>
#include <tuple>
#include <utility>

namespace EWE{
    static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;


    namespace Queue {
        enum Enum : uint32_t {
            graphics,
            present,
            compute,
            transfer,
            _count,
        };
    } //namespace Queue

    uint32_t FindMemoryType(uint32_t typeFilter, const VkMemoryPropertyFlags properties);
#if COMMAND_BUFFER_TRACING
    struct CommandBuffer {
        VkCommandBuffer cmdBuf;
        bool inUse;
        struct Tracking {
            std::string funcName;
            std::source_location srcLoc;
            Tracking(std::string const& funcName, std::source_location const& srcLoc) : funcName{ funcName }, srcLoc{ srcLoc } {}
        };
        std::vector<Tracking> usageTracking;

        CommandBuffer() : cmdBuf{ VK_NULL_HANDLE }, inUse{ false }, usageTracking{} {}

        //dont use these, but i need it for the vector. reenable to spot check and ensure none happen periodically
        //CommandBuffer(CommandBuffer const& other) = delete;
        //CommandBuffer(CommandBuffer&& other) = delete;
        //CommandBuffer& operator=(CommandBuffer const& other) = delete;
        //CommandBuffer& operator=(CommandBuffer&& other) = delete;


        void Reset();
        void BeginSingleTime();

        //void Begin();
    };
#else
    typedef CommandBuffer = VkCommandBuffer;
#endif
    namespace Sampler { //defined in Sampler.cpp, testing a split cpp/header file
        VkSampler GetSampler(VkSamplerCreateInfo const& samplerInfo);
        void RemoveSampler(VkSampler sampler);
        void Initialize();
        void Deconstruct();
    } //namespace Sampler

    struct VK {
        static VK* Object;

        VK() {
            assert(Object == nullptr);
            Object = this;
        }
        VK(VK& copySource) = delete;
        VK(VK&& moveSource) = delete;
        VK& operator=(VK const& copySource) = delete;
        VK& operator=(VK&& moveSource) = delete;

        VkDevice vkDevice;
        VkPhysicalDevice physicalDevice;
        VkInstance instance;
        std::array<std::mutex, Queue::_count> poolMutex{};
        std::mutex STGMutex{};
        std::array<VkCommandPool, Queue::_count> commandPools = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
        VkCommandPool STGCmdPool{ VK_NULL_HANDLE }; //separate graphics pool for single time commands
        std::array<VkQueue, Queue::_count> queues;
        std::array<int, Queue::_count> queueIndex;
        VkSurfaceKHR surface;
        VkPhysicalDeviceProperties properties;

        uint8_t frameIndex{0};

        std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> renderCommands{};

        VkCommandBuffer& GetVKCommandBufferDirect() {
#if COMMAND_BUFFER_TRACING
            return renderCommands[frameIndex].cmdBuf;
#else
            renderCommands[frameIndex];
#endif
        }

        CommandBuffer& GetFrameBuffer() {

            return renderCommands[frameIndex];
        }
#if USING_VMA
        VmaAllocator vmaAllocator;
#endif
    };



    struct StagingBuffer {
        VkBuffer buffer{ VK_NULL_HANDLE };
#if USING_VMA
        VmaAllocation vmaAlloc{};
        StagingBuffer(VkDeviceSize size, VmaAllocator vmaAllocator);
        StagingBuffer(VkDeviceSize size, VmaAllocator vmaAllocator, const void* data);
        void Free(VmaAllocator vmaAllocator);
        void Free(VmaAllocator vmaAllocator) const;
        void Stage(VmaAllocator vmaAllocator, const void* data, uint64_t bufferSize);
#else
        VkDeviceMemory memory{ VK_NULL_HANDLE };
        StagingBuffer(VkDeviceSize size);
        StagingBuffer(VkDeviceSize size, const void* data);
        void Free();
        void Free() const;
        void Stage(const void* data, VkDeviceSize bufferSize);
#endif
    };



} //namespace EWE



#if GPU_LOGGING
#include <fstream>
#define GPU_LOG_FILE "GPULog.log"
#endif

#define WRAPPING_VULKAN_FUNCTIONS false

#if CALL_TRACING
void EWE_VK_RESULT(VkResult vkResult, const std::source_location& sourceLocation = std::source_location::current());

#if COMMAND_BUFFER_TRACING
namespace Recasting {

    template<typename Arg>
    auto ArgumentCasting(std::string const& funcName, std::source_location const& sourceLocation, Arg&& arg) {

        static_assert(!std::is_same_v<Arg, VkCommandBuffer>);

        if constexpr (requires{arg.usageTracking.emplace_back(funcName, sourceLocation); }) {
            arg.usageTracking.emplace_back(funcName, sourceLocation);
            return std::forward<VkCommandBuffer>(arg.cmdBuf);
        }
        else if constexpr (requires{arg->usageTracking.emplace_back(funcName, sourceLocation); }) {
            arg->usageTracking.emplace_back(funcName, sourceLocation);
            return std::forward<VkCommandBuffer*>(&arg->cmdBuf);
        }
        else {
            static_assert(!std::is_same_v<std::decay_t<Arg>, EWE::CommandBuffer>);
            return std::forward<Arg>(arg);
        }
    }

    template<typename... Args>
    auto ReinterpretArguments(std::string const& funcName, std::source_location const& sourceLocation, Args&&... args) {
        return std::make_tuple(ArgumentCasting(funcName, sourceLocation, std::forward<Args>(args))...);
    }
    template<size_t N>
    struct Apply {
        template<typename F, typename T, typename... A>
        static inline auto apply(F&& f, T&& t, A &&... a) {
            return Apply<N - 1>::apply(std::forward<F>(f), std::forward<T>(t),
                std::get<N - 1>(std::forward<T>(t)), std::forward<A>(a)...
            );
        }
    };

    template<>
    struct Apply<0> {
        template<typename F, typename T, typename... A>
        static inline auto apply(F&& f, T&&, A &&... a) requires std::invocable<F, A...> {
            return std::forward<F>(f)(std::forward<A>(a)...);
        }
    };

    template<typename F, typename T>
    inline auto ApplyFunc(F&& f, T&& t) {
        return Apply<std::tuple_size<std::decay_t<T>>::value>::apply(std::forward<F>(f), std::forward<T>(t));
    }

    template<typename F, typename... Args>
    void CallWithReinterpretedArguments(std::source_location srcLoc, F&& func, std::tuple<Args...>&& tuple) {

        if constexpr (std::is_void_v<decltype(ApplyFunc(func, tuple))>) {
            //std::invoke(std::forward<F>(func), std::get<Args>(tuple)...);
            ApplyFunc(func, tuple);
        }
        else {
            //VkResult vkResult = std::invoke(std::forward<F>(func), std::get<Args>(tuple)...);
            VkResult vkResult = ApplyFunc(func, tuple);
            EWE_VK_RESULT(vkResult, srcLoc);
        }
    }
}
#endif


//if having difficulty with template errors related to this function, define the vulkan function by itself before using this function to ensure its correct
template<typename F, typename... Args>
struct EWE_VK {
    EWE_VK(F&& func, Args&&... args, std::source_location const& sourceLocation = std::source_location::current()) {
#if WRAPPING_VULKAN_FUNCTIONS
        //call a preliminary function
#endif
        if constexpr (std::is_same_v<std::decay_t<F>, PFN_vkCmdBindDescriptorSets>) {
            const auto descriptorSetCount = std::get<4>(std::forward_as_tuple(args...));
            const auto pDescriptorSets = std::get<5>(std::forward_as_tuple(args...));
            for (uint32_t i = 0; i < descriptorSetCount; i++) {
                assert((pDescriptorSets[i] != VK_NULL_HANDLE) && (reinterpret_cast<std::size_t>(pDescriptorSets[i]) != 0xCDCDCDCDCDCDCDCD));
            }
        }


#if COMMAND_BUFFER_TRACING
        const std::string funcName = typeid(func).name();
        auto reinterpretedArgs = Recasting::ReinterpretArguments(funcName, sourceLocation, std::forward<Args>(args)...);
        Recasting::CallWithReinterpretedArguments(sourceLocation, func, std::move(reinterpretedArgs));
#else
        if constexpr (std::is_void_v<decltype(std::forward<F>(f)(std::forward<Args>(args)...))>) {
            //std::bind(std::forward<F>(f), std::forward<Args>(args)...)(); //std bind is constexpr, might be worth using
            std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        }
        else {
            VkResult vkResult = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            EWE_VK_RESULT(vkResult, sourceLocation);

        }
#endif
#if WRAPPING_VULKAN_FUNCTIONS
        //call a following function
#endif
    }
};
template<typename F, typename... Args>
EWE_VK(F&& f, Args&&...) -> EWE_VK<F, Args...>;

#else
//if having difficulty with template errors related to this function, define the vulkan function by itself before using this function to ensure its correct
void EWE_VK_RESULT(VkResult vkResult) {
#if DEBUGGING_DEVICE_LOST                                                                                        
    if (vkResult == VK_ERROR_DEVICE_LOST) { EWE::VKDEBUG::OnDeviceLost(); }
    else
#else
    if (vkResult != VK_SUCCESS) {
        printf("VK_ERROR : %d \n", vkResult);
        std::ofstream logFile{};
        logFile.open(GPU_LOG_FILE, std::ios::app);
        assert(logFile.is_open() && "Failed to open log file");
        logFile << "VK_ERROR : VkResult(" << vkResult << ")\n";
        logFile.close();
        assert(vkResult == VK_SUCCESS && "VK_ERROR");
    }
#endif
}

template<typename F, typename... Args>
void EWE_VK(F&& f, Args&&... args) {
#if WRAPPING_VULKAN_FUNCTIONS
    //call a preliminary function
#endif
    if constexpr (std::is_same_v<std::invoke_result<F(Args...)>, void>) {
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        //f(args);
    }
    else {
        //VkResult vkResult = std::forward<F>(f)(std::forward<Args>(args)...);
        VkResult vkResult = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);

        if (vkResult != VK_SUCCESS) {
#if DEBUGGING_DEVICE_LOST                                                                                        
            if (vkResult == VK_ERROR_DEVICE_LOST) { EWE::VKDEBUG::OnDeviceLost(); }
            else
#else
            if (vkResult != VK_SUCCESS) {
                printf("VK_ERROR : %d \n", vkResult);
                std::ofstream logFile{};
                logFile.open(GPU_LOG_FILE, std::ios::app);
                assert(logFile.is_open() && "Failed to open log file");
                logFile << "VK_ERROR : VkResult(" << vkResult << ")\n";
                logFile.close();
                assert(vkResult == VK_SUCCESS && "VK_ERROR");
            }
#endif
        }
    }
#if WRAPPING_VULKAN_FUNCTIONS
    //call a following function
#endif
    }
#endif