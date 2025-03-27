#include "EWEngine/Systems/Rendering/Pipelines/Pipe_Skybox.h"
#include "EWEngine/Graphics/DescriptorHandler.h"

namespace EWE {
	Pipe_Skybox::Pipe_Skybox()
#if EWE_DEBUG
		: PipelineSystem{ Pipe::skybox } {
#else
	{
#endif

		CreatePipeline();
	}


	void Pipe_Skybox::CreatePipeLayout() {

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		EWEDescriptorSetLayout::Builder builder{};
		builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		builder.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		eDSL = builder.Build();
			
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = eDSL->GetDescriptorSetLayout();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipeLayout);
	}
	void Pipe_Skybox::CreatePipeline() {
		CreatePipeLayout();

		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.pipelineLayout = pipeLayout;
		pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<SkyVertex>();
		pipelineConfig.attributeDescriptions = SkyVertex::GetAttributeDescriptions();

		std::string vertString = "skybox.vert.spv";
		std::string fragString = "skybox.frag.spv";

		pipe = std::make_unique<EWEPipeline>(vertString, fragString, pipelineConfig);
#if DEBUG_NAMING
		pipe->SetDebugName("skybox pipeline");
		DebugNaming::SetObjectName(pipeLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "skybox pipe layout");
#endif
	}
}