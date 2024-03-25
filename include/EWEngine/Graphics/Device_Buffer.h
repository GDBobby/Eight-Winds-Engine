#pragma once

#include "Device.hpp"

namespace EWE {

    class EWEBuffer {
    public:
        EWEBuffer(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);

        ~EWEBuffer();
        static void deconstruct(EWEBuffer* deconstructedBuffer);

        EWEBuffer(const EWEBuffer&) = delete;
        EWEBuffer& operator=(const EWEBuffer&) = delete;

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();

        void writeToBufferAligned(void* data, VkDeviceSize size, uint64_t alignmentOffset);
        void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo* descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        VkResult flushMin(uint64_t offset);
        VkResult flushIndex(int index);
        VkDescriptorBufferInfo* descriptorInfoForIndex(int index);
        VkResult invalidateIndex(int index);

        VkBuffer getBuffer() const { return buffer_info.buffer; } //temporarily making this non-const
        void* getMappedMemory() const { return mapped; }
        //uint32_t getInstanceCount() const { return instanceCount; }
        //VkDeviceSize getInstanceSize() const { return instanceSize; }
        //VkDeviceSize getAlignmentSize() const { return instanceSize; }
        VkBufferUsageFlags getUsageFlags() const { return usageFlags; }
        VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
        VkDeviceSize getBufferSize() const { return bufferSize; }

        //allocated with new, up to the user to delete, or put it in a unique_ptr

        static EWEBuffer* createAndInitBuffer(void* data, uint64_t dataSize, uint64_t dataCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);

    private:
        VkDeviceSize getAlignment(VkDeviceSize instanceSize);

        void* mapped = nullptr;
        VkDescriptorBufferInfo buffer_info;
         
        VkDeviceMemory memory = VK_NULL_HANDLE;

        VkDeviceSize bufferSize;
        //uint32_t instanceCount;
        VkDeviceSize alignmentSize;
        VkBufferUsageFlags usageFlags;
        VkMemoryPropertyFlags memoryPropertyFlags;
        VkDeviceSize minOffsetAlignment = 1;
    };

}  // namespace EWE