#pragma once

#include "EWE_Device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace EWE {

    static int64_t activeDescriptors = 0;

    class EWEDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(EWEDevice& eweDevice) : eweDevice{ eweDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<EWEDescriptorSetLayout> build() const;

        private:
            EWEDevice& eweDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        EWEDescriptorSetLayout(EWEDevice& eweDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~EWEDescriptorSetLayout();
        EWEDescriptorSetLayout(const EWEDescriptorSetLayout&) = delete;
        EWEDescriptorSetLayout& operator=(const EWEDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        EWEDevice& eweDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class EWEDescriptorWriter;
    };

    class EWEDescriptorPool {
    public:
        class Builder {
        public:
            Builder(EWEDevice& eweDevice) : eweDevice{ eweDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::shared_ptr<EWEDescriptorPool> build() const;

        private:
            EWEDevice& eweDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        EWEDescriptorPool(
            EWEDevice& eweDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~EWEDescriptorPool();
        EWEDescriptorPool(const EWEDescriptorPool&) = delete;
        EWEDescriptorPool& operator=(const EWEDescriptorPool&) = delete;

        bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
        void freeDescriptor(VkDescriptorSet* descriptor) const;

        //void getPool(); maybe later for imGuiHandler, not rn

        void resetPool();

    private:
        EWEDevice& eweDevice;
        VkDescriptorPool descriptorPool;

        friend class EWEDescriptorWriter;
    };

    class EWEDescriptorWriter {
    public:
        EWEDescriptorWriter(EWEDescriptorSetLayout& setLayout, EWEDescriptorPool& pool);

        EWEDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        EWEDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        EWEDescriptorSetLayout& setLayout;
        EWEDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

    

}  // namespace EWE