#include "EWEngine/Systems/Rendering/Pipelines/Pipe_SimpleTextured.h"
#include "EWEngine/Graphics/Texture/Image.h"

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

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.offset = 0;
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.size = sizeof(SimplePushConstantData);
		pushStageFlags = pushConstantRange.stageFlags;
		pushSize = pushConstantRange.size;

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		//pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		std::vector<VkDescriptorSetLayout> tempDSL = {
			DescriptorHandler::GetDescSetLayout(LDSL_global),
			TextureDSLInfo::GetSimpleDSL(VK_SHADER_STAGE_FRAGMENT_BIT)->GetDescriptorSetLayout()
		};

		//printf("tempDSL size : %d \n", tempDSL->size());
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL.size());
		pipelineLayoutInfo.pSetLayouts = tempDSL.data();

		EWE_VK(vkCreatePipelineLayout, EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipeLayout);
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
		DebugNaming::SetObjectName(EWEDevice::GetVkDevice(), pipeLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "simple textured pipe layout");
#endif
		//memory leak for now, return to std::unique_ptr;
	}
}