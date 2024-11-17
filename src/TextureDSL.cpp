#include "EWEngine/Graphics/Texture/TextureDSL.h"


namespace EWE {
    std::unordered_map<TextureDSLInfo, EWEDescriptorSetLayout*> TextureDSLInfo::descSetLayouts;
    void TextureDSLInfo::SetStageTextureCount(VkShaderStageFlags stageFlag, uint8_t textureCount) {
        switch (stageFlag) {
        case VK_SHADER_STAGE_VERTEX_BIT: {
            stageCounts[0] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: {
            stageCounts[1] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: {
            stageCounts[2] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_GEOMETRY_BIT: {
            stageCounts[3] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_FRAGMENT_BIT: {
            stageCounts[4] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_COMPUTE_BIT: {
            stageCounts[5] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_ALL_GRAPHICS: {
            //_ALL_GRAPHICS and _ALL feel like noob traps, but ill put them in anyways
            stageCounts[6] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_ALL: {
            stageCounts[7] = textureCount;
            break;
        }
        case VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT: {
            stageCounts[8] = textureCount;
            break;
        }
        default: {
            assert(false && "invalid shader stage flag for textures");
            break;
        }
    }
}


    EWEDescriptorSetLayout* TextureDSLInfo::BuildDSL() {
        uint32_t currentBinding = 0;
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //this is 1
        EWEDescriptorSetLayout::Builder dslBuilder{};
        for (uint8_t j = 0; j < 6; j++) {

            for (uint8_t i = 0; i < stageCounts[j]; i++) {
                dslBuilder.AddBinding(currentBinding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stageFlags);
                currentBinding++;
            }
            stageFlags <<= 1;
        }
        for (uint8_t i = 0; i < stageCounts[6]; i++) {
            dslBuilder.AddBinding(currentBinding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS);
            currentBinding++;
        }

        for (uint8_t i = 0; i < stageCounts[7]; i++) {
            dslBuilder.AddBinding(currentBinding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL);
            currentBinding++;
        }

        for (uint8_t i = 0; i < stageCounts[8]; i++) {
            dslBuilder.AddBinding(currentBinding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
            currentBinding++;
        }
        return dslBuilder.Build();
}

    EWEDescriptorSetLayout* TextureDSLInfo::GetSimpleDSL(VkShaderStageFlags stageFlag) {
        TextureDSLInfo dslInfo{};
        dslInfo.SetStageTextureCount(stageFlag, 1);

        auto dslIter = descSetLayouts.find(dslInfo);
        if (dslIter != descSetLayouts.end()) {
            return dslIter->second;
        }

        EWEDescriptorSetLayout::Builder dslBuilder{};
        dslBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stageFlag);
        return descSetLayouts.emplace(dslInfo, dslBuilder.Build()).first->second;
    }

    EWEDescriptorSetLayout* TextureDSLInfo::GetDescSetLayout() {
        auto dslIter = descSetLayouts.find(*this);
        if (dslIter != descSetLayouts.end()) {
            return dslIter->second;
        }

#if EWE_DEBUG
        auto emplaceRet = descSetLayouts.try_emplace(*this, BuildDSL());
        assert(emplaceRet.second && "failed to create dynamic desc set layout");
        return emplaceRet.first->second;
#else
        return descSetLayouts.try_emplace(*this, BuildDSL()).first->second;
#endif
    }

}