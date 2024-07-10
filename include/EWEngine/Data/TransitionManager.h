#pragma once

#include "EWEngine/Data/EngineDataTypes.h"
#include "EWEngine/Global_Macros.h"
#include "EWEngine/Graphics/TransferCommandManager.h"

#include "EWEngine/Graphics/VulkanHeader.h"

#include <memory>
#include <mutex>
#include <vector>
#include <cassert>

namespace EWE {
	/*
	* the current design is built around the current transfer thread structure
	* 	the main transfer thread stages transfers
	* 	an async transfer thread submits and waits on the transfer
	*/

	struct ImageQueueTransitionData {
		VkImage image;
		uint8_t mipLevels;
		uint8_t arrayLayers;
		uint32_t dstQueue;
		StagingBuffer stagingBuffer;
		ImageQueueTransitionData(VkImage image, uint8_t mips, uint8_t arrayLayers, uint32_t dstQueue, StagingBuffer stagingBuffer) : 
			image{image}, 
			mipLevels{mips}, 
			arrayLayers{arrayLayers}, 
			dstQueue{dstQueue},
			stagingBuffer{stagingBuffer}
		{}
	};
	struct BufferQueueTransitionData{
		VkBuffer buffer;
		uint32_t dstQueue;
		StagingBuffer stagingBuffer;
		BufferQueueTransitionData(VkBuffer buffer, uint32_t dstQueue, StagingBuffer stagingBuffer) :
			buffer{buffer}, 
			dstQueue{dstQueue},
			stagingBuffer{ stagingBuffer } 
		{}
	};
	struct TransitionBarrierData {
		PipelineBarrier barrier;
		std::vector<StagingBuffer> stagingBuffers{};
		TransitionBarrierData(PipelineBarrier barrier, StagingBuffer stagingBuffer) :
			barrier{ barrier }
		{}
		TransitionBarrierData(PipelineBarrier barrier) :
			barrier{ barrier }
		{}
	};

