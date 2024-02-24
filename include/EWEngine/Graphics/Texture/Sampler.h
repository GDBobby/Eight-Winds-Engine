#pragma once
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Descriptors.h"

#include <unordered_map>
/*
* i need to figure out how to work this into the texture_builder


* i dont think a hash function is reasonable for VkSamplerCreateInfo
* so instead of using an unordered_map, im just going to do a vector of pairs, and iterate through it with a memory comparison


* i need a better container. vector isn't going to cut it.
* id like a faster way to access the tracker
* i'd like to not have to shift all the data when 1 sampler is erased
*/
namespace EWE {
    inline bool bitwiseEqualOperator(VkSamplerCreateInfo& lhs, VkSamplerCreateInfo& rhs) {
		return memcmp(&lhs, &rhs, sizeof(VkSamplerCreateInfo)) == 0;
	}


    class Sampler {
    private:
        struct SamplerTracker {
            uint32_t inUseCount = 1;
            uint32_t totalUsed = 0;
            void add();
            bool remove();
        };
        struct SamplerDuplicateTracker {
            VkSamplerCreateInfo samplerInfo;
            VkSampler sampler{};
            //putting the tracker here instead of using a unordered_map<SamplerTracker, VkSampler>
            SamplerTracker tracker{}; 
            
            SamplerDuplicateTracker(EWEDevice& device, VkSamplerCreateInfo& samplerInfo);
        };

        std::vector<SamplerDuplicateTracker> storedSamplers{};

    public:
        VkSampler getSampler(EWEDevice& device, VkSamplerCreateInfo& samplerInfo);
        void removeSampler(EWEDevice& device, VkSampler sampler);

    };
}
