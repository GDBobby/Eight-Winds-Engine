#include "GeneratedGrassPipe.h"

//#include "PipelineEnum.h"

#include <EWEngine/Graphics/Model/Basic_Model.h>
#include <EWEngine/Graphics/Texture/Image_Manager.h>
#include <EWEngine/Graphics/PushConstants.h>

#include "PipeEnum.h"

#define INCLUDE_TASK_SHADER true

namespace EWE {
	GeneratedGrassPipe::GeneratedGrassPipe()
#if EWE_DEBUG
		: PipelineSystem{ Pipe::GenGrass } //i need to fix this up later
#endif
	{
		//createPipeline();

		CreatePipeLayout();
		CreatePipeline();
	}

	void GeneratedGrassPipe::CreatePipeLayout() {
		pushStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushSize = sizeof(PushTileConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		//VkPushConstantRange pushConstantRange{};
		//pushConstantRange.offset = 0;
		//pushConstantRange.stageFlags = pushStageFlags;
		//pushConstantRange.size = pushSize;

		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		EWEDescriptorSetLayout::Builder builder;
#if INCLUDE_TASK_SHADER
		builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT);
		builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT);
		//builder.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT);
#else
		builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT);
		builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT);
#endif
		//builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
		//builder.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		//builder.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

		eDSL = builder.Build();

		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = eDSL->GetDescriptorSetLayout();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipeLayout);
	}
	void GeneratedGrassPipe::CreatePipeline() {
		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);


		//pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
		Pipeline_Helper_Functions::CreateShaderModule("grass.task.spv", &pipelineConfig.taskShaderModule);
		Pipeline_Helper_Functions::CreateShaderModule("grass.mesh.spv", &pipelineConfig.meshShaderModule);

		pipelineConfig.pipelineLayout = pipeLayout;

		const std::string fragString = "grass.frag.spv";

		pipe = std::make_unique<EWEPipeline>(fragString, pipelineConfig);
	}
}