#include "VoxelTerrainPipe.h"

#include <EWGraphics/Model/Basic_Model.h>
#include <EWGraphics/Texture/Image_Manager.h>
#include <EWEngine/Graphics/PushConstants.h>

#include "PipeEnum.h"


namespace EWE {
	VoxelTerrainPipe::VoxelTerrainPipe()
#if EWE_DEBUG
		: PipelineSystem{ Pipe::SimpleTerrain } //i need to fix this up later
#endif
	{
		//createPipeline();

		CreatePipeLayout();
		CreatePipeline();
	}
	VoxelTerrainPipe::~VoxelTerrainPipe() {
		Deconstruct(pipe);
	}

	void VoxelTerrainPipe::CreatePipeLayout() {
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
		builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

		eDSL = builder.Build();

		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = eDSL->GetDescriptorSetLayout();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipeLayout);
	}
	void VoxelTerrainPipe::CreatePipeline() {
		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.pipelineLayout = pipeLayout;
		pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<VertexNT>();
		pipelineConfig.attributeDescriptions = VertexNT::GetAttributeDescriptions();

		ShaderTrackingStruct shaderStruct{};
		shaderStruct.shaderData[Shader::vert].filepath = "shaders/simple_terrain.vert.spv";
		shaderStruct.shaderData[Shader::frag].filepath = "shaders/simple_terrain.frag.spv";
		pipe = Construct<EWEPipeline>({ shaderStruct, pipelineConfig });
	}
}