#include "EWEngine/graphics/EWE_descriptors.h"

// std
#include <cassert>
#include <stdexcept>

namespace EWE {

    // *************** Descriptor Set Layout Builder *********************

    EWEDescriptorSetLayout::Builder& EWEDescriptorSetLayout::Builder::addBinding( uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<EWEDescriptorSetLayout> EWEDescriptorSetLayout::Builder::build() const {
        return std::make_unique<EWEDescriptorSetLayout>(eweDevice, bindings);
    }

    // *************** Descriptor Set Layout *********************

    EWEDescriptorSetLayout::EWEDescriptorSetLayout( EWEDevice& eweDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : eweDevice{ eweDevice }, bindings{ bindings } {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(
            eweDevice.device(),
            &descriptorSetLayoutInfo,
            nullptr,
            &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    EWEDescriptorSetLayout::~EWEDescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(eweDevice.device(), descriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    EWEDescriptorPool::Builder& EWEDescriptorPool::Builder::addPoolSize( VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({ descriptorType, count });
        return *this;
    }

    EWEDescriptorPool::Builder& EWEDescriptorPool::Builder::setPoolFlags( VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }
    EWEDescriptorPool::Builder& EWEDescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::shared_ptr<EWEDescriptorPool> EWEDescriptorPool::Builder::build() const {
        return std::make_shared<EWEDescriptorPool>(eweDevice, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    EWEDescriptorPool::EWEDescriptorPool(EWEDevice& eweDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes)
        : eweDevice{ eweDevice } {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(eweDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    EWEDescriptorPool::~EWEDescriptorPool() {
        printf("before destroy pool \n");
        vkDestroyDescriptorPool(eweDevice.device(), descriptorPool, nullptr);
        printf("after destroy pool \n");
    }

    bool EWEDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        if (vkAllocateDescriptorSets(eweDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void EWEDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            eweDevice.device(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
        activeDescriptors -= descriptors.size();
        //printf("active descriptors after removal : %d \n", activeDescriptors);
    }
    void EWEDescriptorPool::freeDescriptor(VkDescriptorSet* descriptor) const {
        vkFreeDescriptorSets(
            eweDevice.device(),
            descriptorPool,
            1,
            descriptor);
        activeDescriptors--;
        //printf("active descriptors after removal : %d \n", activeDescriptors);
    }
    void EWEDescriptorPool::resetPool() {
        vkResetDescriptorPool(eweDevice.device(), descriptorPool, 0);
        //vkResetDescriptorPool(eweDevice.device(), descriptorPool, VK_DESCRIPTOR_POOL);
    }

    // *************** Descriptor Writer *********************

    EWEDescriptorWriter::EWEDescriptorWriter(EWEDescriptorSetLayout& setLayout, EWEDescriptorPool& pool)
        : setLayout{ setLayout }, pool{ pool } {}

    EWEDescriptorWriter& EWEDescriptorWriter::writeBuffer( uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    EWEDescriptorWriter& EWEDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

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

    bool EWEDescriptorWriter::build(VkDescriptorSet& set) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);

        activeDescriptors++;
        //printf("active descriptors after addition : %d \n", activeDescriptors);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void EWEDescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.eweDevice.device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

}  // namespace EWE