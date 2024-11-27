#include "BackgroundPipe.h"

#include <EWEngine/Graphics/Model/Basic_Model.h>
#include <EWEngine/Graphics/Texture/Texture_Manager.h>

#include <functional>
#include <typeinfo>
#include <unordered_map>

namespace EWE {
	BackgroundPipe::BackgroundPipe(EWEDevice& device) {
		//createPipeline();

		createPipeLayout(device);
		createPipeline(device);
	}

	void BackgroundPipe::createPipeLayout(EWEDevice& device) {
		pushStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		//pushSize = sizeof(ModelPushData);
		pushSize = sizeof(PushTileConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.offset = 0;
		pushConstantRange.stageFlags = pushStageFlags;
		pushConstantRange.size = pushSize;

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		//pipelineLayoutInfo.pushConstantRangeCount = 0;
		//pipelineLayoutInfo.pPushConstantRanges = nullptr;

		vertexIndexBufferLayout = EWEDescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();


		std::vector<VkDescriptorSetLayout> tempDSL = {
			DescriptorHandler::getDescSetLayout(LDSL_global, device),
			vertexIndexBufferLayout->getDescriptorSetLayout(),
			TextureDSLInfo::getSimpleDSL(device, VK_SHADER_STAGE_FRAGMENT_BIT)->getDescriptorSetLayout()
		};

		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL.size());
		pipelineLayoutInfo.pSetLayouts = tempDSL.data();

		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipeLayout) != VK_SUCCESS) {
			printf("failed to create background pipe layout \n");
			throw std::runtime_error("Failed to create pipe layout \n");
		}
	}
	void BackgroundPipe::createPipeline(EWEDevice& device) {
		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);
		EWEPipeline::enable2DConfig(pipelineConfig);
		EWEPipeline::enableAlphaBlending(pipelineConfig);

		pipelineConfig.pipelineLayout = pipeLayout;
		//pipelineConfig.bindingDescriptions = EffectVertex::getBindingDescriptions();
		//pipelineConfig.bindingDescriptions = TileVertex::getBindingDescriptions();
		//pipelineConfig.attributeDescriptions = TileVertex::getAttributeDescriptions();
		std::string vertString = "tileInstancing.vert.spv";
		std::string fragString = "tileInstancing.frag.spv";

		pipe = std::make_unique<EWEPipeline>(device, vertString, fragString, pipelineConfig);
	}

	void BackgroundPipe::drawInstanced(EWEModel* model) {
		printf("incorrect clal \n");
		throw std::runtime_error("just draw on location without calling this function");
		//model->BindAndDrawInstanceNoIndex(cmdBuf);
		//vkCmdDraw(commandBuffer, 6, instanceCount, 0, 0);
	}
}