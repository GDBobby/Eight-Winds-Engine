#pragma once

#include "EWEngine/Data/EngineDataTypes.h"
#include "EWEngine/Graphics/Preprocessor.h"
#include "EWEngine/Graphics/VkDebugDeviceLost.h"

#include <functional>

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

    struct SemaphoreData {
        VkSemaphore semaphore{ VK_NULL_HANDLE };
        bool waiting{ false };
        bool signaling{ false };

        bool Idle() const {
            return !(waiting || signaling);
        }
        void FinishSignaling() {
#ifdef _DEBUG
            assert(signaling == true && "finishing a signal that wasn't signaled");
#endif
            signaling = false;
        }
        void FinishWaiting() {
#ifdef _DEBUG
            assert(waiting == true && "finished waiting when not waiting");
#endif
            waiting = false;
        }
        void BeginWaiting() {
#ifdef _DEBUG
            assert(waiting == false && "attempting to begin wait while waiting");
#endif
            waiting = true;
        }
        void BeginSignaling() {
#ifdef _DEBUG
            assert(signaling == false && "attempting to signal while signaled");
#endif
            signaling = true;
        }
    };

    struct FenceData {
        VkFence fence{ VK_NULL_HANDLE };
        std::function<void()> callback{ nullptr }; //i think this is a function pointer, if I can't set it to null, I need to make it a pointer
        bool inUse{ false };
        std::vector<SemaphoreData*> waitSemaphores{}; //each wait could potentially be signaled multiple times in a single queue, and then multiple queues
        SemaphoreData* signalSemaphores[Queue::_count] = { nullptr, nullptr, nullptr, nullptr }; //each signal is unique per submit that could wait on it, and right now I'm expecting max 1 wait per queue

        void Reset(VkDevice device);
    };

} //namespace EWE



#if GPU_LOGGING
#include <fstream>
#define GPU_LOG_FILE "GPULog.log"

#ifndef EWE_VK_RESULT_ASSERT
#if DEBUGGING_DEVICE_LOST
#define EWE_VK_RESULT_ASSERT(result)                                                                                            \
                    if(result == VK_ERROR_DEVICE_LOST){ EWE::VKDEBUG::OnDeviceLost();}                                                  \
                    else if (result != VK_SUCCESS) {                                                                                    \
                        printf("VK_ERROR : %s(%d) : %s - %d \n", __FILE__, __LINE__, __FUNCTION__, result);                             \
                        std::ofstream logFile{};                                                                                        \
                        logFile.open(GPU_LOG_FILE, std::ios::app);                                                                      \
                        assert(logFile.is_open() && "Failed to open log file");                                                         \
                        logFile << "VK_ERROR : " << __FILE__ << '(' << __LINE__ << ") : " << __FUNCTION__ << " - " << result << '\n';   \
                        logFile.close();                                                                                                \
                        assert(result == VK_SUCCESS && "VK_ERROR");                                                                     \
	                }
#else
#define EWE_VK_RESULT_ASSERT(result)                                                                                            \
                    if (result != VK_SUCCESS) {                                                                                         \
                        printf("VK_ERROR : %s(%d) : %s - %d \n", __FILE__, __LINE__, __FUNCTION__, result);                             \
                        std::ofstream logFile{};                                                                                        \
                        logFile.open(GPU_LOG_FILE, std::ios::app);                                                                      \
                        assert(logFile.is_open() && "Failed to open log file");                                                         \
                        logFile << "VK_ERROR : " << __FILE__ << '(' << __LINE__ << ") : " << __FUNCTION__ << " - " << result << '\n';   \
                        logFile.close();                                                                                                \
                        assert(result == VK_SUCCESS && "VK_ERROR");                                                                     \
	                }
#endif
#endif

#ifndef EWE_VK_ASSERT
#define EWE_VK_ASSERT(vkFunc)       \
        {VkResult result = (vkFunc);    \
        EWE_VK_RESULT_ASSERT(result)}
#endif
#else
#ifndef EWE_VK_RESULT_ASSERT
#define EWE_VK_RESULT_ASSERT(result)                                                        \
        if (result != VK_SUCCESS) {                                                             \
            printf("VK_ERROR : %s(%d) : %s - %l\n", __FILE__, __LINE__, __FUNCTION__, result);  \
            assert(result == VK_SUCCESS && "VK_ERROR");                                         \
	    }
#endif

#ifndef EWE_VK_ASSERT
#define EWE_VK_ASSERT(vkFunc)       \
    {VkResult result = (vkFunc);        \
        EWE_VK_RESULT_ASSERT(result)}
#endif
#endif