#pragma once

#include "Device.hpp"

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
            Builder() {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            EWEDescriptorSetLayout* build() const;

        private:
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        EWEDescriptorSetLayout(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> const& bindings);
        void construct(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> const& bindings);
        ~EWEDescriptorSetLayout();
        EWEDescriptorSetLayout(const EWEDescriptorSetLayout&) = delete;
        EWEDescriptorSetLayout& operator=(const EWEDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class EWEDescriptorWriter;
    };

    enum DescriptorPool_ID : uint16_t {
        DescriptorPool_Global = 0,
        DescriptorPool_imgui = 1,
        //DescriptorPool_Compute = 1, //i dont even think im using this
    };

    class EWEDescriptorPool {
    public:
        class Builder {
        public:
            Builder() {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::shared_ptr<EWEDescriptorPool> build() const;


        private:
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        EWEDescriptorPool(uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);
        EWEDescriptorPool(VkDescriptorPoolCreateInfo& pool_info);
        ~EWEDescriptorPool();
        EWEDescriptorPool(const EWEDescriptorPool&) = delete;
        EWEDescriptorPool& operator=(const EWEDescriptorPool&) = delete;

        static bool allocateDescriptor(DescriptorPool_ID poolID, const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor);
        bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        static void freeDescriptors(DescriptorPool_ID poolID, std::vector<VkDescriptorSet>& descriptors);
        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        static void freeDescriptor(DescriptorPool_ID poolID, VkDescriptorSet* descriptors);
        void freeDescriptor(VkDescriptorSet* descriptor) const;

        //void getPool(); maybe later for imGuiHandler, not rn

        void resetPool();
        static void BuildGlobalPool();

        static void DestructPools();
        static void DestructPool(DescriptorPool_ID poolID);
        static void AddPool(DescriptorPool_ID poolID, VkDescriptorPoolCreateInfo& pool_info);

    private:
        struct DescriptorTracker {
            uint32_t current = 0;
            uint32_t max;
            std::vector<VkDescriptorSet*> descriptors{};
            DescriptorTracker(uint32_t maxCount) : max{ maxCount } {}

            bool addDescriptor(uint32_t count);
        };

        std::unordered_map<VkDescriptorType, DescriptorTracker> trackers;

        VkDescriptorPool descriptorPool;

        void addDescriptorToTrackers(VkDescriptorType descType, uint32_t count);

        static std::unordered_map<uint16_t, EWEDescriptorPool> pools;
        friend class EWEDescriptorWriter;
    };

    class EWEDescriptorWriter {
    public:
        EWEDescriptorWriter(EWEDescriptorSetLayout* setLayout, EWEDescriptorPool& pool);
        EWEDescriptorWriter(EWEDescriptorSetLayout* setLayout, DescriptorPool_ID poolID);

        EWEDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        EWEDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        VkDescriptorSet build();
        void overwrite(VkDescriptorSet& set);

    private:
        VkDescriptorSet buildPrint();
        EWEDescriptorSetLayout* setLayout;
        EWEDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

    

}  // namespace EWE