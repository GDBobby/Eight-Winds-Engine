#include "EWEngine/Data/CommandCallbacks.h"

namespace EWE {

    void CommandBufferData::Reset() {
        //VkCommandBufferResetFlags flags = VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;
        VkCommandBufferResetFlags flags = 0;
        EWE_VK(vkResetCommandBuffer, cmdBuf, flags);
        inUse = false;
    }
    void CommandBufferData::BeginSingleTime() {
        inUse = true;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        EWE_VK(vkBeginCommandBuffer, cmdBuf, &beginInfo);
    }


#if SEMAPHORE_TRACKING
    void SemaphoreData::FinishSignaling() {
        assert(signaling && "finishing a signal that wasn't signaled");
        if (!waiting) {
            name = "null";
            DebugNaming::SetObjectName(device, semaphore, VK_OBJECT_TYPE_SEMAPHORE, name.c_str());
        }
        signaling = false;
    }
    void SemaphoreData::FinishWaiting() {
        assert(waiting && "finished waiting when not waiting");
        waiting = false;
        name = "null";
        DebugNaming::SetObjectName(device, semaphore, VK_OBJECT_TYPE_SEMAPHORE, name.c_str());
    }
    void SemaphoreData::BeginWaiting() {
        assert(name != "null" && "semaphore wasn't named");
        assert(!waiting && "attempting to begin wait while waiting");
        waiting = true;
    }
    void SemaphoreData::BeginSignaling(const char* name) {

        assert(this->name == "null" && "name wasn't reset properly");
        this->name = name;
        DebugNaming::SetObjectName(device, semaphore, VK_OBJECT_TYPE_SEMAPHORE, name);
        assert(!signaling && "attempting to signal while signaled");
        signaling = true;
    }
#else     
    void SemaphoreData::FinishSignaling() {
#if EWE_DEBUG
        assert(signaling == true && "finishing a signal that wasn't signaled");
#endif
        signaling = false;
    }
    void SemaphoreData::FinishWaiting() {
#if EWE_DEBUG
        assert(waiting == true && "finished waiting when not waiting");
#endif
        waiting = false;
    }
    void SemaphoreData::BeginWaiting() {
#if EWE_DEBUG
        assert(waiting == false && "attempting to begin wait while waiting");
#endif
        waiting = true;
    }
    void SemaphoreData::BeginSignaling() {
#if EWE_DEBUG
        assert(signaling == false && "attempting to signal while signaled");
#endif
        signaling = true;
    }
#endif




    CommandCallbacks::CommandCallbacks(CommandCallbacks& copySource) : //copy constructor
        commands{ std::move(copySource.commands) },
        stagingBuffers{ std::move(copySource.stagingBuffers) },
        pipeBarriers{ std::move(copySource.pipeBarriers) },
        mipParamPacks{ std::move(copySource.mipParamPacks) },
        semaphoreData{ copySource.semaphoreData }
    {
        copySource.semaphoreData = nullptr;
    }
    CommandCallbacks& CommandCallbacks::operator=(CommandCallbacks& copySource) { //copy assignment
        commands = std::move(copySource.commands);
        stagingBuffers = std::move(copySource.stagingBuffers);
        pipeBarriers = std::move(copySource.pipeBarriers);
        mipParamPacks = std::move(copySource.mipParamPacks);
        semaphoreData = copySource.semaphoreData;
        copySource.semaphoreData = nullptr;
        return *this;
    }

    CommandCallbacks::CommandCallbacks(CommandCallbacks&& moveSource) noexcept ://move constructor
        commands{ std::move(moveSource.commands) },
        stagingBuffers{ std::move(moveSource.stagingBuffers) },
        pipeBarriers{ std::move(moveSource.pipeBarriers) },
        mipParamPacks{ std::move(moveSource.mipParamPacks) },
        semaphoreData{ moveSource.semaphoreData }

    {
        moveSource.semaphoreData = nullptr;
    }

    CommandCallbacks& CommandCallbacks::operator=(CommandCallbacks&& moveSource) noexcept { //move assignment
        commands = std::move(moveSource.commands);
        stagingBuffers = std::move(moveSource.stagingBuffers);
        pipeBarriers = std::move(moveSource.pipeBarriers);
        mipParamPacks = std::move(moveSource.mipParamPacks);
        semaphoreData = moveSource.semaphoreData;
        moveSource.semaphoreData = nullptr;

        return *this;
    }

    /*
    CallbacksForGraphics::CallbacksForGraphics(CallbacksForGraphics& copySource) : //copy constructor
        pipeBarriers{ std::move(copySource.pipeBarriers) },
        mipParamPacks{ std::move(copySource.mipParamPacks) }
    {}
    CallbacksForGraphics& CallbacksForGraphics::operator=(CallbacksForGraphics const& copySource) { //copy assignment
        pipeBarriers = std::move(copySource.pipeBarriers);
        mipParamPacks = std::move(copySource.mipParamPacks);

        return *this;
    }
    CallbacksForGraphics& CallbacksForGraphics::operator=(CallbacksForGraphics&& moveSource) noexcept { //move assignment
        pipeBarriers = std::move(moveSource.pipeBarriers);
        mipParamPacks = std::move(moveSource.mipParamPacks);

        return *this;
    }

    CallbacksForGraphics::CallbacksForGraphics(CommandCallbacks& copySource) : //copy constructor
        pipeBarriers{ std::move(copySource.pipeBarriers) },
        mipParamPacks{ std::move(copySource.mipParamPacks) }
    {
        
    }
    CallbacksForGraphics& CallbacksForGraphics::operator=(CommandCallbacks const& copySource) { //copy assignment
        pipeBarriers = std::move(copySource.pipeBarriers);
        mipParamPacks = std::move(copySource.mipParamPacks);

        return *this;
    }
    CallbacksForGraphics& CallbacksForGraphics::operator=(CommandCallbacks&& moveSource) noexcept { //move assignment
        pipeBarriers = std::move(moveSource.pipeBarriers);
        mipParamPacks = std::move(moveSource.mipParamPacks);

        return *this;
    }
	*/
}//namespace EWE