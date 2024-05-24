#pragma once

#define GPU_LOGGING true


#include "EWEngine/Graphics/VkDebugDeviceLost.h"


#if GPU_LOGGING
#include <fstream>
#define GPU_LOG_FILE "GPULog.log"

#ifndef EWE_VK_RESULT_ASSERT
    #define EWE_VK_RESULT_ASSERT(result) \
        if(result == VK_ERROR_DEVICE_LOST){ EWE::VKDEBUG::OnDeviceLost();} \
        else if (result != VK_SUCCESS) {\
            printf("VK_ERROR : %s(%d) : %s - %d \n", __FILE__, __LINE__, __FUNCTION__, result); \
            std::ofstream logFile{}; \
            logFile.open(GPU_LOG_FILE, std::ios::app); \
            assert(logFile.is_open() && "Failed to open log file"); \
            logFile << "VK_ERROR : " << __FILE__ << '(' << __LINE__ << ") : " << __FUNCTION__ << " - " << result << '\n'; \
            logFile.close(); \
            assert(result == VK_SUCCESS && "VK_ERROR"); \
	    }
    #endif

    #ifndef EWE_VK_ASSERT
    #define EWE_VK_ASSERT(vkFunc) \
        {VkResult result = (vkFunc);\
        EWE_VK_RESULT_ASSERT(result)}
    #endif
#else
    #ifndef EWE_VK_RESULT_ASSERT
    #define EWE_VK_RESULT_ASSERT(result) \
        if (result != VK_SUCCESS) {\
            printf("VK_ERROR : %s(%d) : %s - %l\n", __FILE__, __LINE__, __FUNCTION__, result); \
            assert(result == VK_SUCCESS && "VK_ERROR"); \
	    }
    #endif

    #ifndef EWE_VK_ASSERT
    #define EWE_VK_ASSERT(vkFunc) \
    {\
        VkResult result = (vkFunc);\
        EWE_VK_RESULT_ASSERT(result)}
    #endif
#endif