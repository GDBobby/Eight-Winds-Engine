#include "FloatingRockSystem.h"
#include <EWEngine/Systems/Rendering/Pipelines/Pipe_SimpleTextured.h>
#include <EWEngine/Systems/Rendering/Rigid/RigidRS.h>
#include "EWEngine/Data/EWE_Import.h"

namespace EWE {
	static constexpr uint32_t rock_count = 32 * 32;
#pragma pack(1)
	struct RockData {
		uint32_t trackID;
		uint32_t trackPosition;
		float tilt;
		float RPS;
		glm::vec3 scale;
		uint32_t p_padding; //not for usage
		static void InitData(std::vector<RockData>& rockData, std::default_random_engine& randomGen) {

			std::uniform_real_distribution<float> rotationalDistribution(-2.f, 2.f);
			std::uniform_real_distribution<float> scaleDistribution(0.25f, 0.75f);
			std::uniform_real_distribution<float> tiltDist(0.f, glm::two_pi<float>());


			for (auto& rock : rockData) {
				rock.RPS = rotationalDistribution(randomGen);

				const float scale = scaleDistribution(randomGen);
				rock.scale.x = scale;
				rock.scale.y = scale;
				rock.scale.z = scale;

				rock.tilt = tiltDist(randomGen);
			}
		}
	};
	struct TrackData {
		uint32_t rockCount;
		float RPS;
		float tilt;
		float radius;
		static void InitData(std::vector<TrackData>& trackData, std::vector<RockData>& rockData, uint8_t trackCount, std::default_random_engine& randomGen) {
			std::uniform_int_distribution<uint32_t> rockDistribution(8, 32);

			std::vector<float> rockPerTrack(trackCount);
			float rockSum = 0.f;
			for (uint8_t i = 0; i < trackCount; i++) {
				rockPerTrack[i] = static_cast<float>(rockDistribution(randomGen));
				rockSum += rockPerTrack[i];
			}
			const float normalizationRatio = rock_count / rockSum;
			uint32_t preadjustedSum = 0;
			for (uint8_t i = 0; i < trackCount; i++) {
				rockPerTrack[i] *= normalizationRatio;
				trackData[i].rockCount = glm::round(rockPerTrack[i]);
				preadjustedSum += trackData[i].rockCount;
			}
			int32_t difference = static_cast<int32_t>(preadjustedSum) - static_cast<int32_t>(rock_count);

			while (difference != 0) {
				TrackData* trackPtr = &trackData[0];
				if (difference < 0) {
					for (uint32_t i = 1; i < trackCount; i++) {
						if (trackData[i].rockCount < trackPtr->rockCount) {
							trackPtr = &trackData[i];
						}
					}
					trackPtr->rockCount++;
					difference++;
				}
				else {
					for (uint32_t i = 1; i < trackCount; i++) {
						if (trackData[i].rockCount > trackPtr->rockCount) {
							trackPtr = &trackData[i];
						}
					}
					trackPtr->rockCount--;
					difference--;
				}
			}

#if EWE_DEBUG
			uint32_t finalSum = 0;
			for (uint8_t i = 0; i < trackCount; i++) {
				finalSum += trackData[i].rockCount;
			}
			assert(finalSum == rock_count);
#endif
			std::uniform_real_distribution<float> rotDistribution(0.01f, 0.1f); //the higher this number is, the slower they'll be. 250 is 1 full rotation per second

			uint32_t accountedRocks = 0;
			for (uint8_t i = 0; i < trackCount; i++) {
				trackData[i].tilt = glm::two_pi<float>() * i / trackCount;
				trackData[i].radius = powf(static_cast<float>(i) + 1.f, 0.6f) + 30.f;
				trackData[i].RPS = rotDistribution(randomGen);
				for (uint16_t j = 0; j < trackData[i].rockCount; j++) {
					rockData[j + accountedRocks].trackID = i;
					rockData[j + accountedRocks].trackPosition = j;
				}
				accountedRocks += trackData[i].rockCount;
			}
		}
	};
#pragma pop

