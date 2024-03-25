#pragma once

#include <cstdint>
#include <bitset>
#include <stdexcept>

#include "EWEngine/Data/EWE_Memory.h"

namespace EWE {
	template <std::size_t Size>
	class MemoryTypeBucket {
	private:
		std::bitset<Size> dataChunkTracker{};

		size_t elementSize;
		void* reservedMemory;
	public:

		MemoryTypeBucket(size_t elementSize) : elementSize{ elementSize } {
			reservedMemory = ewe_alloc(elementSize, dataChunkTracker.size());
			//reservedMemory = malloc(elementSize * dataChunkTracker.size());
		}

		~MemoryTypeBucket() {
			if (dataChunkTracker.any()) {
				printf("improper memory bucket deconstruction, items still exist \n");
				//throw std::runtime_error("deconstructing a memory bucket while elements inside are still allocated");
			}
			free(reservedMemory);
		}

		size_t getRemainingSpace() {
			return dataChunkTracker.size() - dataChunkTracker.count();
		}
		void* getDataChunk() {
			if (getRemainingSpace() == 0) {
				return nullptr;
			}
			for (size_t i = 0; i < dataChunkTracker.size(); i++) {
				if (!dataChunkTracker[i]) {
					dataChunkTracker[i] = 1;
					return reinterpret_cast<char*>(reservedMemory) + elementSize * i;
				}
			}
			printf("failed to find a data chunk in getDataChunk\n");
			throw std::runtime_error("failed to find a data chunk in getDataChunk");
			return nullptr; //just to squash warnings/compiler errors
		}
		void freeDataChunk(void* location) {
			size_t freeLocation = (reinterpret_cast<char*>(location) - reinterpret_cast<char*>(reservedMemory)) / elementSize;

#ifdef _DEBUG
			if (!dataChunkTracker[freeLocation]) {
				printf("freeing data from bucket that wasn't allocated\n");
				throw std::runtime_error("freeing data from bucket that wasn't allocated");
			}
#endif

			dataChunkTracker[freeLocation] = 0;
		}

	};
}