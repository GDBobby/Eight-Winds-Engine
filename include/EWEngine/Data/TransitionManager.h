#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>


namespace EWE {
	/*
	* the current design is built around the current transfer thread structure
	* 	the main transfer thread stages transfers
	* 	an async transfer thread submits and waits on the transfer
	*	
	
	*/

	struct ImageQueueTransitionData {
		VkImage image;
		bool generateMips;
		uint32_t dstQueue;
		ImageQueueTransitionData(VkImage image, bool mips, uint32_t dstQueue) : image{image}, generateMips{mips}, dstQueue{dstQueue} {}
	};
	struct BufferQueueTransitionData{
		VkBuffer buffer;
		uint32_t dstQueue;
		BufferQueueTransitionData(VkBuffer buffer, uint32_t dstQueue) : buffer{buffer}, dstQueue{dstQueue} {}
	};

	struct ImageQueueTransitionContainer {
		std::vector<ImageQueueTransitionData> images{};
		std::vector<BufferQueueTransitionData> buffers{};
		VkSemaphore semaphore{VK_NULL_HANDLE};
		//i need a callback here, for images that the graphics pipeline is waiting on
		bool inFlight{false};
	};

	class Transition_Manager {
	public:
		//size is 1 for the staging transfer buffer, one for each transition that can be waiting on the graphics queue, one for each graphics frame that can be in flight
		explicit Transition_Manager(uint8_t max_pending_graphics, uint8_t max_frames_in_flight) : 
			transferFlightCount{max_pending_graphics}, 
			graphicsFlightCount{max_frames_in_flight}, 
			max_size{(uint8_t)(1 + transferFlightCount + graphicsFlightCount)}, 
			buffers{ new ImageQueueTransitionContainer[max_size] }, 
			stagingBuffer{buffers},
			transferInFlightBuffers{new ImageQueueTransitionContainer*[transferFlightCount]},
			graphicsInFlightBuffers{new ImageQueueTransitionContainer*[graphicsFlightCount]} 
		{ 
			// empty 
		}
		~Transition_Manager() {
			delete[] buffers;
			delete[] transferInFlightBuffers;
			delete[] graphicsInFlightBuffers;
		}

		ImageQueueTransitionContainer* GetStagingBuffer(){
			//the main transfer thread is going to keep track of whether or not this can be written to
			return stagingBuffer;
		}
		//this will be called in the main transfer thread, to determine if its possible to spawn the async transfer thread
		bool PrepareSubmission() {
			if(currentTransferInFlightCount >= transferFlightCount){
				return false;
			}

			for(uint16_t i = 0; i < max_size; i++){
				if((&buffers[i] != stagingBuffer) && (buffers[i].images.size() == 0)){

					std::lock_guard<std::mutex> lock(mut);
					transferInFlightBuffers[currentTransferInFlightCount] = stagingBuffer;
					transferInFlightBuffers[currentTransferInFlightCount]->inFlight = true;
					currentTransferInFlightCount++;
					stagingBuffer = &buffers[i];

					return true;
				}
			}
#ifdef _DEBUG
			assert(false && "buffers are being incorrectly handled in PrepareSubmission");
			return true;
#else
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
#endif
		}
		//this is performed in the async transfer thread
		void SignalTransferComplete(ImageQueueTransitionContainer* transferBuffer){

#ifdef _DEBUG
			for(uint8_t i = 0; i < currentTransferInFlightCount; i++){
				if(transferBuffer == transferInFlightBuffers[i]){
					transferBuffer->inFlight = false;
					return;
				}
			}

			assert(false && "buffers are being incorrectly handled in PrepareGraphics");
			return true;
#else
			transferBuffer->inFlight = false;
#endif
		}
		ImageQueueTransitionContainer* PrepareGraphics(uint8_t frameIndex){
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
			return ((stagingBuffer->images.size() == 0) && (currentTransferInFlightCount == 0));
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
		ImageQueueTransitionContainer* buffers;
		ImageQueueTransitionContainer* stagingBuffer;
		ImageQueueTransitionContainer** transferInFlightBuffers = nullptr;
		ImageQueueTransitionContainer** graphicsInFlightBuffers;
	};
} //namespace EWE

