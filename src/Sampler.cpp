#include "EWEngine/Graphics/Texture/Sampler.h"

#include <vector>
#include <cassert>

#define EXPECTED_MAXIMUM_AMOUNT_OF_SAMPLERS 50

#define SAMPLER_DUPLICATION_TRACKING true

#if SAMPLER_DUPLICATION_TRACKING
namespace EWE {
    VkDevice device;


    struct SamplerTracker {
        uint32_t inUseCount = 1;
        uint32_t totalUsed = 0;
        void Add() {
            inUseCount++;
            totalUsed++;

#ifdef _DEBUG
            printf("sampler count after add : %d \n", inUseCount);
#endif
        }
        bool Remove() {
#ifdef _DEBUG
            assert(inUseCount > 0 && "removing sampler from tracker when none exist");
#endif
            inUseCount--;
            return inUseCount == 0;
        }
    };
    struct SamplerDuplicateTracker {
        VkSamplerCreateInfo samplerInfo;
        VkSampler sampler;
        //putting the tracker here instead of using a unordered_map<SamplerTracker, VkSampler>
        SamplerTracker tracker{};

        SamplerDuplicateTracker(VkSamplerCreateInfo const& samplerInfo) : samplerInfo(samplerInfo) {
            EWE_VK_ASSERT(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));
        }
    };


    std::vector<SamplerDuplicateTracker> storedSamplers{};
#else
    std::vector<VkSampler> storedSamplers{};
#endif

#if SAMPLER_DUPLICATION_TRACKING
    inline bool BitwiseEqualOperator(VkSamplerCreateInfo const& lhs, VkSamplerCreateInfo const& rhs) {
		return memcmp(&lhs, &rhs, sizeof(VkSamplerCreateInfo)) == 0;
	}

#endif


    namespace Sampler {
        VkSampler GetSampler(VkSamplerCreateInfo const& samplerInfo) {
#if SAMPLER_DUPLICATION_TRACKING
            SamplerDuplicateTracker* foundDuplicate = nullptr;
            for (auto& duplicate : storedSamplers) {
                if (BitwiseEqualOperator(duplicate.samplerInfo, samplerInfo)) {
                    duplicate.tracker.Add();
                    return duplicate.sampler;
                }
            }
#ifdef _DEBUG
            assert(storedSamplers.size() < EXPECTED_MAXIMUM_AMOUNT_OF_SAMPLERS && "warning: sampler count is greater than expected maximum amount of samplers");
#endif
            storedSamplers.emplace_back(samplerInfo);
            return storedSamplers.back().sampler;
#else
            VkSampler sampler;
            EWE_VK_ASSERT(vkCreateSampler(EWEDevice::GetVkDevice(), &samplerInfo, nullptr, &sampler));
            return sampler;
#endif
        }
        void RemoveSampler(VkSampler sampler) {
#if SAMPLER_DUPLICATION_TRACKING
            for (auto iter = storedSamplers.begin(); iter != storedSamplers.end(); iter++) {
                if (iter->sampler == sampler) {
                    if (iter->tracker.Remove()) {
                        storedSamplers.erase(iter);
                    }
                    return;
                }
            }
            assert(false && "removing a sampler that does not exist");
#else
            vkDestroySampler(EWEDevice::GetVkDevice(), sampler, nullptr);
#endif
        }
        void Initialize(VkDevice vkDevice) {

            device = vkDevice;
#if SAMPLER_DUPLICATION_TRACKING
            storedSamplers.reserve(EXPECTED_MAXIMUM_AMOUNT_OF_SAMPLERS);
#endif
        }
        void Deconstruct() {
            assert(storedSamplers.size() == 0 && "destroying sampler manager with samplers still in use");

            //if the sampler was already destroyed this will create a validation error
            //if the sampler was not destroyed, it'll be fun tracing the source
            for (auto const& sampler : storedSamplers) {
#if SAMPLER_DUPLICATION_TRACKING
                vkDestroySampler(device, sampler.sampler, nullptr);
#else
                vkDestroySampler(device, sampler, nullptr);
#endif
            }
        }
    } //namespace Sampler
} //namespace EWE