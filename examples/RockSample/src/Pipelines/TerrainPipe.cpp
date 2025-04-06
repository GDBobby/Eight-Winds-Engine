#include "TerrainPipe.h"

//#include "PipelineEnum.h"

#include <EWEngine/Graphics/Model/Basic_Model.h>
#include <EWEngine/Graphics/Texture/Image_Manager.h>
#include <EWEngine/Graphics/PushConstants.h>


namespace EWE {
	TerrainPipe::TerrainPipe()
#if EWE_DEBUG
		: PipelineSystem{Pipe::ENGINE_MAX_COUNT} //i need to fix this up later
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

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		EWEDescriptorSetLayout::Builder builder;
			builder
			.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);


		eDSL = builder.Build();

		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = eDSL->GetDescriptorSetLayout();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipeLayout);
	}
	void TerrainPipe::CreatePipeline() {
		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.inputAssemblyInfo.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        pipelineConfig.inputAssemblyInfo.flags = 0;
        pipelineConfig.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        pipelineConfig.hasTesselation = true;
        pipelineConfig.tessCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        pipelineConfig.tessCreateInfo.flags = 0;
        pipelineConfig.tessCreateInfo.patchControlPoints = 4;
        pipelineConfig.tessCreateInfo.pNext = nullptr;

		pipelineConfig.pipelineLayout = pipeLayout;
		//pipelineConfig.bindingDescriptions = EffectVertex::getBindingDescriptions();
		//pipelineConfig.bindingDescriptions = TileVertex::getBindingDescriptions();
		//pipelineConfig.attributeDescriptions = TileVertex::getAttributeDescriptions();
		std::string vertString = "terrain.vert.spv";
		std::string fragString = "terrain.frag.spv";

        std::string tescString = "terrain.tesc.spv";
        std::string teseString = "terrain.tese.spv";
        Pipeline_Helper_Functions::CreateShaderModule(tescString, &pipelineConfig.tessControlModule);
        Pipeline_Helper_Functions::CreateShaderModule(teseString, &pipelineConfig.tessControlModule);

		pipe = std::make_unique<EWEPipeline>(vertString, fragString, pipelineConfig);
	}
}