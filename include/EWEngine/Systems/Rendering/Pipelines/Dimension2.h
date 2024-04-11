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


	public:
		static void Init();
		static void Destruct();

		static void BindNineUI(VkCommandBuffer cmdBuffer, uint8_t frameIndex);
		static void Bind2D(VkCommandBuffer cmdBuffer, uint8_t frameIndex);

		static void BindTexture2DUI(TextureDesc texture);
		static void BindTexture2D(TextureDesc texture);
		static void BindTexture9(TextureDesc texture);

		static void PushAndDraw(Simple2DPushConstantData& push);
		static void PushAndDraw(NineUIPushConstantData& push);


	};
}

