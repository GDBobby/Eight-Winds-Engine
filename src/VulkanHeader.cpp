#include "EWEngine/Graphics/VulkanHeader.h"

#if USING_VMA
#define VMA_IMPLEMENTATION
#include "EWEngine/Graphics/vk_mem_alloc.h"
#endif

void EWE_VK_RESULT(VkResult vkResult, const std::source_location& sourceLocation) {
#if DEBUGGING_DEVICE_LOST                                                                                        
    if (vkResult == VK_ERROR_DEVICE_LOST) { EWE::VKDEBUG::OnDeviceLost(); }
    else
#endif
    if (vkResult != VK_SUCCESS) {
        printf("VK_ERROR : %s(%d) : %s - %d \n", sourceLocation.file_name(), sourceLocation.line(), sourceLocation.function_name(), vkResult);
        std::ofstream logFile{};
        logFile.open(GPU_LOG_FILE, std::ios::app);
        assert(logFile.is_open() && "Failed to open log file");
        logFile << "VK_ERROR : " << sourceLocation.file_name() << '(' << sourceLocation.line() << ") : " << sourceLocation.function_name() << " : VkResult(" << vkResult << ")\n";
        logFile.close();
        assert(vkResult == VK_SUCCESS && "VK_ERROR");
    }
}

namespace EWE {
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, const VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        EWE_VK(vkGetPhysicalDeviceMemoryProperties, physicalDevice, &memProperties);

        /*
        printf("memory heap count : %d \n", memProperties.memoryHeapCount);
        printf("memory type count : %d \n", memProperties.memoryTypeCount);

        for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++) {
            printf("heap[%d] size : %llu \n", i, memProperties.memoryHeaps->size);
            printf("heap flags : %d \n", memProperties.memoryHeaps->flags);
        }
        */

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            //printf("memory type[%d] heap index : %d \n", i, memProperties.memoryTypes->heapIndex);
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        assert(false && "find memory type unsupported");
        return UINT32_MAX;
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



#if USING_VMA
    StagingBuffer::StagingBuffer(VkDeviceSize size, VmaAllocator vmaAllocator, const void* data) {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationInfo vmaAllocInfo{};
        VmaAllocationCreateInfo vmaAllocCreateInfo{};
        vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vmaAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT;
        EWE_VK(vmaCreateBuffer, vmaAllocator, &bufferCreateInfo, &vmaAllocCreateInfo, &buffer, &vmaAlloc, &vmaAllocInfo);

        Stage(vmaAllocator, data, size);
    }
    StagingBuffer::StagingBuffer(VkDeviceSize size, VmaAllocator vmaAllocator) {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationInfo vmaAllocInfo{};
        VmaAllocationCreateInfo vmaAllocCreateInfo{};
        vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vmaAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT;
        EWE_VK(vmaCreateBuffer, vmaAllocator, &bufferCreateInfo, &vmaAllocCreateInfo, &buffer, &vmaAlloc, &vmaAllocInfo);
    }
#else
    StagingBuffer::StagingBuffer(VkDeviceSize size, VkPhysicalDevice physicalDevice, VkDevice device, const void* data) {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        EWE_VK(vkCreateBuffer, device, &bufferCreateInfo, nullptr, &buffer);

        VkMemoryRequirements memRequirements;
        EWE_VK(vkGetBufferMemoryRequirements, device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        EWE_VK(vkAllocateMemory, device, &allocInfo, nullptr, &memory);

        EWE_VK(vkBindBufferMemory, device, buffer, memory, 0);

        Stage(device, data, size);
    }
    StagingBuffer::StagingBuffer(VkDeviceSize size, VkPhysicalDevice physicalDevice, VkDevice device) {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        EWE_VK(vkCreateBuffer, device, &bufferCreateInfo, nullptr, &buffer);

        VkMemoryRequirements memRequirements;
        EWE_VK(vkGetBufferMemoryRequirements, device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        EWE_VK(vkAllocateMemory, device, &allocInfo, nullptr, &memory);

        EWE_VK(vkBindBufferMemory, device, buffer, memory, 0);
    }
#endif

#if USING_VMA
    void StagingBuffer::Free(VmaAllocator vmaAllocator) {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        EWE_VK(vmaDestroyBuffer, vmaAllocator, buffer, vmaAlloc);
#else
    void StagingBuffer::Free(VkDevice device) {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        EWE_VK(vkDestroyBuffer, device, buffer, nullptr);
        EWE_VK(vkFreeMemory, device, memory, nullptr);
#endif
    }

#if USING_VMA
    void StagingBuffer::Free(VmaAllocator vmaAllocator) const {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        EWE_VK(vmaDestroyBuffer, vmaAllocator, buffer, vmaAlloc);
#else
    void StagingBuffer::Free(VkDevice device) const {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        EWE_VK(vkDestroyBuffer, device, buffer, nullptr);
        EWE_VK(vkFreeMemory, device, memory, nullptr);
#endif
    }
#if USING_VMA
    void StagingBuffer::Stage(VmaAllocator vmaAllocator, const void* data, uint64_t bufferSize) {
        void* stagingData;

        EWE_VK(vmaMapMemory, vmaAllocator, vmaAlloc, &stagingData);
        memcpy(stagingData, data, bufferSize);
        EWE_VK(vmaUnmapMemory, vmaAllocator, vmaAlloc);
#else
    void StagingBuffer::Stage(VkDevice device, const void* data, uint64_t bufferSize) {
        void* stagingData;
        EWE_VK(vkMapMemory, device, memory, 0, bufferSize, 0, &stagingData);
        memcpy(stagingData, data, bufferSize);
        EWE_VK(vkUnmapMemory, device, memory);
#endif
    }

    TransferCallbackReturn FenceData::WaitReturnCallbacks(VkDevice device, uint64_t time) {
        mut.lock();
        VkResult ret = vkWaitForFences(device, 1, &fence, true, time);
        if (ret == VK_SUCCESS) {
            EWE_VK(vkResetFences, device, 1, &fence);
            mut.unlock();
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
            if (inlineCallbacks) {
                inlineCallbacks();
                inlineCallbacks = nullptr;
            }
            TransferCallbackReturn ret{ transferCallbacks };
            transferCallbacks.freeCommandBufferCallback = nullptr;
            transferCallbacks.otherCallbacks = nullptr;
            return ret;
        }
        else if (ret == VK_TIMEOUT) {
            mut.unlock();
            return TransferCallbackReturn{};
        }
        else {
            mut.unlock();
            EWE_VK_RESULT(ret);
            return TransferCallbackReturn{}; //error silencing, the above line should throw an error
        }
    }
}