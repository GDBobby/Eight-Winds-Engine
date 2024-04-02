#pragma once
#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/Pipeline.h"
#include "EWEngine/Systems/Ocean/OceanStructs.h"

//this is largely self-contained, and builds its own systems.
//	as i expand the engine, and these systems are needed in mroe places, 
//	ill remove them from this object and abstract them a little

namespace EWE {
	namespace Ocean {

		class Ocean {
			uint8_t cascade_count{ 4 };
			bool notFirstRun = false;

			InitialFrequencySpectrumGPUData ifsGPUData{};
			TimeDependentFrequencySpectrumGPUData tdfsGPUData{};
			FFTGPUData fftGPUData{};
			OceanGraphicsGPUData graphicsGPUData{};
			float depth = 1000.f;

			
			VkDescriptorSet oceanTextures = VK_NULL_HANDLE;
			VkImage oceanOutputImages = VK_NULL_HANDLE;
			VkImage oceanFreqImages = VK_NULL_HANDLE;
			VkDeviceMemory oceanOutputImageMemory = VK_NULL_HANDLE;
			VkDeviceMemory oceanFreqImageMemory = VK_NULL_HANDLE;
			VkDescriptorImageInfo oceanOutputImageInfoDescriptorCompute{};
			VkDescriptorImageInfo oceanOutputImageInfoDescriptorGraphics{};
			VkDescriptorImageInfo oceanFreqImageInfoDescriptor{};


		//RENDER BEGIN
			EWEPipeline* renderPipeline;
			VkPipelineLayout renderPipeLayout;
			std::array<EWEBuffer*, 2> renderParamsBuffer;


			std::unique_ptr<EWEModel> oceanModel;

			EWEBuffer* frequencyBuffer = nullptr;
		//RENDER END

			float time = 0.f;


		public:
			Ocean(VkDescriptorImageInfo* skyboxImage);
			~Ocean();
			void UpdateNoInit(FrameInfo const& frameInfo, float dt);
			void ReinitUpdate(FrameInfo const& frameInfo, float dt);
			void InitializeSpectrum(FrameInfo const& frameInfo);
			void UpdateSpectrum(FrameInfo const& frameInfo, float dt);
			void ComputeFFT(FrameInfo const& frameInfo, float deltaTime);


			void RenderOcean(FrameInfo const& frameInfo);

			void CreateBuffers();
			void PrepareStorageImage();
			void CreateDescriptor(VkDescriptorImageInfo* skyboxImage);

			void TransferComputeToGraphics(VkCommandBuffer cmdBuf);

			void TransferGraphicsToCompute(VkCommandBuffer cmdBuf);
			//void ComputeBarrier(VkCommandBuffer cmdBuf);
			


		};
	}
}