	void InitBuffers(EWEBuffer*& rockBuffer, EWEBuffer*& trackBuffer, std::vector<RockData>& rockData, std::vector<TrackData>& trackData) {
		const std::size_t rockBufferSize = rockData.size() * sizeof(RockData);
		const std::size_t trackBufferSize = trackData.size() * sizeof(TrackData);

		rockBuffer = Construct<EWEBuffer>({rockBufferSize, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT});
		trackBuffer = Construct<EWEBuffer>({ trackBufferSize, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT });
		VkPhysicalDevice physDev = EWEDevice::GetEWEDevice()->GetPhysicalDevice();
		VkDevice vkDevice = EWEDevice::GetVkDevice();
		StagingBuffer* rockStagingBuffer = Construct<StagingBuffer>({ rockBufferSize, physDev, vkDevice });
		StagingBuffer* trackStagingBuffer = Construct<StagingBuffer>({ trackBufferSize, physDev, vkDevice });


		rockStagingBuffer->Stage(vkDevice, rockData.data(), rockBufferSize);
		trackStagingBuffer->Stage(vkDevice, trackData.data(), trackBufferSize);

		SyncHub* syncHub = SyncHub::GetSyncHubInstance();
		VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommandTransfer();
		EWEDevice* eweDevice = EWEDevice::GetEWEDevice();
		eweDevice->CopyBuffer(cmdBuf, rockStagingBuffer->buffer, rockBuffer->GetBuffer(), rockBufferSize);
		eweDevice->CopyBuffer(cmdBuf, trackStagingBuffer->buffer, trackBuffer->GetBuffer(), trackBufferSize);
		CommandWithCallback cb{};
		cb.cmdBuf = cmdBuf;
		cb.callback = [rsb = rockStagingBuffer, tsb = trackStagingBuffer,
#if USING_VMA
			memMgr = EWEDevice::GetAllocator()
#else
			memMgr = EWEDevice::GetVkDevice()
#endif
		] {rsb->Free(memMgr); tsb->Free(memMgr); Deconstruct(rsb); Deconstruct(tsb); };

		syncHub->EndSingleTimeCommandTransfer(cb);
		
	}


