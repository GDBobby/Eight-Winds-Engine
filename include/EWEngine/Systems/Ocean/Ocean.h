#pragma once
#include "OceanFFT.h"
#include "WaveCascades.h"
#include "EWEngine/Graphics/Model/Model.h"

//this is largely self-contained, and builds its own systems.
//	as i expand the engine, and these systems are needed in mroe places, 
//	ill remove them from this object and abstract them a little

namespace EWE {
	namespace Ocean {

#define CASCADE_COUNT 3
		class Ocean {

			const uint16_t ocean_resolution{ 1024 };
			const uint16_t noise_resolution{ 256 };
			EWEDevice& device;
			std::vector<WaveCascades> cascade;
			OceanFFT oceanFFT;
			Wave_Settings waveSettings;

			std::vector<std::array<float, 2>> gaussianNoise;

			float lengthScale[3] = { 250, 17, 5 };
			std::map<Pipe_Enum, EWE_Compute_Pipeline> ocean_compute_pipelines;
			std::array<std::unique_ptr<EWEDescriptorSetLayout>, 6> cascadeDSLs;

			std::shared_ptr<EWEDescriptorPool> oceanPool;

			Ocean_Time_Struct ocean_time_struct;
			std::unique_ptr<EWEBuffer> ocean_time_buffer;

			std::unique_ptr<EWEBuffer> spectrum_parameter_buffer;
			Spectrum_Settings spectrumSettings[2];

			std::unique_ptr<EWEBuffer> gauss_buffer;


		//RENDER BEGIN
			std::unique_ptr<EWEPipeline> renderPipeline;
			VkPipelineLayout renderPipeLayout;
			std::unique_ptr<EWEDescriptorSetLayout> renderParamsDSL;
			std::array<std::unique_ptr<EWEBuffer>, 2> renderParamsBuffer;

			std::vector<VkDescriptorSet> renderParamsDescriptorSets;
			std::array<std::vector<VkDescriptorSet>, 3> renderTextureDescriptorSets;

			Ocean_Draw_Push_Constant ocean_push{};
			Ocean_Ubo ocean_ubo{};
			Ocean_Material ocean_material{};

			std::unique_ptr<EWEModel> oceanModel;

			TextureID foam = 0;
		//RENDER END

			float time = 0.f;

			void initializeDSLs();

			void getGaussNoise() {
				std::string filename = "textures/compute/gaussian_noise.bob";
				//use filesystem to find file, if not, generate

				std::ifstream file{ filename, std::ios::binary };
				if (file.is_open()) {
					file.seekg(0, std::ios::end);
					std::streampos fileSize = file.tellg();
					file.seekg(0, std::ios::beg);

					// Calculate the number of uint64_t values in the file
					size_t numValues = fileSize / (sizeof(float) * 2);

					// Resize the result vector to accommodate the data
					gaussianNoise.resize(numValues);

					// Read the binary data into the vector
					file.read(reinterpret_cast<char*>(&gaussianNoise[0]), fileSize);

					// Close the file
					file.close();
				}
				else {
					gaussianNoise.resize(noise_resolution * noise_resolution);
					std::default_random_engine m_engine{ static_cast<std::uint32_t>(std::random_device{}()) };
					std::uniform_real_distribution<float> distribution{ 0.f, 1.f};// = std::uniform_real_distribution<float>;

					uint64_t index = 0;
					for (int i = 0; i < noise_resolution; i++) {
						for (int j = 0; j < noise_resolution; j++) {

							gaussianNoise[index][0] = GaussianRandom(m_engine, distribution);
							gaussianNoise[index][1] = GaussianRandom(m_engine, distribution);

							//std::cout << "gauss index:values  - " << index << gaussianNoise[index][0] << ":" << gaussianNoise[index][1] << std::endl;
							index++;
						}
					}
					std::ofstream out_file{ "textures/compute/gaussian_noise.bob", std::ios::binary };
					out_file.write(reinterpret_cast<const char*>(gaussianNoise.data()), gaussianNoise.size() * sizeof(float) * 2);
					out_file.close();
					
				}
			}
			float GaussianRandom(std::default_random_engine& randE, std::uniform_real_distribution<float>& distribution) {

				return std::cos(2 * 3.14159265358979323846f * distribution(randE)) * std::sqrt(-2.f * std::log(distribution(randE)));
			}

		public:
			Ocean(EWEDevice& device);

			void InitializeTwiddle(VkCommandBuffer cmdBuf);
			void InitialiseCascades(VkCommandBuffer cmdBuf);

			//maybe construct the command buffer here?
			void ComputeUpdate(std::array<VkCommandBuffer, 5> oceanBuffers, float dt);

			void RenderUpdate(std::pair<VkCommandBuffer, uint8_t> cmdIndexPair);

			void createBuffers();
			void createRenderPipeline(VkPipelineRenderingCreateInfo const& pipeRenderInfo);

			void transferOceanToGraphics(VkCommandBuffer cmdBuffer) {
				std::vector<VkImage> transferVertImages{};
				transferVertImages.reserve(3);
				cascade[0].addVertTransferImages(transferVertImages);
				cascade[1].addVertTransferImages(transferVertImages);
				cascade[2].addVertTransferImages(transferVertImages);
				device.transferImageStage(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, transferVertImages);

				std::vector<VkImage> transferFragImages{};
				transferFragImages.reserve(6);
				cascade[0].addFragTransferImages(transferFragImages);
				cascade[1].addFragTransferImages(transferFragImages);
				cascade[2].addFragTransferImages(transferFragImages);
				device.transferImageStage(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, transferFragImages);
			}

			void transferGraphicsToOcean(VkCommandBuffer cmdBuf) {
				std::vector<VkImage> transferVertImages{};
				transferVertImages.reserve(3);
				cascade[0].addVertTransferImages(transferVertImages);
				cascade[1].addVertTransferImages(transferVertImages);
				cascade[2].addVertTransferImages(transferVertImages);
				device.transferImageStage(cmdBuf, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, transferVertImages);

				std::vector<VkImage> transferFragImages{};
				transferFragImages.reserve(6);
				cascade[0].addFragTransferImages(transferFragImages);
				cascade[1].addFragTransferImages(transferFragImages);
				cascade[2].addFragTransferImages(transferFragImages);
				device.transferImageStage(cmdBuf, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, transferFragImages);
			}
		};
	}
}