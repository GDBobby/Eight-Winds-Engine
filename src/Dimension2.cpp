#include "EWEngine/graphics/Dimension2/Dimension2.h"
#include "EWEngine/graphics/EWE_Texture.h"
#include "EWEngine/graphics/model/EWE_Basic_Model.h"

#define RENDER_DEBUG false

namespace EWE {
	Dimension2* Dimension2::dimension2Ptr{ nullptr };
	Dimension2::Dimension2(EWEDevice& device, VkPipelineRenderingCreateInfo const& pipeRenderInfo) {

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.offset = 0;
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.size = sizeof(Simple2DPushConstantData);
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		auto tempDSL = EWETexture::getSimpleDescriptorSetLayout();
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &tempDSL;

		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &PL_2d) != VK_SUCCESS) {
			printf("failed to create 2d pipe layout\n");
			throw std::runtime_error("Failed to create pipe layout \n");
		}


		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);
		EWEPipeline::enableAlphaBlending(pipelineConfig);

		pipelineConfig.pipelineRenderingInfo = pipeRenderInfo;
		pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<VertexUI>();
		pipelineConfig.attributeDescriptions = VertexUI::getAttributeDescriptions();
		pipelineConfig.pipelineLayout = PL_2d;


		VkPipelineCacheCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		if (vkCreatePipelineCache(device.device(), &createInfo, nullptr, &cache) != VK_SUCCESS) {
			// handle error
			printf("failed to create 2d cache \n");
			throw std::runtime_error("failed to create 2d pipeline cache");
		}

		pipelineConfig.cache = cache;

		std::string vertString = "2d.vert.spv";
		std::string fragString = "2d.frag.spv";
		pipe2d = std::make_unique<EWEPipeline>(device, vertString, fragString, pipelineConfig);

		pushConstantRange.size = sizeof(NineUIPushConstantData);
		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &PL_9) != VK_SUCCESS) {
			printf("failed to create 2d pipe layout\n");
			throw std::runtime_error("Failed to create pipe layout \n");
		}
		pipelineConfig.pipelineLayout = PL_9;

		vertString = "NineUI.vert.spv";
		fragString = "NineUI.frag.spv";
		pipe9 = std::make_unique<EWEPipeline>(device, vertString, fragString, pipelineConfig);

		model2D = Basic_Model::generate2DQuad(device);
		nineUIModel = Basic_Model::generateNineUIQuad(device);
	}


	void Dimension2::init(EWEDevice& device, VkPipelineRenderingCreateInfo const& pipeRenderInfo) {
		if (dimension2Ptr != nullptr) {
			printf("initing twice??? \n");
			throw std::runtime_error("initing twice?");
			return;
		}
		dimension2Ptr = new Dimension2(device, pipeRenderInfo);
	}
	void Dimension2::destruct(EWEDevice& device) {
		vkDestroyPipelineCache(device.device(), dimension2Ptr->cache, nullptr);

		vkDestroyPipelineLayout(device.device(), dimension2Ptr->PL_2d, nullptr);
		vkDestroyPipelineLayout(device.device(), dimension2Ptr->PL_9, nullptr);
		dimension2Ptr->pipe2d.reset();
		dimension2Ptr->pipe9.reset();
		dimension2Ptr->model2D.reset();
		dimension2Ptr->nineUIModel.reset();
		delete dimension2Ptr;
	}

	void Dimension2::bind2D(VkCommandBuffer cmdBuffer, uint8_t frameIndex) {
#if RENDER_DEBUG
		printf("binding 2d pipeline in dimension 2 \n");
#endif

		dimension2Ptr->pipe2d->bind(cmdBuffer);
		dimension2Ptr->model2D->bind(cmdBuffer);
		dimension2Ptr->bindedTexture = TEXTURE_UNBINDED;
		dimension2Ptr->frameIndex = frameIndex;
		dimension2Ptr->cmdBuffer = cmdBuffer;
	}
	void Dimension2::bindNineUI(VkCommandBuffer cmdBuffer, uint8_t frameIndex) {
#if RENDER_DEBUG
		printf("binding 9ui in dimension 2 \n");
#endif
		dimension2Ptr->pipe9->bind(cmdBuffer);
		dimension2Ptr->nineUIModel->bind(cmdBuffer);
		dimension2Ptr->bindedTexture = TEXTURE_UNBINDED;
		dimension2Ptr->frameIndex = frameIndex;
		dimension2Ptr->cmdBuffer = cmdBuffer;
	}

	void Dimension2::bindTexture2DUI(TextureID textureID) {
		if (textureID != dimension2Ptr->bindedTexture) {
			vkCmdBindDescriptorSets(dimension2Ptr->cmdBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				dimension2Ptr->PL_2d,
				0, 1,
				EWETexture::getUIDescriptorSets(textureID, dimension2Ptr->frameIndex),
				0, nullptr
			);
			dimension2Ptr->bindedTexture = textureID;
		}
	}
	void Dimension2::bindTexture2D(TextureID textureID) {
		if (textureID != dimension2Ptr->bindedTexture) {
			vkCmdBindDescriptorSets(dimension2Ptr->cmdBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				dimension2Ptr->PL_2d,
				0, 1,
				EWETexture::getDescriptorSets(textureID, dimension2Ptr->frameIndex),
				0, nullptr
			);
			dimension2Ptr->bindedTexture = textureID;
		}
	}
	void Dimension2::bindTexture9(TextureID textureID) {
		if (textureID != dimension2Ptr->bindedTexture) {
			vkCmdBindDescriptorSets(dimension2Ptr->cmdBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				dimension2Ptr->PL_9,
				0, 1,
				EWETexture::getUIDescriptorSets(textureID, dimension2Ptr->frameIndex),
				0, nullptr
			);
			dimension2Ptr->bindedTexture = textureID;
		}
	}

	void Dimension2::pushAndDraw(Simple2DPushConstantData& push) {
		//possibly do a check here, to ensure the pipeline and descriptors are properly binded
		//thats really just a feature to check bad programming, dont rely on the programmer being bad. (easy enough to debug)

		vkCmdPushConstants(dimension2Ptr->cmdBuffer, dimension2Ptr->PL_2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
		dimension2Ptr->model2D->draw(dimension2Ptr->cmdBuffer);
	}
	void Dimension2::pushAndDraw(NineUIPushConstantData& push) {
		vkCmdPushConstants(dimension2Ptr->cmdBuffer, dimension2Ptr->PL_9, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
		dimension2Ptr->nineUIModel->draw(dimension2Ptr->cmdBuffer);
	}
}