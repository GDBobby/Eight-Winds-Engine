#include "EWEngine/Systems/Rendering/Skin/SkinBufferHandler.h"
#include "EWEngine/Graphics/Texture/Image_Manager.h"

namespace EWE {
	SkinBufferHandler::SkinBufferHandler(uint16_t boneCount, uint8_t maxActorCount) : boneBlockSize{ static_cast<uint32_t>(boneCount * sizeof(glm::mat4)) }, maxActorCount{ maxActorCount }, gpuData{ InnerBufferStruct{maxActorCount, boneBlockSize}, InnerBufferStruct{maxActorCount, boneBlockSize} } {
		/*
		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			EWEDescriptorWriter descWriter{eDSL, DescriptorPool_Global};
			DescriptorHandler::AddGlobalsToDescriptor(descWriter, i);
			descWriter.WriteImage(2, Image_Manager::GetDescriptorImageInfo(imgID));
			gpuData[i].AddDescriptorBindings(descWriter, 3);
			gpuData[i].descriptor = descWriter.Build();
		}
		*/
		
	}
	void SkinBufferHandler::WriteData(void* finalBoneMatrices) {
		gpuData[VK::Object->frameIndex].bone->WriteToBuffer(finalBoneMatrices, boneBlockSize, boneMemOffset);
		boneMemOffset += boneBlockSize;

	}
	void SkinBufferHandler::Flush() {
		gpuData[VK::Object->frameIndex].Flush();
		boneMemOffset = 0;
	}
	//VkDescriptorSet* SkinBufferHandler::GetDescriptor() {
	//	if (gpuReference == nullptr) {
	//		return &gpuData[VK::Object->frameIndex].descriptor;
	//	}
	//	else {
	//		return &gpuReference->at(VK::Object->frameIndex).descriptor;
	//	}
	//}


	SkinBufferHandler::InnerBufferStruct::InnerBufferStruct(uint8_t maxActorCount, uint32_t boneBlockSize) :
		currentActorCount{maxActorCount}
	{
		bone = Construct<EWEBuffer>({ boneBlockSize * maxActorCount, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT });
		bone->Map();
	}
	void SkinBufferHandler::InnerBufferStruct::AddDescriptorBindings(EWEDescriptorWriter& descWriter, uint8_t& currentBinding) {
		//printf("building skin buffer \n");
		descWriter.WriteBuffer(currentBinding++, bone->DescriptorInfo());
	}
	void SkinBufferHandler::InnerBufferStruct::Flush() {
		if (updated) {
			bone->Flush();
			bone->Unmap();
			updated = false;
		}
	}



	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ INSTANCING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	InstancedSkinBufferHandler::InstancedSkinBufferHandler(uint16_t boneCount, uint16_t maxActorCount) : boneBlockSize{ static_cast<uint32_t>(boneCount * sizeof(glm::mat4)) }, maxActorCount{ maxActorCount }, gpuData{ InnerBufferStruct{maxActorCount, boneBlockSize},InnerBufferStruct{maxActorCount, boneBlockSize} } {
		//for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		//	EWEDescriptorWriter descWriter{ eDSL, DescriptorPool_Global };
		//	DescriptorHandler::AddGlobalsToDescriptor(descWriter, i);
		//	descWriter.WriteImage(2, Image_Manager::GetDescriptorImageInfo(imgID));
		//	gpuData[i].AddDescriptorBindings(maxActorCount, descWriter, 3);
		//}
	}
	void InstancedSkinBufferHandler::WriteData(glm::mat4* modelMatrix, void* finalBoneMatrices) {

		gpuData[frameIndex].model->WriteToBuffer(modelMatrix, sizeof(glm::mat4), modelMemOffset);
		gpuData[frameIndex].bone->WriteToBuffer(finalBoneMatrices, boneBlockSize, boneMemOffset);
		modelMemOffset += sizeof(glm::mat4);
		boneMemOffset += boneBlockSize;
		currentInstanceCount++;

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
	}
	void InstancedSkinBufferHandler::InnerBufferStruct::AddDescriptorBindings(uint16_t maxActorCount, EWEDescriptorWriter& descWriter, uint8_t& currentBinding) {
		//if (maxActorCount > 1000) {
#if EWE_DEBUG
		//printf("building instanced skin buffer \n");
#endif
		descWriter.WriteBuffer(currentBinding++, model->DescriptorInfo());
		descWriter.WriteBuffer(currentBinding++, bone->DescriptorInfo());
	}

	void InstancedSkinBufferHandler::InnerBufferStruct::Flush() {
		if (updated) {
			model->Flush();
			bone->Flush();
			updated = false;
		}
	}
}