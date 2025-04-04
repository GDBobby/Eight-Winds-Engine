#include "FloatingRockSystem.h"
#include <EWEngine/Systems/Rendering/Pipelines/Pipe_SimpleTextured.h>
#include <EWEngine/Systems/Rendering/Rigid/RigidRS.h>
#include "EWEngine/Data/EWE_Import.h"

namespace EWE {
	static constexpr uint32_t rock_count = 32 * 32;
#pragma pack(push, 1)
	struct RockData {
		float scale;
		float azimuthalRotation;
		float radius;
		float speed;
	};
#pragma pack(pop)

	static void InitRockData(std::vector<RockData>& rockData, std::default_random_engine& randomGen) {

		std::uniform_real_distribution<float> rotationalDistribution(-2.f, 2.f);
		std::uniform_real_distribution<float> scaleDist(0.25f, 0.75f);
		std::uniform_real_distribution<float> aziDist(0.f, glm::two_pi<float>());
		std::uniform_real_distribution<float> speedDist(0.01f, 0.1f);
		std::uniform_real_distribution<float> radiusDist(30.f, 35.f);
		//std::uniform_real_distribution<float> trackRatioDist(0.0f, 1.0f);

		for (auto& rock : rockData) {
			rock.scale = scaleDist(randomGen);

			rock.azimuthalRotation = aziDist(randomGen);
			rock.radius = radiusDist(randomGen);
			rock.speed = speedDist(randomGen);
			//rock.trackRatio = trackRatioDist(randomGen);
		}
	}

	void InitBuffers(EWEBuffer*& transformRockBuffer, EWEModel* rockModel, std::vector<RockData>& rockData) {


		SyncHub* syncHub = SyncHub::GetSyncHubInstance();
		CommandBuffer& cmdBuf = syncHub->BeginSingleTimeCommandTransfer();
		EWEDevice* eweDevice = EWEDevice::GetEWEDevice();
		const std::size_t rockTransformBufferSize = rockData.size() * sizeof(RockData);
		StagingBuffer* rockStagingBuffer = Construct<StagingBuffer>({ rockTransformBufferSize });
		const std::size_t rockMaterialBufferSize = rockData.size() * sizeof(MaterialBuffer);
		StagingBuffer* rockMatStagingBuffer = Construct<StagingBuffer>({ rockMaterialBufferSize });

		{
			transformRockBuffer = Construct<EWEBuffer>({ rockTransformBufferSize, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT });

			rockStagingBuffer->Stage(rockData.data(), rockTransformBufferSize);

			eweDevice->CopyBuffer(cmdBuf, rockStagingBuffer->buffer, transformRockBuffer->GetBuffer(), rockTransformBufferSize);
		}
		{
			//materialRockBuffer = Construct<EWEBuffer>({ rockMaterialBufferSize, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT });
			const std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> materialBuffers = RigidRenderingSystem::GetBothMaterialBuffers(rockModel);

			MaterialBuffer matData;
			matData.albedo.r = 1.f;
			matData.albedo.g = 1.f;
			matData.albedo.b = 1.f;
			matData.albedo.a = 1.f;
			matData.metal = 0.f;
			matData.rough = 0.5f;

			rockMatStagingBuffer->Stage(rockData.data(), rockMaterialBufferSize);
			void* stagingData;
			rockMatStagingBuffer->Map(stagingData);
			for (uint64_t i = 0; i < rockData.size(); i++) {
				memcpy(reinterpret_cast<void*>(reinterpret_cast<std::size_t>(stagingData) + (i * sizeof(MaterialBuffer))), &matData, sizeof(MaterialBuffer));
			}
			rockMatStagingBuffer->Unmap();

			eweDevice->CopyBuffer(cmdBuf, rockMatStagingBuffer->buffer, materialBuffers[0]->GetBuffer(), rockMaterialBufferSize);
			eweDevice->CopyBuffer(cmdBuf, rockMatStagingBuffer->buffer, materialBuffers[1]->GetBuffer(), rockMaterialBufferSize);
		}

		TransferCommand transferCommand{};
		transferCommand.commands.push_back(&cmdBuf);
		transferCommand.stagingBuffers.push_back(rockStagingBuffer);
		transferCommand.stagingBuffers.push_back(rockMatStagingBuffer);

		syncHub->EndSingleTimeCommandTransfer(transferCommand);
	}


