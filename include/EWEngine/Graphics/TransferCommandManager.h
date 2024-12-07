#pragma once

#include "EWEngine/Graphics/PipelineBarrier.h"
#include "EWEngine/Data/EngineDataTypes.h"
#include "EWEngine/Data/CommandCallbacks.h"

namespace EWE {

	namespace TransferCommandManager {

		bool Empty();
		TransferCommandCallbacks PrepareSubmit();
		void AddCommand(CommandBuffer& cmdBuf);
		void AddPropertyToCommand(PipelineBarrier& pipeBarrier);
		void AddPropertyToCommand(StagingBuffer* stagingBuffer);
		void AddPropertyToCommand(ImageInfo* imageInfo);
		void FinalizeCommand();
		//this requires the pointer to be deleted
	};
}//namespace EWE