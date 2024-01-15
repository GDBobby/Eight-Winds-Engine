#include "EWEngine/Systems/Rendering/Skin/SkinBufferHandler.h"

namespace EWE {
	SkinBufferHandler::SkinBufferHandler(EWEDevice& device, uint16_t boneCount, uint8_t maxActorCount) : boneBlockSize{ static_cast<uint32_t>(boneCount * sizeof(glm::mat4)) }, maxActorCount{ maxActorCount } {
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			gpuData.emplace_back(device, maxActorCount, boneBlockSize);
		}
	}
	SkinBufferHandler::SkinBufferHandler(uint8_t maxActorCount, std::vector<InnerBufferStruct>* innerPtr) : boneBlockSize{0}, maxActorCount{ maxActorCount } {
		gpuReference = innerPtr;
	}
	void SkinBufferHandler::writeData(void* finalBoneMatrices) {
		gpuData[frameIndex].bone->writeToBuffer(finalBoneMatrices, boneBlockSize, boneMemOffset);
		boneMemOffset += boneBlockSize;

	}
	void SkinBufferHandler::changeMaxActorCount(EWEDevice& device, uint8_t actorCount) {
		if (maxActorCount == actorCount) {
			return;
		}
		maxActorCount = actorCount;
		for (int i = 0; i < gpuData.size(); i++) {
			gpuData[i].changeActorCount(device, maxActorCount, boneBlockSize);
		}
	}
	SkinBufferHandler::InnerBufferStruct::InnerBufferStruct(EWEDevice& device, uint8_t maxActorCount, uint32_t boneBlockSize) :
		descriptor{},
		currentActorCount{maxActorCount}
	{
		bone = std::make_unique<EWEBuffer>(device, boneBlockSize * maxActorCount, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		bone->map();


		buildDescriptor();
	}
	void SkinBufferHandler::InnerBufferStruct::changeActorCount(EWEDevice& device, uint8_t maxActorCount, uint32_t boneBlockSize) {
		if (maxActorCount == currentActorCount) {
			return;
		}
		currentActorCount = maxActorCount;
		if (maxActorCount > 5) {
			printf("currently not supporting non-instanced actor counts greater than 5 \n");
			throw std::runtime_error("currently not supporting non-instanced actor counts greater than 5");
		}
		bone.reset(new EWEBuffer(device, boneBlockSize * maxActorCount, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
		bone->map();
		EWEDescriptorPool::freeDescriptor(DescriptorPool_Global, &descriptor);

		buildDescriptor();
	}
	void SkinBufferHandler::InnerBufferStruct::buildDescriptor() {
		printf("building skin buffer \n");
		if (!
			EWEDescriptorWriter(DescriptorHandler::getLDSL(LDSL_boned), DescriptorPool_Global)
			.writeBuffer(0, bone->descriptorInfo())
			.build(descriptor)
			) {
			printf("monster desc failure \n");
			throw std::runtime_error("failed to create monster descriptor set");
		}
	}



	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ INSTANCING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	InstancedSkinBufferHandler::InstancedSkinBufferHandler(EWEDevice& device, uint16_t boneCount, uint16_t maxActorCount) : boneBlockSize{ static_cast<uint32_t>(boneCount * sizeof(glm::mat4)) }, maxActorCount{ maxActorCount } {
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			gpuData.emplace_back(device, maxActorCount, boneBlockSize);
		}
	}
	void InstancedSkinBufferHandler::writeData(glm::mat4* modelMatrix, void* finalBoneMatrices) {

		gpuData[frameIndex].model->writeToBuffer(modelMatrix, sizeof(glm::mat4), modelMemOffset);
		gpuData[frameIndex].bone->writeToBuffer(finalBoneMatrices, boneBlockSize, boneMemOffset);
		modelMemOffset += sizeof(glm::mat4);
		boneMemOffset += boneBlockSize;
		currentInstanceCount++;

	}
	void InstancedSkinBufferHandler::changeMaxActorCount(EWEDevice& device, uint16_t actorCount) {
		if (maxActorCount == actorCount) {
			return;
		}
		maxActorCount = actorCount;
		for (int i = 0; i < gpuData.size(); i++) {
			gpuData[i].changeActorCount(device, maxActorCount, boneBlockSize);
		}
	}

	void InstancedSkinBufferHandler::flush() {
		//printf("flushing \n");
		gpuData[frameIndex].flush();
		modelMemOffset = 0;
		boneMemOffset = 0;
	}

	InstancedSkinBufferHandler::InnerBufferStruct::InnerBufferStruct(EWEDevice& device, uint16_t maxActorCount, uint32_t boneBlockSize)
	{
		//if (maxActorCount > 1000) {
			model = std::make_unique<EWEBuffer>(device, sizeof(glm::mat4) * maxActorCount, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		/*
		}
	
		else {
			model = std::make_unique<EWEBuffer>(device, sizeof(glm::mat4) * maxActorCount, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		}
		*/
		bone = std::make_unique<EWEBuffer>(device, boneBlockSize * maxActorCount, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		model->map();
		bone->map();


		buildDescriptor(maxActorCount);
	}

	void InstancedSkinBufferHandler::InnerBufferStruct::changeActorCount(EWEDevice& device, uint16_t maxActorCount, uint32_t boneBlockSize) {

		//if (maxActorCount > 1000) {
			model.reset(new EWEBuffer(device, sizeof(glm::mat4) * maxActorCount, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
			/*
		}
		else {
			model.reset(new EWEBuffer(device, sizeof(glm::mat4) * maxActorCount, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
		}
		*/
		bone.reset(new EWEBuffer(device, boneBlockSize * maxActorCount, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

		EWEDescriptorPool::freeDescriptor(DescriptorPool_Global, &descriptor);

		buildDescriptor(maxActorCount);
	}
	void InstancedSkinBufferHandler::InnerBufferStruct::buildDescriptor(uint16_t maxActorCount) {
		//if (maxActorCount > 1000) {
		printf("building instanced skin buffer \n");
			if (!
				EWEDescriptorWriter(DescriptorHandler::getLDSL(LDSL_largeInstance), DescriptorPool_Global)
				.writeBuffer(0, model->descriptorInfo())
				.writeBuffer(1, bone->descriptorInfo())
				//.writeBuffer(1, &buffers[i][2]->descriptorInfo())
				.build(descriptor)
				) {
				printf("monster desc failure \n");
				throw std::runtime_error("failed to create monster descriptor set");
			}
			/*
		}
		else {
			if (!
				EWEDescriptorWriter(DescriptorHandler::getLDSL(LDSL_smallInstance), DescriptorPool_Global)
				.writeBuffer(0, &model->descriptorInfo())
				.writeBuffer(1, &bone->descriptorInfo())
				//.writeBuffer(1, &buffers[i][2]->descriptorInfo())
				.build(descriptor)
				) {
				printf("monster desc failure \n");
				throw std::exception("failed to create monster descriptor set");
			}
		}
		*/
	}

	void InstancedSkinBufferHandler::InnerBufferStruct::flush() {
		if (updated) {
			model->flush();
			bone->flush();
			updated = false;
		}
	}
}