#include "EWEngine/Systems/Ocean/OceanStructs.h"

#include "EWEngine/Graphics/DescriptorHandler.h"

namespace EWE {
    namespace Ocean {
        void InitialFrequencySpectrumGPUData::CreatePipeLayout() {
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(IntialFrequencySpectrumPushData);

            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
            pipelineLayoutInfo.pushConstantRangeCount = 1;

            EWEDescriptorSetLayout::Builder dslBuilder{};
            dslBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
            dslBuilder.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1);
            auto const tempDSL = dslBuilder.build();

            VkDescriptorSetLayout dsLayout = tempDSL->getDescriptorSetLayout();

            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &dsLayout;

            EWE_VK_ASSERT(vkCreatePipelineLayout(EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipeLayout));

            ewe_free(tempDSL);
        }
        void CreatePipeline() {
            EWEPipeline::PipelineConfigInfo pipelineConfig{};
        }
    } //namespace Ocean
} //namespace EWE