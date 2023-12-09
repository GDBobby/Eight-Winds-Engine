#include "systems/Graphics/ocean.h"
#include "Data/TransformInclude.h"

namespace EWE {
	namespace Ocean {
		Ocean::Ocean(EWEDevice& device) :
			device{ device },
			oceanFFT{ ocean_resolution, device }
		{
			printf("ocean construction \n");
			throw std::exception("ocean construction not wanted");
			oceanPool =
				EWEDescriptorPool::Builder(device)
				.setMaxSets(1000)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
				.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100)
				.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100)
				.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100)
				.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
				.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
				.build();

			oceanFFT.createDSLs(device, oceanPool);

			ocean_time_struct.size = ocean_resolution;
			TransformComponent transform;
			ocean_push.modelMatrix = transform.mat4();

			oceanModel = EWEModel::createSimpleModelFromFile(device, "ocean.obj");
			foam = EWETexture::addGlobalTexture(device, "ocean/foam.jpg");

			renderPipeline;

			initializeDSLs();
			/*
			std::cout << "STRUCTURE SIZES ~~~~~~~~~~~~~ \n";
			std::cout << "Ocean_Ubo: " << sizeof(Ocean_Ubo) << "\n";
			std::cout << "Ocean_Draw_Push_Constant: " << sizeof(Ocean_Draw_Push_Constant) << "\n";
			std::cout << "Ocean_Material: " << sizeof(Ocean_Material) << "\n";
			std::cout << "Ocean_Time_Struct: " << sizeof(Ocean_Time_Struct) << "\n";
			std::cout << "Spectrum_Settings: " << sizeof(Spectrum_Settings) << "\n";
			std::cout << "Cascade_Parameters: " << sizeof(Cascade_Parameters) << "\n";
			std::cout << "Derivative_Data: " << sizeof(Derivative_Data) << '\n';
			*/
		}

		void Ocean::InitializeTwiddle(VkCommandBuffer cmdBuf) {
			oceanFFT.precompute(cmdBuf);

		}
		void Ocean::InitialiseCascades(VkCommandBuffer cmdBuf) {
			cascade.reserve(CASCADE_COUNT);
			for (int i = 0; i < CASCADE_COUNT; i++) {
				cascade.emplace_back(device, oceanFFT, ocean_resolution, gauss_buffer->descriptorInfo(), oceanPool, cascadeDSLs, spectrum_parameter_buffer->descriptorInfo(), ocean_time_buffer->descriptorInfo());
			}
			ocean_time_struct.lambda = waveSettings.lambda;

			ocean_ubo.LengthScale0 = lengthScale[0];
			ocean_ubo.LengthScale1 = lengthScale[1];
			ocean_ubo.LengthScale2 = lengthScale[2];
			ocean_ubo.LOD_scale = 1.f;
			ocean_ubo.SSSScale = 4.8f;
			ocean_ubo.SSSBase = 0.1f;

			float boundary1 = 2 * PI / lengthScale[1] * 6.f;
			float boundary2 = 2 * PI / lengthScale[2] * 6.f;
			spectrum_parameter_buffer->writeToBuffer(&waveSettings.spectrums[0]);

			spectrum_parameter_buffer->flush();
			std::cout << "BEGINNING CASCADE INIT \n";
			auto& initialPipe = ocean_compute_pipelines.at(Pipe_initial_spectrum);
			initialPipe.bind(cmdBuf);
			cascade[0].calculateInitial(cmdBuf, initialPipe.pipe_layout, lengthScale[0], 0.0001f, boundary1);
			cascade[1].calculateInitial(cmdBuf, initialPipe.pipe_layout, lengthScale[1], boundary1, boundary2);
			cascade[2].calculateInitial(cmdBuf, initialPipe.pipe_layout, lengthScale[2], boundary2, 9999);
			std::cout << "AFTER INIT \n";

			std::array<VkBufferMemoryBarrier, 3> bufferBarrier{};
			for (uint8_t i = 0; i < 3; i++) {
				bufferBarrier[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				bufferBarrier[i].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access type from previous operation (shader write)
				bufferBarrier[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access type for the next operation (shader read)
				bufferBarrier[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferBarrier[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferBarrier[i].buffer = cascade[i].getDerivativeVkBuffers()[4]; // The buffer you want to synchronize
				bufferBarrier[i].offset = 0;
				bufferBarrier[i].size = VK_WHOLE_SIZE; // The entire size of the buffer
			}

			vkCmdPipelineBarrier(
				cmdBuf,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,   // Source pipeline stage (compute shader stage)
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,   // Destination pipeline stage (compute shader stage)
				0,                                      // Dependency flags        
				0, nullptr,
				3, &bufferBarrier[0],
				0, nullptr                              // Image memory barriers
			);

			auto& conjugatePipe = ocean_compute_pipelines.at(Pipe_conjugate_spectrum);
			conjugatePipe.bind(cmdBuf);
			cascade[0].calculateInitialConjugate(cmdBuf, conjugatePipe.pipe_layout);
			cascade[1].calculateInitialConjugate(cmdBuf, conjugatePipe.pipe_layout);
			cascade[2].calculateInitialConjugate(cmdBuf, conjugatePipe.pipe_layout);

			std::cout << "AFTER CONJUGATE INIT \n";
		}

		void Ocean::ComputeUpdate(std::array<VkCommandBuffer, 5> oceanBuffers, float dt) {

			//std::cout << "~~~~~~~~~~~ BEGINNING COMPUTE UPDATE ~~~~~~~~~~~~~" << std::endl;
			/*
			if (alwaysRecalculateInitials)
			{
				InitialiseCascades();
			}
			*/

			ocean_time_struct.time += dt;
			ocean_time_struct.dt = dt;
			ocean_time_buffer->writeToBuffer(&ocean_time_struct);
			ocean_time_buffer->flush();

			std::cout << "ocean_time : " << ocean_time_struct.time << std::endl;

			//update variables
			//bind initial spectrum shader
			
			//std::cout << "before time dependent \n";
			{ // TIME DEPENDENT IS FINE
				auto& timePipe = ocean_compute_pipelines.at(Pipe_time_dependent_spectrum);
				timePipe.bind(oceanBuffers[0]);
				cascade[0].calculateTimeDependentSpectrum(oceanBuffers[0], timePipe.pipe_layout);
				cascade[1].calculateTimeDependentSpectrum(oceanBuffers[0], timePipe.pipe_layout);
				cascade[2].calculateTimeDependentSpectrum(oceanBuffers[0], timePipe.pipe_layout);
			}
			//std::cout << "after time dependent \n";
			
			//do fft stuff, 4 each
			//MERGE THESE PIPELINES INTO 1????????
			//std::cout << "before horizontal fft \n";
			{
				auto& horiPipe = oceanFFT.fft_pipelines.at(Pipe_horizontal_inverse_fft);
				horiPipe.bind(oceanBuffers[1]);
				cascade[0].calculate_Horizontal_IFFTS(oceanBuffers[1], horiPipe.pipe_layout);
				cascade[1].calculate_Horizontal_IFFTS(oceanBuffers[1], horiPipe.pipe_layout);
				cascade[2].calculate_Horizontal_IFFTS(oceanBuffers[1], horiPipe.pipe_layout);
			}
			//std::cout << "after horizontal fft \n";

			
			{
				auto& vertPipe = oceanFFT.fft_pipelines.at(Pipe_vertical_inverse_fft);
				vertPipe.bind(oceanBuffers[2]);
				cascade[0].calculate_Vertical_IFFTS(oceanBuffers[2], vertPipe.pipe_layout);
				cascade[1].calculate_Vertical_IFFTS(oceanBuffers[2], vertPipe.pipe_layout);
				cascade[2].calculate_Vertical_IFFTS(oceanBuffers[2], vertPipe.pipe_layout);
			}
			//std::cout << "after vert fft \n";
			{ //PERMUTE IS FINE
				auto& permutePipe = oceanFFT.fft_pipelines.at(Pipe_permute);
				permutePipe.bind(oceanBuffers[3]);
				cascade[0].calculate_permutes(oceanBuffers[3], permutePipe.pipe_layout);
				cascade[1].calculate_permutes(oceanBuffers[3], permutePipe.pipe_layout);
				cascade[2].calculate_permutes(oceanBuffers[3], permutePipe.pipe_layout);
			}
			
			{
				auto& tmPipe = ocean_compute_pipelines.at(Pipe_textures_merger);
				tmPipe.bind(oceanBuffers[4]);
				cascade[0].mergeTextures(oceanBuffers[4], tmPipe.pipe_layout);
				cascade[1].mergeTextures(oceanBuffers[4], tmPipe.pipe_layout);
				cascade[2].mergeTextures(oceanBuffers[4], tmPipe.pipe_layout);
				
			}
			//std::cout << "after merge \n";
			
			

			//std::cout << "~~~~~~~~~~~ ENDING COMPUTE UPDATE ~~~~~~~~~~~~~" << std::endl;
		}

		void Ocean::initializeDSLs() {
			
			cascadeDSLs[0] = EWEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) //waveData
				.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //noise
				.addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //parameters
				.addBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //spectrum_parameters
				.addBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //differential buffers
				.build();
			ocean_compute_pipelines.emplace(Pipe_initial_spectrum, EWE_Compute_Pipeline::createPipeline(device, { cascadeDSLs[0]->getDescriptorSetLayout() }, std::string{ "initial_spectrum.comp.spv" }));
			std::cout << "after initial spectrum" << std::endl;
				//cascade_compute_structs.emplace_back(device, cascadeDSLs[0], "initial_spectrum.comp.spv");
			
			
			cascadeDSLs[1] = EWEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) //H0
				.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) //waveData
				.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //differential buffers
				.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //differential buffers
				.addBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //differential buffers
				.addBinding(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //differential buffers
				.addBinding(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //time, lambda, size, dt
				.build();
			ocean_compute_pipelines.emplace(Pipe_time_dependent_spectrum, EWE_Compute_Pipeline::createPipeline(device, { cascadeDSLs[1]->getDescriptorSetLayout() }, "time_dependent_spectrum.comp.spv" ));
			std::cout << "after time dependent" << std::endl;
			
			
			cascadeDSLs[2] = EWEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //differential buffers
				.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //differential buffers
				.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //differential buffers
				.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //differential buffers
				.addBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //time, lambda, size, dt
				.build();
				//cascade_compute_structs.emplace_back(device, cascadeDSLs[2], "wave_texture_merger.comp.spv");
			
			cascadeDSLs[4] = EWEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT) //displacement
				.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT) //derivatives
				.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT) //turbulence
				.build();
			
			cascadeDSLs[5] = EWEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT) //displacement
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT) //derivatives
				.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT) //turbulence
				.build();
			

			ocean_compute_pipelines.emplace(Pipe_textures_merger, EWE_Compute_Pipeline::createPipeline(device, {cascadeDSLs[2]->getDescriptorSetLayout(), cascadeDSLs[4]->getDescriptorSetLayout()}, "wave_texture_merger.comp.spv"));
			std::cout << "after texture merge" << std::endl;
			
			
			cascadeDSLs[3] = EWEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) //H0
				.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) //waveData
				.addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //parameters
				.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) //differential buffers
				.build();
				//cascade_compute_structs.emplace_back(device, cascadeDSLs[3], "conjugated_spectrum.comp.spv");

			ocean_compute_pipelines.emplace(Pipe_conjugate_spectrum, EWE_Compute_Pipeline::createPipeline(device, {cascadeDSLs[3]->getDescriptorSetLayout()}, "conjugated_spectrum.comp.spv" ));
			std::cout << "after conjugated spectrum" << std::endl;

		}

		void Ocean::createRenderPipeline(VkPipelineRenderingCreateInfo const& pipeRenderInfo) {
			renderParamsDSL =
				EWEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

			renderParamsBuffer[0] = std::make_unique<EWEBuffer>(device, sizeof(Ocean_Ubo), MAX_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			renderParamsBuffer[0]->map();
			renderParamsBuffer[1] = std::make_unique<EWEBuffer>(device, sizeof(Ocean_Material), MAX_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			renderParamsBuffer[1]->map();

			renderParamsBuffer[0]->writeToBuffer(&ocean_ubo);
			renderParamsBuffer[1]->writeToBuffer(&ocean_material);
			renderParamsBuffer[0]->flush();
			renderParamsBuffer[1]->flush();

			renderParamsDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
			for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				if (!
					EWEDescriptorWriter(*renderParamsDSL, *oceanPool)
					.writeBuffer(0, renderParamsBuffer[0]->descriptorInfo())
					.writeBuffer(1, renderParamsBuffer[1]->descriptorInfo())
					//.writeBuffer(1, &buffers[i][2]->descriptorInfo())
					.build(renderParamsDescriptorSets[i])
					) {
					printf("monster desc failure \n");
					throw std::exception("failed to create monster descriptor set");
				}
			}

			for (uint8_t x = 0; x < 3; x++) {
				renderTextureDescriptorSets[x].resize(MAX_FRAMES_IN_FLIGHT);
				const auto imageInfos = cascade[x].getImageInfo();
				for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					if (!
						EWEDescriptorWriter(*cascadeDSLs[5], *oceanPool)
						.writeImage(0, imageInfos[0])
						.writeImage(1, imageInfos[1])
						.writeImage(2, imageInfos[2])
						//.writeBuffer(1, &buffers[i][2]->descriptorInfo())
						.build(renderTextureDescriptorSets[x][i])
						) {
						printf("monster desc failure \n");
						throw std::exception("failed to create monster descriptor set");
					}
				}
			}


			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

			VkPushConstantRange pushConstantRange{};
			pushConstantRange.offset = 0;
			pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
			pushConstantRange.size = sizeof(Ocean_Draw_Push_Constant);
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;


			std::vector<VkDescriptorSetLayout> tempDSL = *DescriptorHandler::getPipeDescSetLayout(PDSL_global, device);
				tempDSL.push_back(renderParamsDSL->getDescriptorSetLayout());
				tempDSL.push_back(cascadeDSLs[5]->getDescriptorSetLayout());
				tempDSL.push_back(cascadeDSLs[5]->getDescriptorSetLayout());
				tempDSL.push_back(cascadeDSLs[5]->getDescriptorSetLayout());
				tempDSL.push_back(EWETexture::getSimpleDescriptorSetLayout());

			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL.size());
			pipelineLayoutInfo.pSetLayouts = tempDSL.data();

			if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &renderPipeLayout) != VK_SUCCESS) {
				printf("failed to create ocean render pipe layout \n");
				throw std::runtime_error("Failed to create pipe layout \n");
			}

			EWEPipeline::PipelineConfigInfo pipeConfigInfo{};
			EWEPipeline::defaultPipelineConfigInfo(pipeConfigInfo);

			pipeConfigInfo.attributeDescriptions = EWEModel::simpleVertex::getAttributeDescriptions();
			pipeConfigInfo.bindingDescriptions = EWEModel::getBindingDescriptions<EWEModel::simpleVertex>();
			pipeConfigInfo.pipelineLayout = renderPipeLayout;
			pipeConfigInfo.pipelineRenderingInfo = pipeRenderInfo;

			renderPipeline = std::make_unique<EWEPipeline>(device, "ocean.vert.spv", "ocean.frag.spv", pipeConfigInfo);
		}

		void Ocean::createBuffers() {
			getGaussNoise();

			gauss_buffer = std::make_unique<EWEBuffer>(
				device, noise_resolution * noise_resolution * sizeof(float) * 2, 1,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			gauss_buffer->map();
			gauss_buffer->writeToBuffer(gaussianNoise.data());

			ocean_time_buffer = std::make_unique<EWEBuffer>(
				device, sizeof(Ocean_Time_Struct), 1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			ocean_time_buffer->map();

			spectrum_parameter_buffer = std::make_unique<EWEBuffer>(
				device, sizeof(Spectrum_Settings) * 2, 1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			spectrum_parameter_buffer->map();
		}

		void Ocean::RenderUpdate(std::pair<VkCommandBuffer, uint8_t> cmdIndexPair) {
			//std::cout << " ~~~~~~~~~~~~ BEGINNING OCEAN RENDER ~~~~~~~~~~~~" << std::endl;
			renderPipeline->bind(cmdIndexPair.first);

			//global descriptor set
			vkCmdBindDescriptorSets(
				cmdIndexPair.first,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				renderPipeLayout,
				0, 1,
				DescriptorHandler::getDescSet(DS_global, cmdIndexPair.second),
				0,
				nullptr
			);
			//std::cout << "after global desc OCEAN \n";

			vkCmdBindDescriptorSets(
				cmdIndexPair.first,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				renderPipeLayout,
				1, 1,
				&renderParamsDescriptorSets[cmdIndexPair.second],
				0, nullptr
			);
			//std::cout << "after params desc OCEAN \n";

			for (uint8_t i = 0; i < 3; i++) {
				vkCmdBindDescriptorSets(
					cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					renderPipeLayout,
					2 + i, 1,
					&renderTextureDescriptorSets[i][cmdIndexPair.second],
					0, nullptr
				);
				//cascade[i].bindMergedTextures(cmdIndexPair.first, renderPipeLayout, 2 + i);
			}
			//std::cout << "after texture desc OCEAN \n";

			vkCmdBindDescriptorSets(
				cmdIndexPair.first,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				renderPipeLayout,
				5, 1,
				EWETexture::getDescriptorSets(foam, cmdIndexPair.second),
				0, nullptr
			);
			//std::cout << "after foam desc OCEAN \n";

			ocean_push.time = time;
			vkCmdPushConstants(cmdIndexPair.first, renderPipeLayout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(Ocean_Draw_Push_Constant), &ocean_push);

			oceanModel->bind(cmdIndexPair.first);
			oceanModel->draw(cmdIndexPair.first);

			//std::cout << " ~~~~~~~~~~~ ENDING OCEAN RENDER ~~~~~~~~~~~~~" << std::endl;
		}
	}
}