#include "EWEngine/Graphics/Texture/Sampler.h"

#define EXPECTED_MAXIMUM_AMOUNT_OF_SAMPLERS 50

namespace EWE {
#if SAMPLER_DUPLICATION_TRACKING
    inline bool BitwiseEqualOperator(VkSamplerCreateInfo const& lhs, VkSamplerCreateInfo const& rhs) {
		return memcmp(&lhs, &rhs, sizeof(VkSamplerCreateInfo)) == 0;
	}
    Sampler* Sampler::samplerPtr = nullptr;
    Sampler::Sampler(){
        assert(samplerPtr == nullptr);
        samplerPtr = this;
        storedSamplers.reserve(EXPECTED_MAXIMUM_AMOUNT_OF_SAMPLERS);
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


    Sampler::SamplerDuplicateTracker::SamplerDuplicateTracker(VkSamplerCreateInfo const& samplerInfo) : samplerInfo(samplerInfo) {
        EWE_VK_ASSERT(vkCreateSampler(EWEDevice::GetVkDevice(), &samplerInfo, nullptr, &sampler));
    }
#endif

    VkSampler Sampler::GetSampler(VkSamplerCreateInfo const& samplerInfo) {
#if SAMPLER_DUPLICATION_TRACKING
        SamplerDuplicateTracker* foundDuplicate = nullptr;
        for (auto& duplicate : samplerPtr->storedSamplers) {
            if (BitwiseEqualOperator(duplicate.samplerInfo, samplerInfo)) {
                duplicate.tracker.Add();
                return duplicate.sampler;
            }
        }
#ifdef _DEBUG
        assert(samplerPtr->storedSamplers.size() < EXPECTED_MAXIMUM_AMOUNT_OF_SAMPLERS && "warning: sampler count is greater than expected maximum amount of samplers");
#endif
        samplerPtr->storedSamplers.emplace_back(samplerInfo);
        return samplerPtr->storedSamplers.back().sampler;
#else
        VkSampler sampler;
		EWE_VK_ASSERT(vkCreateSampler(EWEDevice::GetVkDevice(), &samplerInfo, nullptr, &sampler));
		return sampler;
#endif
    }
    void Sampler::RemoveSampler(VkSampler sampler) {
#if SAMPLER_DUPLICATION_TRACKING
        for (auto iter = samplerPtr->storedSamplers.begin(); iter != samplerPtr->storedSamplers.end(); iter++) {
            if (iter->sampler == sampler) {
                if (iter->tracker.Remove()) {
                    samplerPtr->storedSamplers.erase(iter);
                }
                return;
            }
        }
        assert(false && "removing a sampler that does not exist");
#else
        vkDestroySampler(EWEDevice::GetVkDevice(), sampler, nullptr);
#endif
    }
    void Sampler::Initialize() {
#if SAMPLER_DUPLICATION_TRACKING
        samplerPtr = new Sampler();
        ewe_alloc_mem_track(samplerPtr, ewe_call_trace);
#endif
    }
    void Sampler::Deconstruct() {
#if SAMPLER_DUPLICATION_TRACKING
        delete samplerPtr;
        ewe_free(samplerPtr);
#endif
    }

}