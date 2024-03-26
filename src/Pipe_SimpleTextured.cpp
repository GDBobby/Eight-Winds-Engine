#include "EWEngine/Systems/Rendering/Pipelines/Pipe_SimpleTextured.h"
#include "EWEngine/Graphics/Texture/Texture.h"

namespace EWE {
	Pipe_SimpleTextured::Pipe_SimpleTextured()
#ifdef _DEBUG
		: PipelineSystem{ Pipe_textured } {
#else
	{
#endif
		createPipeline();
	}

	void Pipe_SimpleTextured::createPipeLayout() {
		
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
			DescriptorHandler::getDescSetLayout(LDSL_global),
			TextureDSLInfo::getSimpleDSL(VK_SHADER_STAGE_FRAGMENT_BIT)->getDescriptorSetLayout()
		};

		//printf("tempDSL size : %d \n", tempDSL->size());
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL.size());
		pipelineLayoutInfo.pSetLayouts = tempDSL.data();

		if (vkCreatePipelineLayout(EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipeLayout) != VK_SUCCESS) {
			printf("failed to create simple textured pipe layout \n");
			throw std::runtime_error("Failed to create pipe layout \n");
		}
	}

	void Pipe_SimpleTextured::createPipeline() {
		createPipeLayout();

		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.pipelineLayout = pipeLayout;
		pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<VertexNT>();
		pipelineConfig.attributeDescriptions = VertexNT::getAttributeDescriptions();

		std::string vertString = "texture_shader.vert.spv";
		std::string fragString = "texture_shader.frag.spv";

		pipe = std::make_unique<EWEPipeline>(vertString, fragString, pipelineConfig);
	}
}