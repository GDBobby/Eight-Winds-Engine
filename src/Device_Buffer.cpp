#include "EWEngine/Graphics/Device_Buffer.h"

// std
#include <cassert>
#include <cstring>

namespace EWE {

    /**
     * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
     *
     * @param instanceSize The size of an instance
     * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
     * minUniformBufferOffsetAlignment)
     *
     * @return VkResult of the buffer mapping call
     */
    VkDeviceSize EWEBuffer::GetAlignment(VkDeviceSize instanceSize) {

        if (((usageFlags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ||
            ((usageFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) == VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
            ) {
            minOffsetAlignment = 1;
        }
        else if (((usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)) {
            minOffsetAlignment = EWEDevice::GetEWEDevice()->GetProperties().limits.minUniformBufferOffsetAlignment;
        }
        else if (((usageFlags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) {
            minOffsetAlignment = EWEDevice::GetEWEDevice()->GetProperties().limits.minStorageBufferOffsetAlignment;
        }

        if (minOffsetAlignment > 0) {
            //printf("get alignment size : %zu \n", (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1));
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    EWEBuffer::EWEBuffer(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
        : usageFlags{usageFlags}, memoryPropertyFlags{memoryPropertyFlags} {

        //buffer_info.buffer = VK_NULL_HANDLE; //not sure if necessary??

        alignmentSize = GetAlignment(instanceSize);
        bufferSize = alignmentSize * instanceCount;
        EWEDevice::GetEWEDevice()->CreateBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer_info.buffer, memory);
    }

    EWEBuffer::~EWEBuffer() {
        Unmap();
        vkDestroyBuffer(EWEDevice::GetEWEDevice()->Device(), buffer_info.buffer, nullptr);
        vkFreeMemory(EWEDevice::GetEWEDevice()->Device(), memory, nullptr);
    }
    void EWEBuffer::Reconstruct(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags) {
        Unmap();
        vkDestroyBuffer(EWEDevice::GetEWEDevice()->Device(), buffer_info.buffer, nullptr);
        vkFreeMemory(EWEDevice::GetEWEDevice()->Device(), memory, nullptr);

        this->usageFlags = usageFlags;
        this->memoryPropertyFlags = memoryPropertyFlags;

        alignmentSize = GetAlignment(instanceSize);
        bufferSize = alignmentSize * instanceCount;
        EWEDevice::GetEWEDevice()->CreateBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer_info.buffer, memory);

        Map();
    }


    /**
     * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
     *
     * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
     * buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the buffer mapping call
     */
    VkResult EWEBuffer::Map(VkDeviceSize size, VkDeviceSize offset) {
#ifdef _DEBUG
        assert(buffer_info.buffer && memory && "Called map on buffer before create");
#endif
        return vkMapMemory(EWEDevice::GetEWEDevice()->Device(), memory, offset, size, 0, &mapped);
    }

    /**
     * Unmap a mapped memory range
     *
     * @note Does not return a result as vkUnmapMemory can't fail
     */
    void EWEBuffer::Unmap() {
        if (mapped) {
            vkUnmapMemory(EWEDevice::GetEWEDevice()->Device(), memory);
            mapped = nullptr;
        }
    }

    /**
     * Copies the specified data to the mapped buffer. Default value writes whole buffer range
     *
     * @param data Pointer to the data to copy
     * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
     * range.
     * @param offset (Optional) Byte offset from beginning of mapped region
     *
     */
    void EWEBuffer::WriteToBufferAligned(void* data, VkDeviceSize size, uint64_t alignmentOffset) {
        assert(mapped && "Cannot copy to unmapped buffer");

        char* memOffset = (char*)mapped;
#if _DEBUG
        uint64_t offset = alignmentOffset * alignmentSize;
        if ((offset + size) > bufferSize) {
            printf("overflow error in buffer - %zu:%zu \n", offset + size, bufferSize);
            throw std::exception("DATA TOO LARGE FOR BUFFER");
        }
        memOffset += offset;
        memcpy(memOffset, data, size);
#else
        memOffset += alignmentOffset * alignmentSize;
        memcpy(memOffset, data, size);
#endif

        
    }

    void EWEBuffer::WriteToBuffer(void const* data, VkDeviceSize size, VkDeviceSize offset) {
        assert(mapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE) {
            memcpy(mapped, data, bufferSize);
        }
        else {
#if _DEBUG
            if ((offset + size) > bufferSize) {
                printf("overflow error in buffer - %zu:%zu \n", offset+size, bufferSize);
                throw std::exception("DATA TOO LARGE FOR BUFFER");
            }
#endif

            char* memOffset = (char*)mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    /**
     * Flush a memory range of the buffer to make it visible to the device
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
     * complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the flush call
     */
    VkResult EWEBuffer::Flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(EWEDevice::GetEWEDevice()->Device(), 1, &mappedRange);
    }
    VkResult EWEBuffer::FlushMin(uint64_t offset) {
        VkDeviceSize trueOffset = offset - (offset % minOffsetAlignment);
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = trueOffset;
        mappedRange.size = minOffsetAlignment;
#ifdef _DEBUG
        printf("flushing minimal : %zu \n", minOffsetAlignment);
#endif
        return vkFlushMappedMemoryRanges(EWEDevice::GetEWEDevice()->Device(), 1, &mappedRange);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
     * the complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the invalidate call
     */
    VkResult EWEBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(EWEDevice::GetEWEDevice()->Device(), 1, &mappedRange);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param size (Optional) Size of the memory range of the descriptor
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkDescriptorBufferInfo of specified offset and range
     */
    VkDescriptorBufferInfo* EWEBuffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        buffer_info.offset = offset;
        buffer_info.range = size;
        return &buffer_info;
        //return &VkDescriptorBufferInfo{ buffer, offset, size, };
    }

    /**
     *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
     *
     * @param index Used in offset calculation
     *
     */
    VkResult EWEBuffer::FlushIndex(int index) { return Flush(alignmentSize, index * alignmentSize); }

    /**
     * Create a buffer info descriptor
     *
     * @param index Specifies the region given by index * alignmentSize
     *
     * @return VkDescriptorBufferInfo for instance at index
     */
    VkDescriptorBufferInfo* EWEBuffer::DescriptorInfoForIndex(int index) {
        return DescriptorInfo(alignmentSize, index * alignmentSize);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param index Specifies the region to invalidate: index * alignmentSize
     *
     * @return VkResult of the invalidate call
     */
    VkResult EWEBuffer::InvalidateIndex(int index) {
        return Invalidate(alignmentSize, index * alignmentSize);
    }

    EWEBuffer* EWEBuffer::CreateAndInitBuffer(void* data, uint64_t dataSize, uint64_t dataCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags) {
        EWEBuffer* retBuffer = ConstructSingular<EWEBuffer>(ewe_call_trace, dataSize * dataCount, 1, usageFlags, memoryPropertyFlags);
        
        retBuffer->Map();
        retBuffer->WriteToBuffer(data, dataSize * dataCount);
        retBuffer->Flush();
        return retBuffer;
    }

}  // namespace EWE