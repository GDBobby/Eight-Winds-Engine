#include "EWEngine/Systems/Rendering/Pipelines/Pipe_SimpleTextured.h"

namespace EWE {
	Pipe_SimpleTextured::Pipe_SimpleTextured()
#if EWE_DEBUG
		: PipelineSystem{ Pipe::textured } {
#else
	{
#endif
		CreatePipeline();
	}

	void Pipe_SimpleTextured::CreatePipeLayout() {
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.offset = 0;
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.size = sizeof(ModelAndNormalPushData);
		pushStageFlags = pushConstantRange.stageFlags;
		pushSize = pushConstantRange.size;

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		//pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		
		EWEDescriptorSetLayout::Builder builder{};
		builder.AddGlobalBindings();
		builder.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		eDSL = builder.Build();

		//printf("tempDSL size : %d \n", tempDSL->size());
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = eDSL->GetDescriptorSetLayout();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipeLayout);
	}

	void Pipe_SimpleTextured::CreatePipeline() {
		CreatePipeLayout();

		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.pipelineLayout = pipeLayout;
		pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<VertexNT>();
		pipelineConfig.attributeDescriptions = VertexNT::GetAttributeDescriptions();

		std::string vertString = "texture_shader.vert.spv";
		std::string fragString = "texture_shader.frag.spv";

		//EWEPipeline* tempPtr = new EWEPipeline(vertString, fragString, pipelineConfig);

		pipe = std::make_unique<EWEPipeline>(vertString, fragString, pipelineConfig);
#if DEBUG_NAMING
		pipe->SetDebugName("simple textured pipeline");
		DebugNaming::SetObjectName(pipeLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "simple textured pipe layout");
#endif
		//memory leak for now, return to std::unique_ptr;
	}
}