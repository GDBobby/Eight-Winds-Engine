#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"
#include "EWEngine/Graphics/PipelineBarrier.h"

#include <vector>

namespace EWE {


    struct CommandBufferData {
        bool inUse;
        VkCommandBuffer cmdBuf;

        CommandBufferData() : inUse{ false }, cmdBuf{ VK_NULL_HANDLE } {}
        void Reset();
        void BeginSingleTime();
        //void Begin();
    };

    struct SemaphoreData {
        VkSemaphore semaphore{ VK_NULL_HANDLE };
#if SEMAPHORE_TRACKING
        std::string name{ "null" };
        VkDevice device;
#endif
        bool waiting{ false };
        bool signaling{ false };

        bool Idle() const {
            return !(waiting || signaling);
        }
        void FinishSignaling();
        void FinishWaiting();
        void BeginWaiting();
#if SEMAPHORE_TRACKING
        void BeginSignaling(const char* name);
#else
        void BeginSignaling();
#endif
    };

    struct CommandCallbacks {
        std::vector<CommandBufferData*> commands;
        std::vector<StagingBuffer*> stagingBuffers;
        std::vector<PipelineBarrier> pipeBarriers;
        std::vector<MipParamPack> mipParamPacks;
        SemaphoreData* semaphoreData;

        CommandCallbacks() : commands{}, stagingBuffers{}, pipeBarriers{}, mipParamPacks{}, semaphoreData{ nullptr } {} //constructor
        CommandCallbacks(CommandCallbacks& copySource); //copy constructor
        CommandCallbacks& operator=(CommandCallbacks& copySource); //copy assignment
        CommandCallbacks(CommandCallbacks&& moveSource) noexcept;//move constructor
        CommandCallbacks& operator=(CommandCallbacks&& moveSource) noexcept; //move assignment
    };
    /*
    struct CallbacksForGraphics {
        std::vector<PipelineBarrier> pipeBarriers;
        std::vector<MipParamPack> mipParamPacks;
        CallbacksForGraphics() : pipeBarriers{}, mipParamPacks{} {}
        CallbacksForGraphics(CallbacksForGraphics& copySource); //copy constructor
        CallbacksForGraphics& operator=(CallbacksForGraphics const& copySource); //copy assignment
        CallbacksForGraphics(CallbacksForGraphics&& moveSource) noexcept = delete;//move constructor
        CallbacksForGraphics& operator=(CallbacksForGraphics&& moveSource) noexcept; //move assignment


        CallbacksForGraphics(CommandCallbacks& copySource); //copy constructor
        CallbacksForGraphics& operator=(CommandCallbacks const& copySource); //copy assignment
        CallbacksForGraphics(CommandCallbacks&& moveSource) noexcept = delete;//move constructor
        CallbacksForGraphics& operator=(CommandCallbacks&& moveSource) noexcept; //move assignment
    };
    */
} //namespace EWE