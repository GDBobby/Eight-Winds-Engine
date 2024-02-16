#include "EWEngine/Systems/Rendering/Pipelines/Pipe_Skybox.h"
#include "EWEngine/Graphics/DescriptorHandler.h"
#include "EWEngine/Graphics/Texture/Texture.h"

namespace EWE {
	Pipe_Skybox::Pipe_Skybox(EWEDevice& device)
#ifdef _DEBUG
		: PipelineSystem{ Pipe_skybox } {
#else
	{
#endif

		createPipeline(device);
	}

	void Pipe_Skybox::createPipeLayout(EWEDevice& device) {

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		std::vector<VkDescriptorSetLayout> tempDSL = {
			DescriptorHandler::getDescSetLayout(LDSL_global, device),
			TextureDSLInfo::getSimpleDSL(device, VK_SHADER_STAGE_FRAGMENT_BIT)->getDescriptorSetLayout()
		};
			
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL.size());
		pipelineLayoutInfo.pSetLayouts = tempDSL.data();

		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipeLayout) != VK_SUCCESS) {
			printf("failed to create background pipe layout \n");
			throw std::runtime_error("Failed to create pipe layout \n");
		}
	}
	void Pipe_Skybox::createPipeline(EWEDevice& device) {
		createPipeLayout(device);

		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.pipelineLayout = pipeLayout;
		pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<skyVertex>();
		pipelineConfig.attributeDescriptions = skyVertex::getAttributeDescriptions();

		std::string vertString = "skybox.vert.spv";
		std::string fragString = "skybox.frag.spv";

		pipe = std::make_unique<EWEPipeline>(device, vertString, fragString, pipelineConfig);
	}
}