	FloatingRock::FloatingRock() : rockMaterial{ Material_Image::CreateMaterialImage("eye/", true, true) } {
		//rockModel = EWEModel::CreateModelFromFile("rock1.obj", Queue::transfer);
		rockMaterial.materialFlags |= Material::Instanced;

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

		rockModel = Construct<EWEModel>({ importMesh.meshes[0].vertices.data(), static_cast<uint32_t>(importMesh.meshes[0].vertices.size()), static_cast<uint32_t>(importMesh.vertex_size), importMesh.meshes[0].indices});

#if DEBUG_NAMING
		rockModel->SetDebugNames("eyeModel");
#endif

		//RANDOM NUMBER GENERATOR
		std::random_device r;
		std::default_random_engine randomGen(r());

		std::vector<RockData> rockData(rock_count);

		InitRockData(rockData, randomGen);

		RigidRenderingSystem::AddInstancedMaterialObject(rockMaterial, rockModel, rock_count, true);
		//buffers
		InitBuffers(rockTransformBuffer, rockModel, rockData);

		InitComputeData();
	}
	FloatingRock::~FloatingRock() {
		Deconstruct(rockModel);
		EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, compDSL, &compDescriptorSet[0]);
		EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, compDSL, &compDescriptorSet[1]);

		EWE_VK(vkDestroyPipeline, VK::Object->vkDevice, compPipeline, nullptr);
		EWE_VK(vkDestroyPipelineLayout, VK::Object->vkDevice, compPipeLayout, nullptr);
		EWE_VK(vkDestroyShaderModule, VK::Object->vkDevice, compShaderModule, nullptr);
	}
	void FloatingRock::Dispatch(float dt) {
		if (previouslySubmitted) {
			EWE_VK(vkCmdPipelineBarrier, VK::Object->GetFrameBuffer(),
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
				0,
				0, nullptr,
				1, &bufferBarrier[VK::Object->frameIndex + MAX_FRAMES_IN_FLIGHT],
				0, nullptr
			);
		}
		else {
			previouslySubmitted = true;
		}
		EWE_VK(vkCmdBindPipeline, VK::Object->GetFrameBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, compPipeline);
		EWE_VK(vkCmdBindDescriptorSets, VK::Object->GetFrameBuffer(),
			VK_PIPELINE_BIND_POINT_COMPUTE,
			compPipeLayout,
			0, 1,
			&compDescriptorSet[VK::Object->frameIndex],
			0, nullptr
		);

		compPushData.secondsSinceBeginning += dt;
		//compPushData.whichIndex = frameInfo.index;
		EWE_VK(vkCmdPushConstants, VK::Object->GetFrameBuffer(), compPipeLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(RockCompPushData), &compPushData);

		EWE_VK(vkCmdDispatch, VK::Object->GetFrameBuffer(), 1, 1, 1);

		EWE_VK(vkCmdPipelineBarrier, VK::Object->GetFrameBuffer(),
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
			0,
			0, nullptr,
			1, &bufferBarrier[VK::Object->frameIndex],
			0, nullptr
		);
	}
	void FloatingRock::InitComputeData(){
		const std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> transformBuffers = RigidRenderingSystem::GetBothTransformBuffers(rockModel);

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
		dslBuilder.AddGlobalBindingForCompute();
		dslBuilder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		dslBuilder.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
		compDSL = dslBuilder.Build();

		for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			EWEDescriptorWriter descWriter{ compDSL, DescriptorPool_Global };
			DescriptorHandler::AddCameraDataToDescriptor(descWriter, i);
			descWriter.WriteBuffer(rockTransformBuffer->DescriptorInfo());
			descWriter.WriteBuffer(transformBuffers[i]->DescriptorInfo());
			compDescriptorSet[i] = descWriter.Build();
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


		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = compDSL->GetDescriptorSetLayout();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &compPipeLayout);

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

		EWE_VK(vkCreateComputePipelines, VK::Object->vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &compPipeline);
	}

}