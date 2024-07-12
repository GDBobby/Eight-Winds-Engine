#include "EWEngine/Graphics/VulkanHeader.h"

namespace EWE {
    void FenceData::Reset(VkDevice device) {
        EWE_VK_ASSERT(vkResetFences(device, 1, &fence));
        for (auto& waitSem : waitSemaphores) {
            waitSem->FinishWaiting();
        }
        waitSemaphores.clear();
        for (auto& sigSem : signalSemaphores) {
            sigSem->FinishSignaling();
            sigSem = nullptr;
        }

        inUse = false;
    }
}