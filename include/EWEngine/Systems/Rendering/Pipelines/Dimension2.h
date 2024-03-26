#pragma once
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Pipeline.h"
#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/PushConstants.h"

namespace EWE {

	class Dimension2 {
	private: //protected? no dif
		static Dimension2* dimension2Ptr;
		Dimension2();
		~Dimension2() {}

		enum WhichPipe {
			Pipe_2D,
			Pipe_NineUI,
		};

		EWEPipeline* pipe2d;
		EWEPipeline* pipe9;
		VkPipelineLayout PL_2d;
		VkPipelineLayout PL_9;
		TextureDesc bindedTexture;
		VkPipelineCache cache;
		std::unique_ptr<EWEModel> model2D;
		std::unique_ptr<EWEModel> nineUIModel;
		VkCommandBuffer cmdBuffer;
		uint8_t frameIndex;

		EWEPipeline::PipelineConfigInfo InitConfigInfo();
		EWEPipeline::PipelineConfigInfo SecondConfigInfo();

	public:
		static void init();
		static void destruct();

		static void bindNineUI(VkCommandBuffer cmdBuffer, uint8_t frameIndex);
		static void bind2D(VkCommandBuffer cmdBuffer, uint8_t frameIndex);

		static void bindTexture2DUI(TextureDesc texture);
		static void bindTexture2D(TextureDesc texture);
		static void bindTexture9(TextureDesc texture);

		static void pushAndDraw(Simple2DPushConstantData& push);
		static void pushAndDraw(NineUIPushConstantData& push);


	};
}

