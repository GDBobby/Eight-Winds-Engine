#include "EWEngine/Systems/Rendering/Pipelines/Dimension2.h"

#include "EWEngine/Graphics/Model/Basic_Model.h"
#include "EWEngine/Graphics/Texture/Image_Manager.h"

#define RENDER_DEBUG false

namespace EWE {
	Dimension2* Dimension2::dimension2Ptr{ nullptr };
	Dimension2::Dimension2() {

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.offset = 0;
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.size = sizeof(Array2DPushConstantData);
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		{
			EWEDescriptorSetLayout::Builder eDSLBuilder{};
			eDSLBuilder.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
			eDSL = eDSLBuilder.Build();
		}
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = eDSL->GetDescriptorSetLayout();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &PL_array);

		pushConstantRange.size = sizeof(Single2DPushConstantData);
		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &PL_single);

		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);
		EWEPipeline::EnableAlphaBlending(pipelineConfig);
		pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<VertexUI>();
		pipelineConfig.attributeDescriptions = VertexUI::GetAttributeDescriptions();
		pipelineConfig.pipelineLayout = PL_array;

		VkPipelineCacheCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		EWE_VK(vkCreatePipelineCache, VK::Object->vkDevice, &createInfo, nullptr, &cache);

		pipelineConfig.cache = cache;
		{
			const std::string vertString = "texture2D_array.vert.spv";
			const std::string fragString = "texture2D_array.frag.spv";
#if EWE_DEBUG
			printf("before constructing with ui shaders - %s - %s\n", vertString.c_str(), fragString.c_str());
#endif
			pipe_array = Construct<EWEPipeline>({ vertString, fragString, pipelineConfig });
		}
		pipelineConfig.pipelineLayout = PL_single;
		pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		{
			const std::string vertString = "texture2D_singular.vert.spv";
			const std::string fragString = "texture2D_singular.frag.spv";
			pipe_single = Construct<EWEPipeline>({ vertString, fragString, pipelineConfig });
		}


#if EWE_DEBUG
		printf("after constructing with UI shaders\n");
#endif
#if DEBUG_NAMING
		pipe_array->SetDebugName("UI 2d pipeline");
		DebugNaming::SetObjectName(PL_array, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "2d pipe layout");
#endif
		CreateDefaultDesc();

		/*
		pushConstantRange.size = sizeof(NineUIPushConstantData);
		EWE_VK(vkCreatePipelineLayout, EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &PL_9);
		pipelineConfig.pipelineLayout = PL_9;

		vertString = "NineUI.vert.spv";
		fragString = "NineUI.frag.spv";
		pipe9 = ConstructSingular<EWEPipeline>(ewe_call_trace, vertString, fragString, pipelineConfig);
		*/
		model2D = Basic_Model::Quad2D();
		//nineUIModel = Basic_Model::NineUIQuad();
	}


	void Dimension2::Init() {
		assert(dimension2Ptr == nullptr && "initing dimension2 twice?");
		dimension2Ptr = new Dimension2();
#if CALL_TRACING
		ewe_alloc_mem_track(dimension2Ptr);
#endif
		//dimension2Ptr = ConstructSingular<Dimension2>(ewe_call_trace);

	}
	void Dimension2::Destruct() {
		EWE_VK(vkDestroyPipelineCache, VK::Object->vkDevice, dimension2Ptr->cache, nullptr);

		EWE_VK(vkDestroyPipelineLayout, VK::Object->vkDevice, dimension2Ptr->PL_array, nullptr);
		Deconstruct(dimension2Ptr->eDSL);
		Deconstruct(dimension2Ptr->model2D);
		Deconstruct(dimension2Ptr->pipe_array);
		Deconstruct(dimension2Ptr);
	}

	void Dimension2::BindArrayPipeline() {
#if RENDER_DEBUG
		printf("binding 2d pipeline in dimension 2 \n");
#endif

		dimension2Ptr->pipe_array->Bind();
		dimension2Ptr->model2D->Bind();
		dimension2Ptr->bindedTexture = IMAGE_INVALID;
	}
	void Dimension2::BindSingularPipeline() {
		dimension2Ptr->pipe_single->Bind();
		dimension2Ptr->model2D->Bind();
		dimension2Ptr->bindedTexture = IMAGE_INVALID;
	}
	void Dimension2::CreateDefaultDesc() {
		uiArrayID = Image_Manager::CreateUIImage();

		EWEDescriptorWriter descWriter(eDSL, DescriptorPool_Global);
#if DESCRIPTOR_IMAGE_IMPLICIT_SYNCHRONIZATION
		descWriter.WriteImage(uiArrayID);
#else
		descWriter.WriteImage(Image_Manager::GetDescriptorImageInfo(uiArrayID));
#endif
		defaultDesc = descWriter.Build();
#if DEBUG_NAMING
		DebugNaming::SetObjectName(defaultDesc, VK_OBJECT_TYPE_DESCRIPTOR_SET, "ui array desc");
#endif
	}

	void Dimension2::BindDefaultDesc() {
		EWE_VK(vkCmdBindDescriptorSets, VK::Object->GetFrameBuffer(),
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			dimension2Ptr->PL_array,
			0, 1,
			&dimension2Ptr->defaultDesc,
			0, nullptr
		);
	}
	void Dimension2::BindDescriptor(VkDescriptorSet desc) {
		EWE_VK(vkCmdBindDescriptorSets, VK::Object->GetFrameBuffer(),
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			dimension2Ptr->PL_single,
			0, 1,
			&desc,
			0, nullptr
		);
	}

	void Dimension2::PushAndDraw(Array2DPushConstantData& push) {
		//possibly do a check here, to ensure the pipeline and descriptors are properly binded
		//thats really just a feature to check bad programming, dont rely on the programmer being bad. (easy enough to debug)

		EWE_VK(vkCmdPushConstants, VK::Object->GetFrameBuffer(), dimension2Ptr->PL_array, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, static_cast<uint32_t>(sizeof(Array2DPushConstantData)), &push);
		dimension2Ptr->model2D->Draw();
	}

	void Dimension2::PushAndDraw(Single2DPushConstantData& push) {
		EWE_VK(vkCmdPushConstants, VK::Object->GetFrameBuffer(), dimension2Ptr->PL_single, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, static_cast<uint32_t>(sizeof(Single2DPushConstantData)), &push);
		dimension2Ptr->model2D->Draw();
	}
}