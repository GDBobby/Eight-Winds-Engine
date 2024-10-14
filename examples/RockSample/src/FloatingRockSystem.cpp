#include "FloatingRockSystem.h"
#include <EWEngine/Systems/Rendering/Pipelines/Pipe_SimpleTextured.h>
#include <EWEngine/Systems/Rendering/Rigid/RigidRS.h>

namespace EWE {
	static constexpr uint32_t rock_count = 32 * 32;

	struct RockData {
		uint32_t trackID;
		uint32_t trackPosition;
		float tilt;
		float RPS;
		glm::vec3 scale;
		static void InitData(std::vector<RockData>& rockData, std::default_random_engine& randomGen) {

			std::uniform_real_distribution<float> rotationalDistribution(0.1f, 2.f);
			std::uniform_real_distribution<float> scaleDistribution(0.25f, 0.75f);
			std::uniform_real_distribution<float> tiltDist(0.f, glm::two_pi<float>());


			for (uint32_t i = 0; i < rockData.size(); i++) {
				rockData[i].RPS = rotationalDistribution(randomGen);

				const float scale = scaleDistribution(randomGen);
				rockData[i].scale.x = scale;
				rockData[i].scale.y = scale;
				rockData[i].scale.z = scale;

				rockData[i].tilt = tiltDist(randomGen);
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
				}
				else {
					for (uint32_t i = 1; i < trackCount; i++) {
						if (trackData[i].rockCount > trackPtr->rockCount) {
							trackPtr = &trackData[i];
						}
					}
					trackPtr->rockCount--;
				}
			}

#if EWE_DEBUG
			uint32_t finalSum = 0;
			for (uint8_t i = 0; i < trackCount; i++) {
				finalSum += trackData[i].rockCount;
			}
			assert(finalSum == rock_count);
#endif
			std::uniform_real_distribution<float> rotDistribution(0.1f, 2.f); //the higher this number is, the slower they'll be. 250 is 1 full rotation per second

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

	void InitBuffers(EWEBuffer*& rockBuffer, EWEBuffer*& trackBuffer, std::vector<RockData>& rockData, std::vector<TrackData>& trackData, EWEBuffer** transformBuffer) {
		const std::size_t rockBufferSize = rockData.size() * sizeof(RockData);
		const std::size_t trackBufferSize = trackData.size() * sizeof(TrackData);

		rockBuffer = Construct<EWEBuffer>({rockBufferSize, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT});
		trackBuffer = Construct<EWEBuffer>({ trackBufferSize, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT });

		transformBuffer[0] = Construct<EWEBuffer>({ rock_count * sizeof(glm::mat4), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT });
		transformBuffer[1] = Construct<EWEBuffer>({ rock_count * sizeof(glm::mat4), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT });

		VkPhysicalDevice physDev = EWEDevice::GetEWEDevice()->GetPhysicalDevice();
		VkDevice vkDevice = EWEDevice::GetVkDevice();
		StagingBuffer* rockStagingBuffer = Construct<StagingBuffer>({ rockBufferSize, physDev, vkDevice });
		StagingBuffer* trackStagingBuffer = Construct<StagingBuffer>({ trackBufferSize, physDev, vkDevice });


		rockStagingBuffer->Stage(vkDevice, rockData.data(), rockBufferSize);
		trackStagingBuffer->Stage(vkDevice, trackData.data(), trackBufferSize);

		SyncHub* syncHub = SyncHub::GetSyncHubInstance();
		VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommandTransfer();
		EWEDevice* eweDevice = EWEDevice::GetEWEDevice();
		{
			eweDevice->CopyBuffer(cmdBuf, rockStagingBuffer->buffer, rockBuffer->GetBuffer(), rockBufferSize);
			CommandWithCallback cb{};
			cb.cmdBuf = cmdBuf;
			cb.callback = [sb = rockStagingBuffer,
#if USING_VMA
				memMgr = EWEDevice::GetAllocator()
#else
				memMgr = EWEDevice::GetVkDevice()
#endif
			] {sb->Free(memMgr); Deconstruct(sb);};
			syncHub->EndSingleTimeCommandTransfer(cb);
		}
		{
			eweDevice->CopyBuffer(cmdBuf, trackStagingBuffer->buffer, trackBuffer->GetBuffer(), trackBufferSize);
			CommandWithCallback cb{};
			cb.cmdBuf = cmdBuf;
			cb.callback = [sb = trackStagingBuffer,
#if USING_VMA
				memMgr = EWEDevice::GetAllocator()
#else
				memMgr = EWEDevice::GetVkDevice()
#endif
			] {sb->Free(memMgr); Deconstruct(sb);};
			syncHub->EndSingleTimeCommandTransfer(cb);
		}
	}


