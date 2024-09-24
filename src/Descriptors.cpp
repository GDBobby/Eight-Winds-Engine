#include "EWEngine/Graphics/Descriptors.h"

// std
#include <cassert>
#include <stdexcept>

#define DESCRIPTOR_DEBUGGING true

namespace EWE {

    // *************** Descriptor Set Layout Builder *********************

    EWEDescriptorSetLayout::Builder& EWEDescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count) {
        assert(!bindings.contains(binding) && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings.emplace(binding, layoutBinding);
        return *this;
    }

    EWEDescriptorSetLayout* EWEDescriptorSetLayout::Builder::build() const {
        EWEDescriptorSetLayout* ret = ConstructSingular<EWEDescriptorSetLayout>(ewe_call_trace, bindings);
        return ret;
    }

    // *************** Descriptor Set Layout *********************

    EWEDescriptorSetLayout::EWEDescriptorSetLayout(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> const& bindings)
        : bindings{ bindings } {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        setLayoutBindings.reserve(bindings.size());
        for (auto& kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        EWE_VK_ASSERT(vkCreateDescriptorSetLayout(EWEDevice::GetVkDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout));
    }
    void EWEDescriptorSetLayout::construct(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> const& bindings) {
        this->bindings = bindings;
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        setLayoutBindings.reserve(bindings.size());
        for (auto& kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        EWE_VK_ASSERT(vkCreateDescriptorSetLayout(EWEDevice::GetVkDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout));
    }

    EWEDescriptorSetLayout::~EWEDescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(EWEDevice::GetVkDevice(), descriptorSetLayout, nullptr);
#if EWE_DEBUG
        printf("probably have memory leaks currently, address this ASAP \n");
        //the reason i have this print statement (feb 2024)
        //i changed the builder to return a new pointer instead of a unique pointer
        //all deconstruction must be followed by delete. 
        //if this is printed, and it is not preceded by delete, it is a memory leak
        //the print is being put in, instead of immediately addressing the issue because i have multiple other bugs im dealing with currently
        //that all come before deconstruction time
#endif
    }

    // *************** Descriptor Pool Builder *********************

    EWEDescriptorPool::Builder& EWEDescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({ descriptorType, count });
        return *this;
    }

    EWEDescriptorPool::Builder& EWEDescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }
    EWEDescriptorPool::Builder& EWEDescriptorPool::Builder::SetMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::shared_ptr<EWEDescriptorPool> EWEDescriptorPool::Builder::Build() const {
        return std::make_shared<EWEDescriptorPool>(maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    std::unordered_map<uint16_t, EWEDescriptorPool> EWEDescriptorPool::pools{};

    EWEDescriptorPool::EWEDescriptorPool(uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes) {

        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        for (auto& poolSize : poolSizes) {
            trackers.emplace(poolSize.type, DescriptorTracker(poolSize.descriptorCount));
        }

        EWE_VK_ASSERT(vkCreateDescriptorPool(EWEDevice::GetVkDevice(), &descriptorPoolInfo, nullptr, &descriptorPool));
    }
    EWEDescriptorPool::EWEDescriptorPool(VkDescriptorPoolCreateInfo& pool_info) {
        for (int i = 0; i < pool_info.poolSizeCount; i++) {
            trackers.emplace(pool_info.pPoolSizes[i].type, DescriptorTracker(pool_info.pPoolSizes[i].descriptorCount));
        }

        EWE_VK_ASSERT(vkCreateDescriptorPool(EWEDevice::GetVkDevice(), &pool_info, nullptr, &descriptorPool));
    }

    EWEDescriptorPool::~EWEDescriptorPool() {
        printf("before destroy pool \n");
        for (auto& tracker : trackers) {
			printf("active:max - %d:%d \n", tracker.second.current, tracker.second.max);

		}

        vkDestroyDescriptorPool(EWEDevice::GetVkDevice(), descriptorPool, nullptr);
        printf("after destroy pool \n");
    }
    bool EWEDescriptorPool::AllocateDescriptor(DescriptorPool_ID poolID, const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) {
        return pools.at(poolID).AllocateDescriptor(descriptorSetLayout, descriptor);
    }
    bool EWEDescriptorPool::AllocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        if (vkAllocateDescriptorSets(EWEDevice::GetVkDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }
    void EWEDescriptorPool::FreeDescriptors(DescriptorPool_ID poolID, std::vector<VkDescriptorSet>& descriptors) {
        pools.at(poolID).FreeDescriptors(descriptors);
        //printf("active descriptors after removal : %d \n", activeDescriptors);
    }
    void EWEDescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            EWEDevice::GetVkDevice(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
        activeDescriptors -= descriptors.size();
        //printf("active descriptors after removal : %d \n", activeDescriptors);
    }
    void EWEDescriptorPool::FreeDescriptor(DescriptorPool_ID poolID, VkDescriptorSet* descriptor) {
        pools.at(poolID).FreeDescriptor(descriptor);
    }
    void EWEDescriptorPool::FreeDescriptor(VkDescriptorSet* descriptor) const {
        vkFreeDescriptorSets(
            EWEDevice::GetVkDevice(),
            descriptorPool,
            1,
            descriptor);
        activeDescriptors--;
        //printf("active descriptors after removal : %d \n", activeDescriptors);
    }
    void EWEDescriptorPool::ResetPool() {
        vkResetDescriptorPool(EWEDevice::GetVkDevice(), descriptorPool, 0);
    }
    void EWEDescriptorPool::BuildGlobalPool() {
        uint32_t maxSets = 1000;
        std::vector<VkDescriptorPoolSize> poolSizes{};
        VkDescriptorPoolSize poolSize;
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 200;
        poolSizes.emplace_back(poolSize);
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSizes.emplace_back(poolSize);
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes.emplace_back(poolSize);
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        poolSizes.emplace_back(poolSize);
        VkDescriptorPoolCreateFlags poolFlags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        EWEDescriptorPool::pools.try_emplace(0, maxSets, poolFlags, poolSizes);
        //EWEDescriptorPool(EWEDevice & eweDevice, uint32_t maxSets,VkDescriptorPoolCreateFlags poolFlags,const std::vector<VkDescriptorPoolSize>&poolSizes);
    }
    void EWEDescriptorPool::AddPool(DescriptorPool_ID poolID, VkDescriptorPoolCreateInfo& pool_info) {
        EWEDescriptorPool::pools.try_emplace(poolID, pool_info);
    }

    void EWEDescriptorPool::DestructPools() {
		pools.clear();
	}
    void EWEDescriptorPool::DestructPool(DescriptorPool_ID poolID) {
#if EWE_DEBUG
        printf("deconstructing pool : %d \n", poolID);
        assert(pools.contains(poolID) && "destructing pool that doesn't exist");
#endif
        pools.erase(poolID);
    }

    bool EWEDescriptorPool::DescriptorTracker::AddDescriptor(uint32_t count) {
        current += count;
        return current >= max;
    }

    void EWEDescriptorPool::AddDescriptorToTrackers(VkDescriptorType descType, uint32_t count) {
        if (trackers.at(descType).AddDescriptor(count)) {
            printf("adding too many descirptors - type:max - %d:%d \n", descType, trackers.at(descType).max);
            assert(false && "Descriptor pool exhausted");
        }
    }


    // *************** Descriptor Writer *********************

    EWEDescriptorWriter::EWEDescriptorWriter(EWEDescriptorSetLayout* setLayout, EWEDescriptorPool& pool)
        : setLayout{ setLayout }, pool{ pool } {}
    EWEDescriptorWriter::EWEDescriptorWriter(EWEDescriptorSetLayout* setLayout, DescriptorPool_ID poolID) 
        : setLayout{ setLayout }, pool{ EWEDescriptorPool::pools.at(poolID) }
    {}

    EWEDescriptorWriter& EWEDescriptorWriter::WriteBuffer( uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout->bindings.contains(binding) && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout->bindings.at(binding);

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    EWEDescriptorWriter& EWEDescriptorWriter::WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(setLayout->bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout->bindings.at(binding);

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    VkDescriptorSet EWEDescriptorWriter::Build() {
#if DESCRIPTOR_DEBUGGING
        return BuildPrint();
#else

        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);

        activeDescriptors++;
        for (auto& binding : setLayout.bindings) {
            pool.addDescriptorToTrackers(binding.second.descriptorType, binding.second.descriptorCount);
        }
        //printf("active descriptors after addition : %d \n", activeDescriptors);
        if (!success) {
            throw std::runtime_error("failed to construct descriptor set");
            return VK_NULL_HANDLE;
        }
        overwrite(set);
        return set;
#endif
    }
    VkDescriptorSet EWEDescriptorWriter::BuildPrint() {
        VkDescriptorSet set;
        bool success = pool.AllocateDescriptor(setLayout->GetDescriptorSetLayout(), set);

        activeDescriptors++;
        for (int i = 0; i < setLayout->bindings.size(); i++) {
            if (setLayout->bindings.at(i).descriptorCount != 1) {
                //printf("\t count:%d\n", setLayout.bindings.at(i).descriptorCount);
            }
            pool.AddDescriptorToTrackers(setLayout->bindings.at(i).descriptorType, setLayout->bindings.at(i).descriptorCount);
        }
        //printf("active descriptors after addition : %d \n", activeDescriptors);
        assert(success && "failed to construct descriptor set");
        Overwrite(set);
        return set;
    }

    void EWEDescriptorWriter::Overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(EWEDevice::GetVkDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
}  // namespace EWE