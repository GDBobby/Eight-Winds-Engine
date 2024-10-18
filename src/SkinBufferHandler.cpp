#include "EWEngine/Systems/Rendering/Skin/SkinBufferHandler.h"

namespace EWE {
	SkinBufferHandler::SkinBufferHandler(uint16_t boneCount, uint8_t maxActorCount) : boneBlockSize{ static_cast<uint32_t>(boneCount * sizeof(glm::mat4)) }, maxActorCount{ maxActorCount } {
		gpuData.reserve(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			gpuData.emplace_back(maxActorCount, boneBlockSize);
		}
	}
	SkinBufferHandler::SkinBufferHandler(uint8_t maxActorCount, std::vector<InnerBufferStruct>* innerPtr) : boneBlockSize{0}, maxActorCount{ maxActorCount } {
		gpuReference = innerPtr;
	}
	void SkinBufferHandler::WriteData(void* finalBoneMatrices) {
		gpuData[frameIndex].bone->WriteToBuffer(finalBoneMatrices, boneBlockSize, boneMemOffset);
		boneMemOffset += boneBlockSize;

	}
	void SkinBufferHandler::ChangeMaxActorCount(uint8_t actorCount) {
		if (maxActorCount == actorCount) {
			return;
		}
		maxActorCount = actorCount;
		for (int i = 0; i < gpuData.size(); i++) {
			gpuData[i].ChangeActorCount(maxActorCount, boneBlockSize);
		}
	}
	void SkinBufferHandler::Flush() {
		gpuData[frameIndex].Flush();
		boneMemOffset = 0;
	}
	void SkinBufferHandler::SetFrameIndex(uint8_t frameIndex) {
		this->frameIndex = frameIndex;

	}
	VkDescriptorSet* SkinBufferHandler::GetDescriptor() {
		if (gpuReference == nullptr) {
			return &gpuData[frameIndex].descriptor;
		}
		else {
			return &gpuReference->at(frameIndex).descriptor;
		}
	}
	SkinBufferHandler::InnerBufferStruct::InnerBufferStruct(uint8_t maxActorCount, uint32_t boneBlockSize) :
		currentActorCount{maxActorCount}
	{
		bone = Construct<EWEBuffer>({ boneBlockSize * maxActorCount, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT });
		bone->Map();


		BuildDescriptor();
	}
	void SkinBufferHandler::InnerBufferStruct::ChangeActorCount(uint8_t maxActorCount, uint32_t boneBlockSize) {
		if (maxActorCount == currentActorCount) {
			return;
		}
		currentActorCount = maxActorCount;
		assert(maxActorCount <= 5 && "currently not supporting non-instanced actor counts greater than 5");

		bone->Reconstruct(boneBlockSize * maxActorCount, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, &descriptor);

		BuildDescriptor();
	}
	void SkinBufferHandler::InnerBufferStruct::BuildDescriptor() {
		//printf("building skin buffer \n");
		descriptor = EWEDescriptorWriter(DescriptorHandler::GetLDSL(LDSL_boned), DescriptorPool_Global)
			.WriteBuffer(0, bone->DescriptorInfo())
			.Build();
	}
	void SkinBufferHandler::InnerBufferStruct::Flush() {
		if (updated) {
			bone->Flush();
			bone->Unmap();
			updated = false;
		}
	}



	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ INSTANCING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	InstancedSkinBufferHandler::InstancedSkinBufferHandler(uint16_t boneCount, uint16_t maxActorCount) : boneBlockSize{ static_cast<uint32_t>(boneCount * sizeof(glm::mat4)) }, maxActorCount{ maxActorCount } {
		gpuData.reserve(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			gpuData.emplace_back(maxActorCount, boneBlockSize);
		}
	}
	void InstancedSkinBufferHandler::WriteData(glm::mat4* modelMatrix, void* finalBoneMatrices) {

		gpuData[frameIndex].model->WriteToBuffer(modelMatrix, sizeof(glm::mat4), modelMemOffset);
		gpuData[frameIndex].bone->WriteToBuffer(finalBoneMatrices, boneBlockSize, boneMemOffset);
		modelMemOffset += sizeof(glm::mat4);
		boneMemOffset += boneBlockSize;
		currentInstanceCount++;

	}
	void InstancedSkinBufferHandler::ChangeMaxActorCount(uint16_t actorCount) {
		if (maxActorCount == actorCount) {
			return;
		}
		maxActorCount = actorCount;
		for (int i = 0; i < gpuData.size(); i++) {
			gpuData[i].ChangeActorCount(maxActorCount, boneBlockSize);
		}
	}

	void InstancedSkinBufferHandler::Flush() {
		//printf("flushing \n");
		gpuData[frameIndex].Flush();
		modelMemOffset = 0;
		boneMemOffset = 0;
	}

	InstancedSkinBufferHandler::InnerBufferStruct::InnerBufferStruct(uint16_t maxActorCount, uint32_t boneBlockSize) {

		model = Construct<EWEBuffer>({ sizeof(glm::mat4) * maxActorCount, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT });

		bone = Construct<EWEBuffer>({ boneBlockSize * maxActorCount, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT });

		model->Map();
		bone->Map();


		BuildDescriptor(maxActorCount);
	}

	void InstancedSkinBufferHandler::InnerBufferStruct::ChangeActorCount(uint16_t maxActorCount, uint32_t boneBlockSize) {
		model->Reconstruct(sizeof(glm::mat4) * maxActorCount, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		bone->Reconstruct(boneBlockSize * maxActorCount, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, &descriptor);

		BuildDescriptor(maxActorCount);
	}
	void InstancedSkinBufferHandler::InnerBufferStruct::BuildDescriptor(uint16_t maxActorCount) {
		//if (maxActorCount > 1000) {
		printf("building instanced skin buffer \n");
		descriptor = EWEDescriptorWriter(DescriptorHandler::GetLDSL(LDSL_largeInstance), DescriptorPool_Global)
			.WriteBuffer(0, model->DescriptorInfo())
			.WriteBuffer(1, bone->DescriptorInfo())
			//.writeBuffer(1, &buffers[i][2]->descriptorInfo())
			.Build();
	}

	void InstancedSkinBufferHandler::InnerBufferStruct::Flush() {
		if (updated) {
			model->Flush();
			bone->Flush();
			updated = false;
		}
	}
}