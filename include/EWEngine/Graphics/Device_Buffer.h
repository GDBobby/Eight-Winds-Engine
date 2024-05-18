#pragma once

#include "Device.hpp"

namespace EWE {

    class EWEBuffer {
    public:
        EWEBuffer(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);

        ~EWEBuffer();
        void Reconstruct(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);

        EWEBuffer(const EWEBuffer&) = delete;
        EWEBuffer& operator=(const EWEBuffer&) = delete;

        VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap();

        void WriteToBufferAligned(void* data, VkDeviceSize size, uint64_t alignmentOffset);
        void WriteToBuffer(void const* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo* DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        VkResult FlushMin(uint64_t offset);
        VkResult FlushIndex(int index);
        VkDescriptorBufferInfo* DescriptorInfoForIndex(int index);
        VkResult InvalidateIndex(int index);

        VkBuffer GetBuffer() const { return buffer_info.buffer; } //temporarily making this non-const
        void* GetMappedMemory() const { return mapped; }
        //uint32_t getInstanceCount() const { return instanceCount; }
        //VkDeviceSize getInstanceSize() const { return instanceSize; }
        //VkDeviceSize getAlignmentSize() const { return instanceSize; }
        VkBufferUsageFlags GetUsageFlags() const { return usageFlags; }
        VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return memoryPropertyFlags; }
        VkDeviceSize GetBufferSize() const { return bufferSize; }

        //allocated with new, up to the user to delete, or put it in a unique_ptr

        static EWEBuffer* CreateAndInitBuffer(void* data, uint64_t dataSize, uint64_t dataCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);

    private:
        VkDeviceSize GetAlignment(VkDeviceSize instanceSize);

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