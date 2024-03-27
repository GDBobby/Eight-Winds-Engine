#pragma once

#define GPU_LOGGING true


#if GPU_LOGGING
#include <fstream>
#define GPU_LOG_FILE "GPULog.log"
#define EWE_VK_ASSERT(vkFunc) \
{\
    VkResult result = (vkFunc);\
    if (result != VK_SUCCESS) {\
        printf("VK_ERROR : %s(%s) : %s - %l\n", __FILE__, __LINE__, __FUNCTION__, result); \
        std::ofstream logFile{}; \
        logFile.open(GPU_LOG_FILE, std::ios::app); \
        assert(logFile.is_open() && "Failed to open log file"); \
        logFile << "VK_ERROR : " << __FILE__ << '(' << __LINE__ << ") : " << __FUNCTION__ << " - " << result << '\n'; \
        assert(result == VK_SUCCESS && "VK_ERROR"); \
	}}
#else
#define EWE_VK_ASSERT(vkFunc) \
{\
    VkResult result = (vkFunc);\
    if (result != VK_SUCCESS) {\
        printf("VK_ERROR : %s(%s) : %s - %l\n", __FILE__, __LINE__, __FUNCTION__, result); \
        assert(result == VK_SUCCESS && "VK_ERROR"); \
	}\
}
#endif