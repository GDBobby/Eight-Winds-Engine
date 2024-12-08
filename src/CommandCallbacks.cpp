#include "EWEngine/Data/CommandCallbacks.h"

namespace EWE {



#if SEMAPHORE_TRACKING
    void Semaphore::FinishSignaling(std::source_location srcLoc) {
        tracking.emplace_back(Tracking::State::FinishSignaling, srcLoc);
        assert(signaling && "finishing a signal that wasn't signaled");
        signaling = false;
    }
    void Semaphore::FinishWaiting(std::source_location srcLoc) {
        assert(waiting && "finished waiting when not waiting");
        waiting = false;
        tracking.emplace_back(Tracking::State::FinishWaiting, srcLoc);
    }
    void Semaphore::BeginWaiting(std::source_location srcLoc) {
        assert(!waiting && "attempting to begin wait while waiting");
        waiting = true;
        tracking.emplace_back(Tracking::State::BeginWaiting, srcLoc);
    }
    void Semaphore::BeginSignaling(std::source_location srcLoc) {

        assert(!signaling && "attempting to signal while signaled");
        signaling = true;
        tracking.emplace_back(Tracking::State::BeginSignaling, srcLoc);
    }
#else     
    void Semaphore::FinishSignaling() {
#if EWE_DEBUG
        assert(signaling == true && "finishing a signal that wasn't signaled");
#endif
        signaling = false;
    }
    void Semaphore::FinishWaiting() {
#if EWE_DEBUG
        assert(waiting == true && "finished waiting when not waiting");
#endif
        waiting = false;
        signaling = false; //im not sure if this is good or not. currently its a bit rough to keep track of the graphics single time signaling
    }
    void Semaphore::BeginWaiting() {
#if EWE_DEBUG
        assert(waiting == false && "attempting to begin wait while waiting");
#endif
        waiting = true;
    }
    void Semaphore::BeginSignaling() {
#if EWE_DEBUG
        assert(signaling == false && "attempting to signal while signaled");
#endif
        signaling = true;
    }
#endif




    TransferCommandCallbacks::TransferCommandCallbacks(TransferCommandCallbacks& copySource) : //copy constructor
        commands{ std::move(copySource.commands) },
        stagingBuffers{ std::move(copySource.stagingBuffers) },
        pipeBarriers{ std::move(copySource.pipeBarriers) },
        images{ std::move(copySource.images) },
        semaphore{ copySource.semaphore }
    {
        printf("TransferCommandCallbacks:: copy constructor\n");

        copySource.semaphore = nullptr;
    }
    TransferCommandCallbacks& TransferCommandCallbacks::operator=(TransferCommandCallbacks& copySource) { //copy assignment
        commands = std::move(copySource.commands);
        stagingBuffers = std::move(copySource.stagingBuffers);
        pipeBarriers = std::move(copySource.pipeBarriers);
        images = std::move(copySource.images);
        semaphore = copySource.semaphore;
        copySource.semaphore = nullptr;
        printf("TransferCommandCallbacks:: copy constructor\n");

        return *this;
    }

    //TransferCommandCallbacks& TransferCommandCallbacks::operator+=(TransferCommandCallbacks& copySource) {
    //    if (copySource.commands.size() > 0) {
    //        commands.insert(commands.end(), copySource.commands.begin(), copySource.commands.end());
    //        copySource.commands.clear();
    //    }
    //    if (copySource.stagingBuffers.size() > 0) {
    //        stagingBuffers.insert(stagingBuffers.end(), copySource.stagingBuffers.begin(), copySource.stagingBuffers.end());
    //        copySource.stagingBuffers.clear();
    //    }
    //    if (copySource.pipeBarriers.size() > 0) {
    //        pipeBarriers.insert(pipeBarriers.end(), copySource.pipeBarriers.begin(), copySource.pipeBarriers.end());
    //        copySource.pipeBarriers.clear();
    //    }
    //    if (copySource.images.size() > 0) {
    //        images.insert(images.end(), copySource.images.begin(), copySource.images.end());
    //        copySource.images.clear();
    //    }

    //    Semaphore = copySource.Semaphore;
    //    copySource.Semaphore = nullptr;
    //}

    TransferCommandCallbacks::TransferCommandCallbacks(TransferCommandCallbacks&& moveSource) noexcept ://move constructor
        commands{ std::move(moveSource.commands) },
        stagingBuffers{ std::move(moveSource.stagingBuffers) },
        pipeBarriers{ std::move(moveSource.pipeBarriers) },
        images{ std::move(moveSource.images) },
        semaphore{ moveSource.semaphore }

    {

        printf("TransferCommandCallbacks:: move constructor\n");

        moveSource.semaphore = nullptr;
    }

    TransferCommandCallbacks& TransferCommandCallbacks::operator=(TransferCommandCallbacks&& moveSource) noexcept { //move assignment
        commands = std::move(moveSource.commands);
        stagingBuffers = std::move(moveSource.stagingBuffers);
        pipeBarriers = std::move(moveSource.pipeBarriers);
        images = std::move(moveSource.images);
        semaphore = moveSource.semaphore;
        moveSource.semaphore = nullptr;

        printf("TransferCommandCallbacks:: move assignment\n");

        return *this;
    }

}//namespace EWE