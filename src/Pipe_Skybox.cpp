#include "EWEngine/Systems/Rendering/Pipelines/Pipe_Skybox.h"
#include "EWEngine/Graphics/DescriptorHandler.h"
#include "EWEngine/Graphics/Texture/Image.h"

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
		std::vector<VkDescriptorSetLayout> tempDSL = {
			DescriptorHandler::getDescSetLayout(LDSL_global),
			TextureDSLInfo::GetSimpleDSL(VK_SHADER_STAGE_FRAGMENT_BIT)->GetDescriptorSetLayout()
		};
			
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL.size());
		pipelineLayoutInfo.pSetLayouts = tempDSL.data();

		EWE_VK_ASSERT(vkCreatePipelineLayout(EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipeLayout));
	}
	void Pipe_Skybox::CreatePipeline() {
		CreatePipeLayout();

		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.pipelineLayout = pipeLayout;
		pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<SkyVertex>();
		pipelineConfig.attributeDescriptions = SkyVertex::GetAttributeDescriptions();

		std::string vertString = "skybox.vert.spv";
		std::string fragString = "skybox.frag.spv";

		pipe = std::make_unique<EWEPipeline>(vertString, fragString, pipelineConfig);
#if DEBUG_NAMING
		pipe->SetDebugName("skybox pipeline");
		DebugNaming::SetObjectName(EWEDevice::GetVkDevice(), pipeLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "skybox pipe layout");
#endif
	}
}