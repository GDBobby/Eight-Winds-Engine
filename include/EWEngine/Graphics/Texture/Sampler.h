#pragma once
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Descriptors.h"

#include <unordered_map>
/*
* i need to figure out how to work this into the texture_builder


* i dont think a hash function is reasonable for VkSamplerCreateInfo
* vector isnt great either
*/
namespace EWE {
    class Sampler {
    private:
        static Sampler* samplerPtr;
        struct SamplerTracker {
            uint32_t inUseCount = 1;
            uint32_t totalUsed = 0;
            void Add();
            bool Remove();
        };
        struct SamplerDuplicateTracker {
            VkSamplerCreateInfo samplerInfo;
            VkSampler sampler{};
            //putting the tracker here instead of using a unordered_map<SamplerTracker, VkSampler>
            SamplerTracker tracker{}; 
            
            SamplerDuplicateTracker(VkSamplerCreateInfo& samplerInfo);
        };
        Sampler();
        std::vector<SamplerDuplicateTracker> storedSamplers{};

    public:
        static VkSampler GetSampler(VkSamplerCreateInfo& samplerInfo);
        static void RemoveSampler(VkSampler sampler);

    };
}
