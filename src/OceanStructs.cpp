#include "EWEngine/Systems/Ocean/OceanStructs.h"

#include "EWEngine/Graphics/DescriptorHandler.h"

namespace EWE {
    namespace Ocean {
        InitialFrequencySpectrumGPUData::InitialFrequencySpectrumGPUData() {
            CreatePipeLayout();
            CreatePipeline();
            CreateBuffers();
        }
        InitialFrequencySpectrumGPUData::~InitialFrequencySpectrumGPUData() {
			vkDestroyPipeline(EWEDevice::GetVkDevice(), pipeline, nullptr);
			vkDestroyPipelineLayout(EWEDevice::GetVkDevice(), pipeLayout, nullptr);
			vkDestroyShaderModule(EWEDevice::GetVkDevice(), shaderModule, nullptr);
			ewe_free(jonswapBuffer);

            ewe_free(eweDSL);
		}

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
            dslBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1);
            dslBuilder.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
            eweDSL = dslBuilder.build();

            VkDescriptorSetLayout dsLayout = eweDSL->getDescriptorSetLayout();

            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &dsLayout;

            EWE_VK_ASSERT(vkCreatePipelineLayout(EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipeLayout));
        }
        void InitialFrequencySpectrumGPUData::CreatePipeline() {
            VkComputePipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipelineInfo.layout = pipeLayout;
            Pipeline_Helper_Functions::createShaderModule("InitialFrequencySpectrum.comp.spv", &shaderModule);
            VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
            computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            computeShaderStageInfo.module = shaderModule;
            computeShaderStageInfo.pName = "main";
            pipelineInfo.stage = computeShaderStageInfo;
            vkCreateComputePipelines(EWEDevice::GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
        }
        void InitialFrequencySpectrumGPUData::CreateBuffers() {
            jonswapBuffer = EWEBuffer::createAndInitBuffer(&jonswapParams, sizeof(JONSWAP_Parameters), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        }
        void InitialFrequencySpectrumGPUData::CreateDescriptorSet(VkDescriptorImageInfo* descImageInfo) {

            EWEDescriptorWriter descWriter{eweDSL, DescriptorPool_Global};
            descWriter.writeImage(0, descImageInfo);
            descWriter.writeBuffer(1, jonswapBuffer->descriptorInfo());
            descriptorSet[0] = descWriter.build();
            descriptorSet[1] = descWriter.build();
        }
    } //namespace Ocean
} //namespace EWE