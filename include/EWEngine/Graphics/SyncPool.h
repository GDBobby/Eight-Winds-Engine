#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"
#include "EWEngine/Graphics/PipelineBarrier.h"
#include "EWEngine/Data/CommandCallbacks.h"

#include <cassert>
#include <thread>
#include <mutex>
#include <array>
#include <vector>


/*
* std::this_thread::sleep_for(std::chrono::microseconds(1));
    this is a small amount of time, 
    and most likely 
    it'll allow the OS to take over the thread and do other operations on it, 
    then return control of this thead to our program.
    It's not 100% for the OS to take over, but if it does, the thread will potentially sleep for longer (up to multiple milliseconds)

    it is possible to spin the thread until a certain amount of time has passed
    BUT if immediate control of a new fence/semaphore is required,
    it's recommended throw an error if the requested data is not available, and increase pool size as needed
*/

namespace EWE{
    struct FenceData {
        VkFence fence{ VK_NULL_HANDLE };
        bool inUse{ false };
        std::mutex mut{};
        std::vector<SemaphoreData*> waitSemaphores{};

        //its up to the calling function to unlock the mutex
        bool CheckReturn(uint64_t time);
    };

    struct GraphicsFenceData {
        FenceData fenceData{};
        std::vector<CommandBuffer*> commands{};
        std::vector<VkImageLayout*> imageLayouts{};

        void CheckReturn(std::vector<CommandBuffer*>& output, uint64_t time);
    };

    struct TransferFenceData {
        FenceData fenceData{};
        SemaphoreData* signalSemaphoreForGraphics{ nullptr };
        TransferCommandCallbacks callbacks{};

        void WaitReturnCallbacks(std::vector<TransferCommandCallbacks>& output, uint64_t time);
    };

    //not thread safe

    //each semaphore and fence could track which queue it's being used in, but I don't think that's the best approach

    struct WaitData {
        std::vector<SemaphoreData*> semaphores{};
        std::vector<VkPipelineStageFlags> waitDstMask{};

        //for submission, the lifetime needs to be controlled here
        //theres probably a better way to control lifetime but i cant think of it right now
        //main concern is repeatedly reallocating memory, which this might avoid depending on how the vector is handled behind the scenes
        //an alternative solution is using an array, with a max size of say 16, and using a counter to keep track of the active count, rather than vectors
        std::vector<VkSemaphore> semaphoreData{}; 
    };


    struct RenderSyncData {
    private:
        std::mutex waitMutex{};
        std::mutex signalMutex{};
        WaitData previousWait[MAX_FRAMES_IN_FLIGHT]{};
        std::vector<SemaphoreData*> previousSignals[MAX_FRAMES_IN_FLIGHT]{};
    public:
        VkFence inFlight[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        VkSemaphore imageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        VkSemaphore renderFinishedSemaphore[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        WaitData waitData{};
        std::vector<SemaphoreData*> signalSemaphores{};

        RenderSyncData();
        ~RenderSyncData();
        void AddWaitSemaphore(SemaphoreData* semaphore, VkPipelineStageFlags waitDstStageMask);
        void AddSignalSemaphore(SemaphoreData* semaphore);
        void SetWaitData(VkSubmitInfo& submitInfo);
        std::vector<VkSemaphore> GetSignalData();
    };


    //this is explicitly for VkFence, Semaphore, and Event (i don't know yet if event will fit here)
    class SyncPool{
    private:
        const uint8_t size;


        std::vector<TransferFenceData> transferFences;
        std::vector<GraphicsFenceData> graphicsFences;
        std::vector<SemaphoreData> semaphores;
#if COMMAND_BUFFER_TRACING
        std::array<std::vector<CommandBuffer>, Queue::_count> cmdBufs;
#else
        struct CommandBufferTracker {
            VkCommandBuffer cmdBuf;
            bool inUse = false;

            void Reset();
            void BeginSingleTime();
        };
        std::array<std::vector<CommandBufferTracker>, Queue::_count> cmdBufs;
#endif

        //acquisition mutexes to ensure multiple threads dont acquire a single object
        std::mutex graphicsFenceAcqMut{};
        std::mutex transferFenceAcqMut{};
        std::mutex semAcqMut{};
        std::mutex cmdBufAcqMut{};


        void HandleTransferCallbacks(std::vector<TransferCommandCallbacks> callbacks);

    public:
        SyncPool(uint8_t size);

        ~SyncPool();
        SemaphoreData& GetSemaphoreData(VkSemaphore semaphore);

#if SEMAPHORE_TRACKING
        void SemaphoreBeginSignaling(VkSemaphore semaphore, std::source_location srcLoc = std::source_location::current()){
            GetSemaphoreData(semaphore).BeginSignaling(srcLoc);
        }
        void SemaphoreBeginWaiting(VkSemaphore semaphore, std::source_location srcLoc = std::source_location::current()) {
            GetSemaphoreData(semaphore).BeginWaiting(srcLoc);
        }
        void SemaphoreFinishedWaiting(VkSemaphore semaphore, std::source_location srcLoc = std::source_location::current()) {
            GetSemaphoreData(semaphore).FinishWaiting(srcLoc);
        }
        void SemaphoreFinishedSignaling(VkSemaphore semaphore, std::source_location srcLoc = std::source_location::current()) {
            GetSemaphoreData(semaphore).FinishSignaling(srcLoc);
        }
#else
        void SemaphoreBeginSignaling(VkSemaphore semaphore) {
            GetSemaphoreData(semaphore).BeginSignaling();
        }
        void SemaphoreBeginWaiting(VkSemaphore semaphore) {
            GetSemaphoreData(semaphore).BeginWaiting();
        }
        void SemaphoreFinishedWaiting(VkSemaphore semaphore) {
            GetSemaphoreData(semaphore).FinishWaiting();
        }
        void SemaphoreFinishedSignaling(VkSemaphore semaphore) {
            GetSemaphoreData(semaphore).FinishSignaling();
        }
#endif
        
        bool CheckFencesForUsage();
        void CheckFencesForCallbacks();
#if SEMAPHORE_TRACKING
        SemaphoreData* GetSemaphoreForSignaling(std::source_location srcLoc = std::source_location::current());
#else
        SemaphoreData* GetSemaphoreForSignaling();
#endif
        //this locks the fence as well
        TransferFenceData& GetTransferFence();
        GraphicsFenceData& GetGraphicsFence();
        CommandBuffer& GetCmdBufSingleTime(Queue::Enum queue);
        void ResetCommandBuffer(CommandBuffer& cmdBuf, Queue::Enum queue);
        void ResetCommandBuffer(CommandBuffer& cmdBuf);
        void ResetCommandBuffers(std::vector<CommandBuffer*>& cmdBufs, Queue::Enum queue);
        void ResetCommandBuffers(std::vector<CommandBuffer*>& cmdBufs);

        static void (*SubmitGraphicsAsync)(CommandBuffer&, std::vector<SemaphoreData*>, std::vector<VkImageLayout*>);
    };
} //namespace EWE