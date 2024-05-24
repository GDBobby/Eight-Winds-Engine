#pragma once
#include "vulkan/vulkan.h"


namespace EWE {
    namespace Sampler {
        VkSampler GetSampler(VkSamplerCreateInfo const& samplerInfo);
        void RemoveSampler(VkSampler sampler);
        void Initialize(VkDevice device);
        void Deconstruct();
    } //namespace Sampler
}
