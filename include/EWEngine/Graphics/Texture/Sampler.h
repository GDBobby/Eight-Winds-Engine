#pragma once
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Descriptors.h"

#include <unordered_map>
/*
* i need to figure out how to work this into the texture_builder


* i dont think a hash function is reasonable for VkSamplerCreateInfo
* vector isnt great either
* i dont want memory to be moved, and I don't want to create a hash for VkSamplerCreateInfo
*/

#define SAMPLER_DUPLICATION_TRACKING true

namespace EWE {
    class Sampler {
    private:
#if SAMPLER_DUPLICATION_TRACKING
        static Sampler* samplerPtr;
        struct SamplerTracker {
            uint32_t inUseCount = 1;
            uint32_t totalUsed = 0;
            void Add();
            bool Remove();
        };
        struct SamplerDuplicateTracker {
            VkSamplerCreateInfo samplerInfo;
            VkSampler sampler;
            //putting the tracker here instead of using a unordered_map<SamplerTracker, VkSampler>
            SamplerTracker tracker{}; 
            
            SamplerDuplicateTracker(VkSamplerCreateInfo const& samplerInfo);
        };
        Sampler();
        std::vector<SamplerDuplicateTracker> storedSamplers{};
#else
        std::vector<VkSampler> storedSamplers{};
#endif

    public:
        static VkSampler GetSampler(VkSamplerCreateInfo const& samplerInfo);
        static void RemoveSampler(VkSampler sampler);
        static void Initialize();
        static void Deconstruct();

    };
}
