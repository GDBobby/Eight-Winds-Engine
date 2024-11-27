#include "BackgroundPipe.h"

#include "PipelineEnum.h"

#include <EWEngine/Graphics/Model/Basic_Model.h>
#include <EWEngine/Graphics/Texture/Image_Manager.h>

#include <functional>
#include <typeinfo>
#include <unordered_map>

namespace EWE {
	BackgroundPipe::BackgroundPipe()
#if EWE_DEBUG
		: PipelineSystem{Pipe::background} //i need to fix this up later
#endif
	{
		//createPipeline();

		CreatePipeLayout();
		CreatePipeline();
	}

	void BackgroundPipe::CreatePipeLayout() {
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

		//vertexIndexBufferLayout = EWEDescriptorSetLayout::Builder()
		//	.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		//	.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		//	.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		//	.Build();


		//std::vector<VkDescriptorSetLayout> tempDSL = {
		//	DescriptorHandler::getDescSetLayout(LDSL_global, device),
		//	vertexIndexBufferLayout->getDescriptorSetLayout(),
		//	TextureDSLInfo::getSimpleDSL(device, VK_SHADER_STAGE_FRAGMENT_BIT)->getDescriptorSetLayout()
		//};
		EWEDescriptorSetLayout::Builder builder{};
		builder.AddGlobalBindings()
			
			.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);


		eDSL = builder.Build();

		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = eDSL->GetDescriptorSetLayout();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipeLayout);
	}
	void BackgroundPipe::CreatePipeline() {
		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);
		EWEPipeline::Enable2DConfig(pipelineConfig);
		EWEPipeline::EnableAlphaBlending(pipelineConfig);

		pipelineConfig.pipelineLayout = pipeLayout;
		//pipelineConfig.bindingDescriptions = EffectVertex::getBindingDescriptions();
		//pipelineConfig.bindingDescriptions = TileVertex::getBindingDescriptions();
		//pipelineConfig.attributeDescriptions = TileVertex::getAttributeDescriptions();
		std::string vertString = "tileInstancing.vert.spv";
		std::string fragString = "tileInstancing.frag.spv";

		pipe = std::make_unique<EWEPipeline>(vertString, fragString, pipelineConfig);
	}

	void BackgroundPipe::DrawInstanced(EWEModel* model) {
		printf("just draw on location without calling this function \n");
		assert(false);
		//model->BindAndDrawInstanceNoIndex(cmdBuf);
		//vkCmdDraw(commandBuffer, 6, instanceCount, 0, 0);
	}
}