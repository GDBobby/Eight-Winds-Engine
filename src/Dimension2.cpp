#include "EWEngine/Systems/Rendering/Pipelines/Dimension2.h"

#include "EWEngine/Graphics/Model/Basic_Model.h"
#include "EWEngine/Graphics/Texture/Texture_Manager.h"

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
		pushConstantRange.size = sizeof(Simple2DPushConstantData);
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		VkDescriptorSetLayout tempDSL = TextureDSLInfo::GetSimpleDSL(VK_SHADER_STAGE_FRAGMENT_BIT)->GetDescriptorSetLayout();
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &tempDSL;

		EWE_VK(vkCreatePipelineLayout, EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &PL_2d);
		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);
		EWEPipeline::EnableAlphaBlending(pipelineConfig);
		pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<VertexUI>();
		pipelineConfig.attributeDescriptions = VertexUI::GetAttributeDescriptions();
		pipelineConfig.pipelineLayout = PL_2d;

		VkPipelineCacheCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		EWE_VK(vkCreatePipelineCache, EWEDevice::GetVkDevice(), &createInfo, nullptr, &cache);

		pipelineConfig.cache = cache;

		std::string vertString = "UI.vert.spv";
		std::string fragString = "UI.frag.spv";
		printf("before constructing with ui shaders\n");
		pipe2d = Construct<EWEPipeline>({ vertString, fragString, pipelineConfig });
		printf("after constructing with UI shaders\n");
#if DEBUG_NAMING
		pipe2d->SetDebugName("UI 2d pipeline");
		DebugNaming::SetObjectName(EWEDevice::GetVkDevice(), PL_2d, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "2d pipe layout");
#endif

		/*
		pushConstantRange.size = sizeof(NineUIPushConstantData);
		EWE_VK(vkCreatePipelineLayout, EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &PL_9);
		pipelineConfig.pipelineLayout = PL_9;

		vertString = "NineUI.vert.spv";
		fragString = "NineUI.frag.spv";
		pipe9 = ConstructSingular<EWEPipeline>(ewe_call_trace, vertString, fragString, pipelineConfig);
		*/
		model2D = Basic_Model::Quad2D(Queue::transfer);
		//nineUIModel = Basic_Model::NineUIQuad(Queue::transfer);
	}


	void Dimension2::Init() {
		assert(dimension2Ptr == nullptr && "initing dimension2 twice?");
		dimension2Ptr = new Dimension2();
		ewe_alloc_mem_track(dimension2Ptr);
		//dimension2Ptr = ConstructSingular<Dimension2>(ewe_call_trace);

	}
	void Dimension2::Destruct() {
		EWE_VK(vkDestroyPipelineCache, EWEDevice::GetVkDevice(), dimension2Ptr->cache, nullptr);

		EWE_VK(vkDestroyPipelineLayout, EWEDevice::GetVkDevice(), dimension2Ptr->PL_2d, nullptr);
		//vkDestroyPipelineLayout(EWEDevice::GetVkDevice(), dimension2Ptr->PL_9, nullptr);
		Deconstruct(dimension2Ptr->model2D);
		//delete dimension2Ptr->nineUIModel;
		Deconstruct(dimension2Ptr->pipe2d);
		//ewe_free(dimension2Ptr->pipe9);
		Deconstruct(dimension2Ptr);
	}

	void Dimension2::Bind2D(VkCommandBuffer cmdBuffer, uint8_t frameIndex) {
#if RENDER_DEBUG
		printf("binding 2d pipeline in dimension 2 \n");
#endif

		dimension2Ptr->pipe2d->Bind(cmdBuffer);
		dimension2Ptr->model2D->Bind(cmdBuffer);
		dimension2Ptr->bindedTexture = TEXTURE_UNBINDED_DESC;
		dimension2Ptr->frameIndex = frameIndex;
		dimension2Ptr->cmdBuffer = cmdBuffer;
	}
//	void Dimension2::BindNineUI(VkCommandBuffer cmdBuffer, uint8_t frameIndex) {
//#if RENDER_DEBUG
//		printf("binding 9ui in dimension 2 \n");
//#endif
//		dimension2Ptr->pipe9->bind(cmdBuffer);
//		dimension2Ptr->nineUIModel->Bind(cmdBuffer);
//		dimension2Ptr->bindedTexture = TEXTURE_UNBINDED_DESC;
//		dimension2Ptr->frameIndex = frameIndex;
//		dimension2Ptr->cmdBuffer = cmdBuffer;
//	}

	void Dimension2::BindTexture2DUI(TextureDesc texture) {
		if (texture != dimension2Ptr->bindedTexture) {
			EWE_VK(vkCmdBindDescriptorSets, dimension2Ptr->cmdBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				dimension2Ptr->PL_2d,
				0, 1,
				&texture,
				0, nullptr
			);
			dimension2Ptr->bindedTexture = texture;
		}
	}
	void Dimension2::BindTexture2D(TextureDesc texture) {
		if (texture != dimension2Ptr->bindedTexture) {
			EWE_VK(vkCmdBindDescriptorSets, dimension2Ptr->cmdBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				dimension2Ptr->PL_2d,
				0, 1,
				&texture,
				0, nullptr
			);
			dimension2Ptr->bindedTexture = texture;
		}
	}
	//void Dimension2::BindTexture9(TextureDesc texture) {
	//	if (texture != dimension2Ptr->bindedTexture) {
	//		vkCmdBindDescriptorSets(dimension2Ptr->cmdBuffer,
	//			VK_PIPELINE_BIND_POINT_GRAPHICS,
	//			dimension2Ptr->PL_9,
	//			0, 1,
	//			&texture,
	//			0, nullptr
	//		);
	//		dimension2Ptr->bindedTexture = texture;
	//	}
	//}

	void Dimension2::PushAndDraw(Simple2DPushConstantData& push) {
		//possibly do a check here, to ensure the pipeline and descriptors are properly binded
		//thats really just a feature to check bad programming, dont rely on the programmer being bad. (easy enough to debug)

		EWE_VK(vkCmdPushConstants, dimension2Ptr->cmdBuffer, dimension2Ptr->PL_2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
		dimension2Ptr->model2D->Draw(dimension2Ptr->cmdBuffer);
	}
	//void Dimension2::PushAndDraw(NineUIPushConstantData& push) {
	//	vkCmdPushConstants(dimension2Ptr->cmdBuffer, dimension2Ptr->PL_9, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
	//	dimension2Ptr->nineUIModel->Draw(dimension2Ptr->cmdBuffer);
	//}
}