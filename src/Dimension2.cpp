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

		VkDescriptorSetLayout tempDSL = TextureDSLInfo::getSimpleDSL(VK_SHADER_STAGE_FRAGMENT_BIT)->getDescriptorSetLayout();
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &tempDSL;

		if (vkCreatePipelineLayout(EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &PL_2d) != VK_SUCCESS) {
			printf("failed to create 2d pipe layout\n");
			throw std::runtime_error("Failed to create pipe layout \n");
		}


		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);
		EWEPipeline::enableAlphaBlending(pipelineConfig);

		pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<VertexUI>();
		pipelineConfig.attributeDescriptions = VertexUI::getAttributeDescriptions();
		pipelineConfig.pipelineLayout = PL_2d;


		VkPipelineCacheCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		if (vkCreatePipelineCache(EWEDevice::GetVkDevice(), &createInfo, nullptr, &cache) != VK_SUCCESS) {
			// handle error
			printf("failed to create 2d cache \n");
			throw std::runtime_error("failed to create 2d pipeline cache");
		}

		pipelineConfig.cache = cache;

		std::string vertString = "2d.vert.spv";
		std::string fragString = "2d.frag.spv";
		pipe2d = ConstructSingular<EWEPipeline>(ewe_call_trace, vertString, fragString, pipelineConfig);

		pushConstantRange.size = sizeof(NineUIPushConstantData);
		if (vkCreatePipelineLayout(EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &PL_9) != VK_SUCCESS) {
			printf("failed to create 2d pipe layout\n");
			throw std::runtime_error("Failed to create pipe layout \n");
		}
		pipelineConfig.pipelineLayout = PL_9;

		vertString = "NineUI.vert.spv";
		fragString = "NineUI.frag.spv";
		pipe9 = ConstructSingular<EWEPipeline>(ewe_call_trace, vertString, fragString, pipelineConfig);

		model2D = Basic_Model::generate2DQuad();
		nineUIModel = Basic_Model::generateNineUIQuad();
	}


	void Dimension2::init() {
		if (dimension2Ptr != nullptr) {
			printf("initing twice??? \n");
			throw std::runtime_error("initing twice?");
			return;
		}
		dimension2Ptr = reinterpret_cast<Dimension2*>(ewe_alloc(sizeof(Dimension2), 1));
		new(dimension2Ptr) Dimension2();

	}
	void Dimension2::destruct() {
		vkDestroyPipelineCache(EWEDevice::GetVkDevice(), dimension2Ptr->cache, nullptr);

		vkDestroyPipelineLayout(EWEDevice::GetVkDevice(), dimension2Ptr->PL_2d, nullptr);
		vkDestroyPipelineLayout(EWEDevice::GetVkDevice(), dimension2Ptr->PL_9, nullptr);
		dimension2Ptr->model2D.reset();
		dimension2Ptr->nineUIModel.reset();

		dimension2Ptr->pipe2d->~EWEPipeline();
		dimension2Ptr->pipe9->~EWEPipeline();
		dimension2Ptr->~Dimension2();
		ewe_free(dimension2Ptr->pipe2d);
		ewe_free(dimension2Ptr->pipe9);
		ewe_free(dimension2Ptr);
	}

	void Dimension2::bind2D(VkCommandBuffer cmdBuffer, uint8_t frameIndex) {
#if RENDER_DEBUG
		printf("binding 2d pipeline in dimension 2 \n");
#endif

		dimension2Ptr->pipe2d->bind(cmdBuffer);
		dimension2Ptr->model2D->Bind(cmdBuffer);
		dimension2Ptr->bindedTexture = TEXTURE_UNBINDED_DESC;
		dimension2Ptr->frameIndex = frameIndex;
		dimension2Ptr->cmdBuffer = cmdBuffer;
	}
	void Dimension2::bindNineUI(VkCommandBuffer cmdBuffer, uint8_t frameIndex) {
#if RENDER_DEBUG
		printf("binding 9ui in dimension 2 \n");
#endif
		dimension2Ptr->pipe9->bind(cmdBuffer);
		dimension2Ptr->nineUIModel->Bind(cmdBuffer);
		dimension2Ptr->bindedTexture = TEXTURE_UNBINDED_DESC;
		dimension2Ptr->frameIndex = frameIndex;
		dimension2Ptr->cmdBuffer = cmdBuffer;
	}

	void Dimension2::bindTexture2DUI(TextureDesc texture) {
		if (texture != dimension2Ptr->bindedTexture) {
			vkCmdBindDescriptorSets(dimension2Ptr->cmdBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				dimension2Ptr->PL_2d,
				0, 1,
				&texture,
				0, nullptr
			);
			dimension2Ptr->bindedTexture = texture;
		}
	}
	void Dimension2::bindTexture2D(TextureDesc texture) {
		if (texture != dimension2Ptr->bindedTexture) {
			vkCmdBindDescriptorSets(dimension2Ptr->cmdBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				dimension2Ptr->PL_2d,
				0, 1,
				&texture,
				0, nullptr
			);
			dimension2Ptr->bindedTexture = texture;
		}
	}
	void Dimension2::bindTexture9(TextureDesc texture) {
		if (texture != dimension2Ptr->bindedTexture) {
			vkCmdBindDescriptorSets(dimension2Ptr->cmdBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				dimension2Ptr->PL_9,
				0, 1,
				&texture,
				0, nullptr
			);
			dimension2Ptr->bindedTexture = texture;
		}
	}

	void Dimension2::pushAndDraw(Simple2DPushConstantData& push) {
		//possibly do a check here, to ensure the pipeline and descriptors are properly binded
		//thats really just a feature to check bad programming, dont rely on the programmer being bad. (easy enough to debug)

		vkCmdPushConstants(dimension2Ptr->cmdBuffer, dimension2Ptr->PL_2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
		dimension2Ptr->model2D->Draw(dimension2Ptr->cmdBuffer);
	}
	void Dimension2::pushAndDraw(NineUIPushConstantData& push) {
		vkCmdPushConstants(dimension2Ptr->cmdBuffer, dimension2Ptr->PL_9, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
		dimension2Ptr->nineUIModel->Draw(dimension2Ptr->cmdBuffer);
	}
}