	FloatingRock::FloatingRock() :rockTexture{ Material_Texture::CreateMaterialTexture("eye/", true) } {
		//rockModel = EWEModel::CreateModelFromFile("rock1.obj", Queue::transfer);
		rockTexture.materialFlags |= MaterialF_instanced;

		std::ifstream inFile("models/eye_simpleMesh.ewe", std::ifstream::binary);
		//inFile.open();
		assert(inFile.is_open() && "failed to open eye model");
		ImportData::TemplateMeshData<Vertex> importMesh{};

		uint32_t endianTest = 1;
		bool endian = (*((char*)&endianTest) == static_cast<char>(1));

		if (endian) {
			importMesh.ReadFromFile(inFile);
		}
		else {
			importMesh.ReadFromFileSwapEndian(inFile);
		}
		inFile.close();
		//printf("file read successfully \n");

		rockModel = EWEModel::CreateMesh(importMesh.meshes[0].vertices.data(), importMesh.meshes[0].vertices.size(), importMesh.vertex_size, importMesh.meshes[0].indices, Queue::transfer);

#if DEBUG_NAMING
		rockModel->SetDebugNames("eyeModel");
#endif

		//RANDOM NUMBER GENERATOR
		std::random_device r;
		std::default_random_engine randomGen(r());
		std::uniform_int_distribution<uint32_t> trackDistribution(16, 32); //tracks per field (only 1 field)
		uint32_t trackCount = trackDistribution(randomGen);

		std::vector<RockData> rockData(rock_count);
		std::vector<TrackData> trackData(trackCount);

		TrackData::InitData(trackData, rockData, trackCount, randomGen);
		RockData::InitData(rockData, randomGen);

		//buffers
		InitBuffers(rockBuffer, trackBuffer, rockData, trackData);

		InitComputeData();
	}
	FloatingRock::~FloatingRock() {
		Deconstruct(rockModel);
		EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, &compDescriptorSet[0]);
		EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, &compDescriptorSet[1]);

		EWE_VK(vkDestroyPipeline, EWEDevice::GetVkDevice(), compPipeline, nullptr);
		EWE_VK(vkDestroyPipelineLayout, EWEDevice::GetVkDevice(), compPipeLayout, nullptr);
		EWE_VK(vkDestroyShaderModule, EWEDevice::GetVkDevice(), compShaderModule, nullptr);
	}
	void FloatingRock::Dispatch(float dt, FrameInfo const& frameInfo) {
		if (previouslySubmitted) {
			EWE_VK(vkCmdPipelineBarrier, frameInfo.cmdBuf,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
				0,
				0, nullptr,
				1, &bufferBarrier[frameInfo.index + MAX_FRAMES_IN_FLIGHT],
				0, nullptr
			);
		}
		else {
			previouslySubmitted = true;
		}
		EWE_VK(vkCmdBindPipeline, frameInfo.cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, compPipeline);

		EWE_VK(vkCmdBindDescriptorSets, frameInfo.cmdBuf,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			compPipeLayout,
			0, 1,
			&compDescriptorSet[frameInfo.index],
			0, nullptr
		);

		compPushData.secondsSinceBeginning += dt;
		//compPushData.whichIndex = frameInfo.index;
		EWE_VK(vkCmdPushConstants, frameInfo.cmdBuf, compPipeLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(RockCompPushData), &compPushData);

		EWE_VK(vkCmdDispatch, frameInfo.cmdBuf, 1, 1, 1);

		EWE_VK(vkCmdPipelineBarrier, frameInfo.cmdBuf,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
			0,
			0, nullptr,
			1, &bufferBarrier[frameInfo.index],
			0, nullptr
		);
	}
	void FloatingRock::InitComputeData(){
		RigidRenderingSystem::AddInstancedMaterialObject(rockTexture, rockModel, rock_count, true);
		const std::array<const EWEBuffer*, MAX_FRAMES_IN_FLIGHT> transformBuffers = RigidRenderingSystem::GetBothTransformBuffers(rockModel);

		bufferBarrier[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier[0].pNext = nullptr;
		bufferBarrier[0].buffer = transformBuffers[0]->GetBuffer();
		bufferBarrier[0].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		bufferBarrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		bufferBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier[0].offset = 0;
		bufferBarrier[0].size = VK_WHOLE_SIZE;

		bufferBarrier[1] = bufferBarrier[0];
		bufferBarrier[1].buffer = transformBuffers[1]->GetBuffer();

		bufferBarrier[2] = bufferBarrier[0];

		bufferBarrier[2].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		bufferBarrier[2].dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

		bufferBarrier[3] = bufferBarrier[1];
		bufferBarrier[2].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		bufferBarrier[2].dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;


		//descriptor set
		EWEDescriptorSetLayout::Builder dslBuilder{};
		dslBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		dslBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		dslBuilder.AddBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		auto tempDSL = dslBuilder.Build();

		{
			VkDescriptorBufferInfo descInfo = transformBuffers[0]->DescriptorInfo();
			EWEDescriptorWriter descWriter{ tempDSL, DescriptorPool_Global };
			descWriter.WriteBuffer(0, rockBuffer->DescriptorInfo());
			descWriter.WriteBuffer(1, trackBuffer->DescriptorInfo());
			descWriter.WriteBuffer(2, &descInfo);
			compDescriptorSet[0] = descWriter.Build();
		}
		{
			VkDescriptorBufferInfo descInfo = transformBuffers[1]->DescriptorInfo();
			EWEDescriptorWriter descWriter{ tempDSL, DescriptorPool_Global };
			descWriter.WriteBuffer(0, rockBuffer->DescriptorInfo());
			descWriter.WriteBuffer(1, trackBuffer->DescriptorInfo());
			descWriter.WriteBuffer(2, &descInfo);
			compDescriptorSet[1] = descWriter.Build();
		}

		//layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(RockCompPushData);
		
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		pipelineLayoutInfo.pushConstantRangeCount = 1;

		VkDescriptorSetLayout tempVkDSL = tempDSL->GetDescriptorSetLayout();

		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &tempVkDSL;

		EWE_VK(vkCreatePipelineLayout, EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &compPipeLayout);

		//pipeline
		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;
		pipelineInfo.layout = compPipeLayout;
		Pipeline_Helper_Functions::CreateShaderModule("RockTrack.comp.spv", &compShaderModule);
		//VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipelineInfo.stage.pNext = nullptr;
		pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		pipelineInfo.stage.module = compShaderModule;
		pipelineInfo.stage.pName = "main";
		pipelineInfo.stage.flags = 0;
		pipelineInfo.stage.pSpecializationInfo = nullptr;

		EWE_VK(vkCreateComputePipelines, EWEDevice::GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &compPipeline);
	}

}