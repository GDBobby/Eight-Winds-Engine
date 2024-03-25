#include "EWEngine/Graphics/Texture/Sampler.h"


namespace EWE {
	void Sampler::SamplerTracker::add() {
        inUseCount++;
        totalUsed++;

#ifdef _DEBUG
        printf("sampler count after add : %d \n", inUseCount);
#endif
    }
    bool Sampler::SamplerTracker::remove() {
#ifdef _DEBUG
        if (inUseCount == 0) {
            printf("removing sampler from tracker when none exist");
            throw std::runtime_error("removing a sampler when none exist");
        }
#endif
        inUseCount--;
        return inUseCount == 0;
    }


    Sampler::SamplerDuplicateTracker::SamplerDuplicateTracker(EWEDevice& device, VkSamplerCreateInfo& samplerInfo) : samplerInfo(samplerInfo) {
        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler");
        }
    }


    VkSampler Sampler::getSampler(EWEDevice& device, VkSamplerCreateInfo& samplerInfo) {
        SamplerDuplicateTracker* foundDuplicate = nullptr;
        for (auto& duplicate : storedSamplers) {
            if (bitwiseEqualOperator(duplicate.samplerInfo, samplerInfo)) {
                duplicate.tracker.add();
                return duplicate.sampler;
            }
        }
        auto& emplaceRet = storedSamplers.emplace_back(device, samplerInfo);
        return emplaceRet.sampler;

        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        return VK_NULL_HANDLE;
    }

    void Sampler::removeSampler(EWEDevice& device, VkSampler sampler) {
        for (auto iter = storedSamplers.begin(); iter != storedSamplers.end(); iter++) {
            if (iter->sampler == sampler) {
                if (iter->tracker.remove()) {
                    storedSamplers.erase(iter);
                }
                return;
            }
        }
        throw std::runtime_error("removing a sampler that does not exist");
    }
}