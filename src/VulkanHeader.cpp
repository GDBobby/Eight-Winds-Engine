#include "EWEngine/Graphics/VulkanHeader.h"

#if USING_VMA
#define VMA_IMPLEMENTATION
#include "EWEngine/Graphics/vk_mem_alloc.h"
#endif



namespace EWE {
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, const VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

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
#if DEBUGGING_MEMORY_WITH_VMA
        vmaAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT;
#endif
        EWE_VK_ASSERT(vmaCreateBuffer(vmaAllocator, &bufferCreateInfo, &vmaAllocCreateInfo, &buffer, &vmaAlloc, &vmaAllocInfo));

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
#if DEBUGGING_MEMORY_WITH_VMA
        vmaAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT;
#endif
        EWE_VK_ASSERT(vmaCreateBuffer(vmaAllocator, &bufferCreateInfo, &vmaAllocCreateInfo, &buffer, &vmaAlloc, &vmaAllocInfo));
    }
#else
    StagingBuffer::StagingBuffer(VkDeviceSize size, VkDevice device, const void* data) {
        EWE_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        EWE_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &memory));

        EWE_VK_ASSERT(vkBindBufferMemory(device, buffer, memory, 0));

        Stage(device, data, size);
    }
    StagingBuffer::StagingBuffer(VkDeviceSize size, VkDevice device) {
        EWE_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        EWE_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &memory));

        EWE_VK_ASSERT(vkBindBufferMemory(device, buffer, memory, 0));
    }
#endif

#if USING_VMA
    void StagingBuffer::Free(VmaAllocator vmaAllocator) {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        vmaDestroyBuffer(vmaAllocator, buffer, vmaAlloc);
#else
    void StagingBuffer::Free(VkDevice device) {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
#endif
    }

#if USING_VMA
    void StagingBuffer::Free(VmaAllocator vmaAllocator) const {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        vmaDestroyBuffer(vmaAllocator, buffer, vmaAlloc);
#else
    void StagingBuffer::Free(VkDevice device) const {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
#endif
    }
#if USING_VMA
    void StagingBuffer::Stage(VmaAllocator vmaAllocator, const void* data, uint64_t bufferSize) {
        void* stagingData;

        vmaMapMemory(vmaAllocator, vmaAlloc, &stagingData);
        memcpy(stagingData, data, bufferSize);
        vmaUnmapMemory(vmaAllocator, vmaAlloc);
#else
    void StagingBuffer::Stage(VkDevice device, const void* data, uint64_t bufferSize) {
        void* stagingData;
        vkMapMemory(device, memory, 0, bufferSize, 0, &stagingData);
        memcpy(stagingData, data, bufferSize);
        vkUnmapMemory(device, memory);
#endif
    }

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