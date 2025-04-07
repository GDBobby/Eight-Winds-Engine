#include "PerlinPipe.h"
#include "EWEngine/Graphics/PushConstants.h"

#include "PipeEnum.h"

namespace EWE {
	PerlinPipe::PerlinPipe()
#if EWE_DEBUG
		: PipelineSystem{ Pipe::Perlin} {
#else
			{
#endif
				CreatePipeline();
			}

	void PerlinPipe::CreatePipeLayout() {

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
		eDSL = builder.Build();

		//printf("tempDSL size : %d \n", tempDSL->size());
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = eDSL->GetDescriptorSetLayout();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipeLayout);
	}

	void PerlinPipe::CreatePipeline() {
		CreatePipeLayout();

		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.pipelineLayout = pipeLayout;
		pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<VertexNT>();
		pipelineConfig.attributeDescriptions = VertexNT::GetAttributeDescriptions();

		std::string vertString = "texture_shader.vert.spv";
		std::string fragString = "perlin.frag.spv";

		//EWEPipeline* tempPtr = new EWEPipeline(vertString, fragString, pipelineConfig);

		pipe = std::make_unique<EWEPipeline>(vertString, fragString, pipelineConfig);
#if DEBUG_NAMING
		pipe->SetDebugName("perlin pipeline");
		DebugNaming::SetObjectName(pipeLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "perlin pipe layout");
#endif
		//memory leak for now, return to std::unique_ptr;
	}
	}