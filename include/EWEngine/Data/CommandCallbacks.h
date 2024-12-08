#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"
#include "EWEngine/Graphics/PipelineBarrier.h"
#include "EWEngine/Graphics/Texture/Image.h"

#include <vector>

namespace EWE {


    struct Semaphore {
        VkSemaphore vkSemaphore{ VK_NULL_HANDLE };
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
        std::vector<ImageInfo*> images;
        Semaphore* semaphore;

        TransferCommandCallbacks() : commands{}, stagingBuffers{}, pipeBarriers{}, images{}, semaphore{ nullptr } {} //constructor
        TransferCommandCallbacks(TransferCommandCallbacks& copySource); //copy constructor
        TransferCommandCallbacks& operator=(TransferCommandCallbacks& copySource); //copy assignment
        TransferCommandCallbacks(TransferCommandCallbacks&& moveSource) noexcept;//move constructor
        TransferCommandCallbacks& operator=(TransferCommandCallbacks&& moveSource) noexcept; //move assignment
        //TransferCommandCallbacks& operator+=(TransferCommandCallbacks& copySource);
    };
} //namespace EWE