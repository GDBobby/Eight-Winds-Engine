#pragma once

#include "EWEngine/Graphics/PipelineBarrier.h"
#include "EWEngine/Data/EngineDataTypes.h"
#include "EWEngine/Data/CommandCallbacks.h"

#define TRANSFER_COMMAND_DEPTH_MAXIMUM 10

#define SYNC_QUEUES false

namespace EWE {

	namespace TransferCommandManager {

		bool Empty();
		CommandCallbacks PrepareSubmit();
		void AddCommand(CommandBufferData& cmdBuf);
		void AddPropertyToCommand(PipelineBarrier& pipeBarrier);
		void AddPropertyToCommand(StagingBuffer* stagingBuffer);
		void AddPropertyToCommand(VkImage image, uint8_t mipLevels, uint32_t width, uint32_t height);
		void FinalizeCommand();
		//this requires the pointer to be deleted
	};
}//namespace EWE