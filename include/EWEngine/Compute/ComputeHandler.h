/*
#pragma once

#include "../graphics/EWE_Device.hpp"

namespace EWE {
class ComputeHandler {
private:
	EWEDevice& device;
	//VkSemaphore graphicsSemaphore = VK_NULL_HANDLE;
	bool computeStarted = false;

	//holding this for later, so that when computation ends, graphics can be notified
	//VkSemaphore* graphicsComputeSemaphore; 

	VkPipelineStageFlags waitStageMask{ VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };

	std::shared_ptr<SyncHub> syncHub;

public:

												//the semaphore pointer is to the graphic's compute semaphore. the second, semaphore object, is the graphic semaphore
	ComputeHandler(EWEDevice& device);

	void beginComputation() {
		syncHub->enableCompute();
	}

	void endComputation() {
		syncHub->disableCompute();
	}

	VkCommandBuffer beginCommandBuffer();
	void endCommandBuffer();
};
}
*/