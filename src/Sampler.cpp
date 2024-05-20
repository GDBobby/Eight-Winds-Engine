#include "EWEngine/Graphics/Texture/Sampler.h"


namespace EWE {
    inline bool BitwiseEqualOperator(VkSamplerCreateInfo const& lhs, VkSamplerCreateInfo const& rhs) {
		return memcmp(&lhs, &rhs, sizeof(VkSamplerCreateInfo)) == 0;
	}

    Sampler* Sampler::samplerPtr = nullptr;
    Sampler::Sampler(){
        assert(samplerPtr == nullptr);
        samplerPtr = this;
        storedSamplers.reserve(50);
    }

	void Sampler::SamplerTracker::Add() {
        inUseCount++;
        totalUsed++;

#ifdef _DEBUG
        printf("sampler count after add : %d \n", inUseCount);
#endif
    }
    bool Sampler::SamplerTracker::Remove() {
#ifdef _DEBUG
        if (inUseCount == 0) {
            printf("removing sampler from tracker when none exist");
            throw std::runtime_error("removing a sampler when none exist");
        }
#endif
        inUseCount--;
        return inUseCount == 0;
    }


    Sampler::SamplerDuplicateTracker::SamplerDuplicateTracker(VkSamplerCreateInfo& samplerInfo) : samplerInfo(samplerInfo) {
        EWE_VK_ASSERT(vkCreateSampler(EWEDevice::GetVkDevice(), &samplerInfo, nullptr, &sampler));
    }


    VkSampler Sampler::GetSampler(VkSamplerCreateInfo& samplerInfo) {

        SamplerDuplicateTracker* foundDuplicate = nullptr;
        for (auto& duplicate : samplerPtr->storedSamplers) {
            if (BitwiseEqualOperator(duplicate.samplerInfo, samplerInfo)) {
                duplicate.tracker.Add();
                return duplicate.sampler;
            }
        }
        samplerPtr->storedSamplers.push_back(samplerInfo);
        return samplerPtr->storedSamplers.back().sampler;
    }

    void Sampler::RemoveSampler(VkSampler sampler) {
        for (auto iter = samplerPtr->storedSamplers.begin(); iter != samplerPtr->storedSamplers.end(); iter++) {
            if (iter->sampler == sampler) {
                if (iter->tracker.Remove()) {
                    samplerPtr->storedSamplers.erase(iter);
                }
                return;
            }
        }
        assert(false && "removing a sampler that does not exist");
    }
}