	FloatingRock::FloatingRock() {
		rockModel = EWEModel::CreateModelFromFile("rock1.obj", Queue::transfer);

		rockTexture = Texture_Builder::CreateSimpleTexture("rock/rock_albedo.jpg", true, true, VK_SHADER_STAGE_FRAGMENT_BIT);

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
		InitBuffers(rockBuffer, trackBuffer, rockData, trackData, transformBuffer);

		InitComputeData();
	}
	FloatingRock::~FloatingRock() {
		delete rockModel;
		EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, &compDescriptorSet[0]);
		EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, &compDescriptorSet[1]);

		EWE_VK(vkDestroyPipeline, EWEDevice::GetVkDevice(), compPipeline, nullptr);
		EWE_VK(vkDestroyPipelineLayout, EWEDevice::GetVkDevice(), compPipeLayout, nullptr);
		EWE_VK(vkDestroyShaderModule, EWEDevice::GetVkDevice(), compShaderModule, nullptr);
	}
	void FloatingRock::Dispatch(float dt, FrameInfo const& frameInfo) {
		EWE_VK(vkCmdBindPipeline, frameInfo.cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, compPipeline);

		EWE_VK(vkCmdBindDescriptorSets, frameInfo.cmdBuf,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			compPipeLayout,
			0, 1,
			&compDescriptorSet[frameInfo.index],
			0, nullptr
		);

		compPushData.secondsSinceBeginning += dt;
		compPushData.whichIndex = frameInfo.index;
		EWE_VK(vkCmdPushConstants, frameInfo.cmdBuf, compPipeLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(RockCompPushData), &compPushData);

		EWE_VK(vkCmdDispatch, frameInfo.cmdBuf, 1, 1, 1);

	}
	void FloatingRock::render(FrameInfo& frameInfo) {

		PipelineSystem::SetFrameInfo(frameInfo);
		auto pipe = PipelineSystem::At(Pipe::textured);

		pipe->BindPipeline();

		pipe->BindDescriptor(0, DescriptorHandler::GetDescSet(DS_global, frameInfo.index));
		pipe->BindDescriptor(1, &rockTexture);

		pipe->BindModel(rockModel);


		SimplePushConstantData push{ renderModelMatrix, renderNormalMatrix };
		for (int i = 0; i < rockField.size(); i++) {

			for (int j = 0; j < rockField[i].currentPosition.size(); j++) {

				glm::vec3& tempPosition = rockField[i].trackPositions[rockField[i].currentPosition[j]];
				push.modelMatrix[3].x = tempPosition.x;
				push.modelMatrix[3].y = tempPosition.y;
				push.modelMatrix[3].z = tempPosition.z;

				pipe->PushAndDraw(&push);
				//ockCount++;
				//std::cout << "post draw simple" << std::endl;
			}
		}
 
	}
	void FloatingRock::InitComputeData(){
		//descriptor set
		EWEDescriptorSetLayout::Builder dslBuilder{};
		dslBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		dslBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		dslBuilder.AddBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		auto tempDSL = dslBuilder.Build();

		{
			EWEDescriptorWriter descWriter{ tempDSL, DescriptorPool_Global };
			descWriter.WriteBuffer(0, rockBuffer->DescriptorInfo());
			descWriter.WriteBuffer(1, trackBuffer->DescriptorInfo());
			descWriter.WriteBuffer(2, transformBuffer[0]->DescriptorInfo());
			compDescriptorSet[0] = descWriter.Build();
		}
		{
			EWEDescriptorWriter descWriter{ tempDSL, DescriptorPool_Global };
			descWriter.WriteBuffer(0, rockBuffer->DescriptorInfo());
			descWriter.WriteBuffer(1, trackBuffer->DescriptorInfo());
			descWriter.WriteBuffer(2, transformBuffer[1]->DescriptorInfo());
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

		VkDescriptorSetLayout tempVkDSL[2] = {
			DescriptorHandler::GetDescSetLayout(LDSL_global),
			tempDSL->GetDescriptorSetLayout()
		};

		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = tempVkDSL;

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