#include "GridPipe.h"

#include <EWEngine/Graphics/Texture/Image_Manager.h>

#include "PipelineEnum.h"

namespace EWE {
	GridPipe::GridPipe()
#if EWE_DEBUG
		: PipelineSystem{Pipe::Grid2d}
#endif
	{

		CreatePipeLayout();
		CreatePipeline();
	}

	void GridPipe::CreatePipeLayout() {
		pushStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushSize = sizeof(Grid2DPushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.offset = 0;
		pushConstantRange.stageFlags = pushStageFlags;
		pushConstantRange.size = pushSize;

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;


		EWEDescriptorSetLayout::Builder builder{};
		builder.AddGlobalBindings();
		builder.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		eDSL = builder.Build();


		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = eDSL->GetDescriptorSetLayout();
		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipeLayout);
	}
	void GridPipe::CreatePipeline() {
		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);
		EWEPipeline::EnableAlphaBlending(pipelineConfig);

		pipelineConfig.pipelineLayout = pipeLayout;
		pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<VertexGrid2D>();
		pipelineConfig.attributeDescriptions = VertexGrid2D::GetAttributeDescriptions();
		std::string vertString = "2dGrid.vert.spv";
		std::string fragString = "2dGrid.frag.spv";

		pipe = std::make_unique<EWEPipeline>(vertString, fragString, pipelineConfig);
	}

	void GridPipe::PushAndDraw(void* push) {

		EWE_VK(vkCmdPushConstants, VK::Object->GetFrameBuffer(), pipeLayout, pushStageFlags, 0, pushSize, push);

		EWE_VK(vkCmdDraw, VK::Object->GetFrameBuffer(), 6, 1, 0, 0);
	}
}