	struct QueueTransitionContainer {
		std::vector<ImageQueueTransitionData> images{};
		std::vector<BufferQueueTransitionData> buffers{};
		VkSemaphore semaphore;
		//i need a callback here, for images that the graphics pipeline is waiting on
		bool inFlight{false};
		void DestroyImageStagingBuffers(VkDevice device) {
			for (auto& image : images) {
				image.stagingBuffer.Free(device);
			}
		}
		void CreateSemaphore(VkDevice device) {
			VkSemaphoreCreateInfo semaphoreCreateInfo{};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreCreateInfo.pNext = nullptr;
			semaphoreCreateInfo.flags = 0;
			EWE_VK_ASSERT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore));
		}
		void DestroySemaphore(VkDevice device) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}
		bool Empty() const {
			return (images.size() == 0) && (buffers.size() == 0);
		}
	};

	class Transition_Manager {
	public:
		//size is 1 for the staging transfer buffer, one for each transition that can be waiting on the graphics queue, one for each graphics frame that can be in flight
		explicit Transition_Manager(uint8_t max_pending_graphics, uint8_t max_frames_in_flight) :
			transferFlightCount{max_pending_graphics}, 
			graphicsFlightCount{max_frames_in_flight}, 
			max_size{(uint8_t)(1 + transferFlightCount + graphicsFlightCount)}, 
			transitionBuffers{ new QueueTransitionContainer[max_size] },
			transferStagingBuffer{ transitionBuffers},
			transferInFlightBuffers{new QueueTransitionContainer*[transferFlightCount]},
			graphicsInFlightBuffers{new QueueTransitionContainer*[graphicsFlightCount]}
		{}

		~Transition_Manager() {
			for (uint8_t i = 0; i < max_size; i++) {
				transitionBuffers[i].DestroySemaphore(vkDevice);
			}
			delete[] transitionBuffers;
			delete[] transferInFlightBuffers;
			delete[] graphicsInFlightBuffers;
		}

		void InitializeSemaphores(VkDevice vkDevice) {
			this->vkDevice = vkDevice;
			for (uint8_t i = 0; i < max_size; i++) {
				transitionBuffers[i].CreateSemaphore(vkDevice);
			}
		}

		QueueTransitionContainer* GetStagingBuffer(){
			//the main transfer thread is going to keep track of whether or not this can be written to
			return transferStagingBuffer;
		}
		//this will be called in the main transfer thread, to determine if its possible to spawn the async transfer thread
		QueueTransitionContainer* PrepareSubmission() {
			if(currentTransferInFlightCount >= transferFlightCount){
				return nullptr;
			}

			for(uint16_t i = 0; i < max_size; i++){
				bool bufferNotAvailable = false;
				bufferNotAvailable |= &transitionBuffers[i] == transferStagingBuffer;
				for(uint8_t j = 0; j < transferFlightCount; j++){
					bufferNotAvailable |= &transitionBuffers[i] == transferInFlightBuffers[j];
				}
				for(uint8_t j = 0; j < graphicsFlightCount; j++){
					bufferNotAvailable |= &transitionBuffers[i] == graphicsInFlightBuffers[j];
				}
				if(bufferNotAvailable){
					continue;
				}

				std::lock_guard<std::mutex> lock(mut);
				auto*& submissionBuffer = transferInFlightBuffers[currentTransferInFlightCount];

				submissionBuffer = transferStagingBuffer;
				submissionBuffer->inFlight = true;
				currentTransferInFlightCount++;
				transferStagingBuffer = &transitionBuffers[i];

				return submissionBuffer;
			
			}
#ifdef _DEBUG
			assert(false && "buffers are being incorrectly handled in PrepareSubmission");
			return nullptr;
#else
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
#endif
		}
		//this is performed in the async transfer thread
		void SignalTransferComplete(QueueTransitionContainer* transferBuffer){

#ifdef _DEBUG
			for(uint8_t i = 0; i < currentTransferInFlightCount; i++){
				if(transferBuffer == transferInFlightBuffers[i]){
					transferBuffer->inFlight = false;
					for (auto& buffers : transferBuffer->buffers) {
						buffers.stagingBuffer.Free(vkDevice);
					}
					for (auto& images : transferBuffer->images) {
						images.stagingBuffer.Free(vkDevice);
					}
					return;
				}
			}

			assert(false && "buffers are being incorrectly handled in PrepareGraphics");
			//return true;
#else
			transferBuffer->inFlight = false;
			for (auto& buffers : transferBuffer->buffers) {
				buffers.stagingBuffer.Free(vkDevice);
			}
			for (auto& images : transferBuffer->images) {
				images.stagingBuffer.Free(vkDevice);
			}
#endif
		}
		QueueTransitionContainer* PrepareGraphics(uint8_t frameIndex){
			//the synchronization of graphics frames is handled in the graphics thread

			for(uint8_t i = 0; i < currentTransferInFlightCount; i++) {
				if(transferInFlightBuffers[i]->inFlight == false) {
					
					std::lock_guard<std::mutex> lock(mut);

					graphicsInFlightBuffers[frameIndex] = transferInFlightBuffers[i];
					transferInFlightBuffers[i] = nullptr;
					currentTransferInFlightCount--;

					return graphicsInFlightBuffers[frameIndex];
				}

			}
			return nullptr;
		}

		bool Empty() const {
			return (transferStagingBuffer->Empty() && (currentTransferInFlightCount == 0));
		}
		bool TransferFull()  const {
			return transferFlightCount == currentTransferInFlightCount;
		}

	private:
		const uint8_t transferFlightCount;
		const uint8_t graphicsFlightCount;
		const uint8_t max_size;
		uint8_t currentTransferInFlightCount = 0;
		
		std::mutex mut;
		QueueTransitionContainer* transitionBuffers;
		QueueTransitionContainer* transferStagingBuffer;
		QueueTransitionContainer** transferInFlightBuffers = nullptr;
		QueueTransitionContainer** graphicsInFlightBuffers;

		VkDevice vkDevice;
	};
} //namespace EWE

