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
#ifdef _DEBUG
			assert(!dataChunkTracker.any() && "improper memory bucket deconstruction");
#endif
			free(reservedMemory);
		}

		size_t GetRemainingSpace() {
			return dataChunkTracker.size() - dataChunkTracker.count();
		}
		void* GetDataChunk() {
			if (GetRemainingSpace() == 0) {
				return nullptr;
			}
			for (size_t i = 0; i < dataChunkTracker.size(); i++) {
				if (!dataChunkTracker[i]) {
					dataChunkTracker[i] = 1;
					return reinterpret_cast<char*>(reservedMemory) + elementSize * i;
				}
			}
			assert(false && "failed to find a data chunk in getDataChunk");
			return nullptr; //just to squash warnings/compiler errors
		}
		void FreeDataChunk(void* location) {
			size_t freeLocation = (reinterpret_cast<char*>(location) - reinterpret_cast<char*>(reservedMemory)) / elementSize;

#ifdef _DEBUG
			assert(dataChunkTracker[freeLocation] && "freeing data from bucket that wasn't allocated");
#endif

			dataChunkTracker[freeLocation] = 0;
		}

	};
}