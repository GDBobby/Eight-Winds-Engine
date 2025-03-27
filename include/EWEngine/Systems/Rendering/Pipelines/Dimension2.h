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

		enum WhichPipe {
			Pipe_2D,
			Pipe_NineUI,
		};

		EWEPipeline* pipe_array;
		VkPipelineLayout PL_array;
		EWEPipeline* pipe_single;
		VkPipelineLayout PL_single;

		//VkPipelineLayout PL_9;
		ImageID bindedTexture;
		VkPipelineCache cache;
		EWEModel* model2D;
		VkDescriptorSet defaultDesc;
		EWEDescriptorSetLayout* eDSL{ nullptr };
		//EWEModel* nineUIModel;

		ImageID uiArrayID;
		void CreateDefaultDesc();

	public:
		~Dimension2() {}
		static void Init();
		static void Destruct();

		//static void BindNineUI(CommandBuffer cmdBuffer, uint8_t frameIndex);
		static void BindModel() {
			dimension2Ptr->model2D->Bind();
		}
		static void DrawModel() {
			dimension2Ptr->model2D->Draw();
		}
		static EWEModel* GetModel() {
			return dimension2Ptr->model2D;
		}
		static void BindArrayPipeline();
		static void BindSingularPipeline();

		static void BindDefaultDesc();
		static void BindSingleDescriptor(VkDescriptorSet* desc);
		static void BindArrayDescriptor(VkDescriptorSet* desc);
		//static void BindTexture2DUI(ImageID texture);
		//static void BindTexture2D(ImageID texture);
		//static void BindTexture9(TextureDesc texture);

		static void PushAndDraw(Array2DPushConstantData& push);
		static void PushAndDraw(Single2DPushConstantData& push);

		static EWEDescriptorSetLayout* GetDSL() {
			return dimension2Ptr->eDSL;
		}
		//static void PushAndDraw(NineUIPushConstantData& push);


	};
}

