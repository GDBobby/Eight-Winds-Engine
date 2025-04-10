#include "TerrainPipe.h"

//#include "PipelineEnum.h"

#include <EWEngine/Graphics/Model/Basic_Model.h>
#include <EWEngine/Graphics/Texture/Image_Manager.h>
#include <EWEngine/Graphics/PushConstants.h>

#include "PipeEnum.h"


namespace EWE {
	TerrainPipe::TerrainPipe()
#if EWE_DEBUG
		: PipelineSystem{Pipe::Terrain} //i need to fix this up later
#endif
	{
		//createPipeline();

		CreatePipeLayout();
		CreatePipeline();
	}

	void TerrainPipe::CreatePipeLayout() {
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

		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		EWEDescriptorSetLayout::Builder builder;
		builder.AddGlobalBindings();
		builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

		eDSL = builder.Build();

		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = eDSL->GetDescriptorSetLayout();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipeLayout);
	}
	void TerrainPipe::CreatePipeline() {
		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.pipelineLayout = pipeLayout;
		pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<VertexNT>();
		pipelineConfig.attributeDescriptions = VertexNT::GetAttributeDescriptions();

        pipelineConfig.inputAssemblyInfo.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        pipelineConfig.inputAssemblyInfo.flags = 0;
        pipelineConfig.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        pipelineConfig.hasTesselation = true;
        pipelineConfig.tessCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        pipelineConfig.tessCreateInfo.flags = 0;
        pipelineConfig.tessCreateInfo.patchControlPoints = 4;
        pipelineConfig.tessCreateInfo.pNext = nullptr;

		const std::string vertString = "terrain.vert.spv";
		const std::string fragString = "terrain.frag.spv";
        const std::string tescString = "terrain.tesc.spv";
        const std::string teseString = "terrain.tese.spv";
        Pipeline_Helper_Functions::CreateShaderModule(tescString, &pipelineConfig.tessControlModule);
        Pipeline_Helper_Functions::CreateShaderModule(teseString, &pipelineConfig.tessEvaluationModule);

		pipe = std::make_unique<EWEPipeline>(vertString, fragString, pipelineConfig);
	}
}