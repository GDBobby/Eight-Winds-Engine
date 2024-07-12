#include "EWEngine/Graphics/VulkanHeader.h"

namespace EWE {
    std::function<void()> FenceData::Reset(VkDevice device) {
        EWE_VK_ASSERT(vkResetFences(device, 1, &fence));
        for (auto& waitSem : waitSemaphores) {
            waitSem->FinishWaiting();
        }
        waitSemaphores.clear();
        for (uint8_t i = 0; i < Queue::_count; i++) {
            if (signalSemaphores[i] != nullptr) {
                signalSemaphores[i]->FinishSignaling();
                signalSemaphores[i] = nullptr;
            }
        }

        inUse = false;
        std::function<void()> ret{ callback };
        callback = nullptr;
        return ret;
    }
}