#pragma once
#include "EWEngine/graphics/EWE_Device.hpp"
#include "EWEngine/graphics/EWE_Pipeline.h"
#include "EWEngine/graphics/model/EWE_model.h"
#include "EWEngine/graphics/PushConstants.h"

namespace EWE {

	class Dimension2 {
	private: //protected? no dif
		static Dimension2* dimension2Ptr;
		Dimension2(EWEDevice& device, VkPipelineRenderingCreateInfo const& pipeRenderInfo);

		enum WhichPipe {
			Pipe_2D,
			Pipe_NineUI,
		};

		std::unique_ptr<EWEPipeline> pipe2d;
		std::unique_ptr<EWEPipeline> pipe9;
		VkPipelineLayout PL_2d;
		VkPipelineLayout PL_9;
		TextureID bindedTexture;
		VkPipelineCache cache;
		std::unique_ptr<EWEModel> model2D;
		std::unique_ptr<EWEModel> nineUIModel;
		VkCommandBuffer cmdBuffer;
		uint8_t frameIndex;

	public:
		static void init(EWEDevice& device, VkPipelineRenderingCreateInfo const& pipeRenderInfo);
		static void destruct(EWEDevice& device);

		static void bindNineUI(VkCommandBuffer cmdBuffer, uint8_t frameIndex);
		static void bind2D(VkCommandBuffer cmdBuffer, uint8_t frameIndex);

		static void bindTexture2D(TextureID  textureID);
		static void bindTexture9(TextureID  textureID);

		static void pushAndDraw(Simple2DPushConstantData& push);
		static void pushAndDraw(NineUIPushConstantData& push);


	};
}

