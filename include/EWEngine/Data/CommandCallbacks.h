#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"
#include "EWEngine/Graphics/PipelineBarrier.h"
#include "EWEngine/Graphics/Texture/Image.h"

#include <vector>

namespace EWE {


    struct SemaphoreData {
        VkSemaphore semaphore{ VK_NULL_HANDLE };
#if SEMAPHORE_TRACKING
        struct Tracking{
            enum State {
                BeginSignaling,
                FinishSignaling,
                BeginWaiting,
                FinishWaiting,
            };
            State state;
            std::source_location srcLocation;
            Tracking(State state, std::source_location srcLocation) : state{ state }, srcLocation{ srcLocation } {}
        };
        std::vector<Tracking> tracking{};
#endif
        bool waiting{ false };
        bool signaling{ false };

        bool Idle() const {
            return !(waiting || signaling);
        }
#if SEMAPHORE_TRACKING

        void BeginSignaling(std::source_location srcLoc = std::source_location::current());
        void FinishSignaling(std::source_location srcLoc = std::source_location::current());
        void FinishWaiting(std::source_location srcLoc = std::source_location::current());
        void BeginWaiting(std::source_location srcLoc = std::source_location::current());
#else
        void BeginSignaling();
        void FinishSignaling();
        void FinishWaiting();
        void BeginWaiting();
#endif
    };

    struct TransferCommandCallbacks {
        std::vector<CommandBuffer*> commands;
        std::vector<StagingBuffer*> stagingBuffers;
        std::vector<PipelineBarrier> pipeBarriers;
        std::vector<ImageInfo> images;
        SemaphoreData* semaphoreData;

        TransferCommandCallbacks() : commands{}, stagingBuffers{}, pipeBarriers{}, images{}, semaphoreData{ nullptr } {} //constructor
        TransferCommandCallbacks(TransferCommandCallbacks& copySource); //copy constructor
        TransferCommandCallbacks& operator=(TransferCommandCallbacks& copySource); //copy assignment
        TransferCommandCallbacks(TransferCommandCallbacks&& moveSource) noexcept;//move constructor
        TransferCommandCallbacks& operator=(TransferCommandCallbacks&& moveSource) noexcept; //move assignment
        //TransferCommandCallbacks& operator+=(TransferCommandCallbacks& copySource);
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