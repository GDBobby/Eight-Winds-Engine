#pragma once

#include "TileSet.h"
#include "TileMap.h"

//typedef uint32_t TileID;

namespace EWE {

	class TileContainer {
	public:
		struct BufferInterface {
			EWEBuffer* bufferPtr;
			char* memory;
			uint64_t blockSize;
			BufferInterface(EWEBuffer* bufferPtr, uint64_t blockSize) : bufferPtr{ bufferPtr }, memory{ reinterpret_cast<char*>(bufferPtr->getMappedMemory()) }, blockSize{blockSize} {}
			
			void flush() {
				bufferPtr->flush();
			}/*
			void flushMin(uint32_t memIndex) {
				bufferPtr->flushMin(memIndex * blockSize);
			}*/

			void shiftRight(uint32_t memoryIndex, uint32_t shiftAmount, uint32_t shiftSize) {
				memmove(memory + (memoryIndex + shiftAmount) * blockSize, memory + (memoryIndex)*blockSize, shiftSize * blockSize);
			}
			void shiftLeft(uint32_t memoryIndex, uint32_t shiftAmount, uint32_t shiftSize) {
				memmove(memory + (memoryIndex)*blockSize, memory + (memoryIndex + shiftAmount) * blockSize, shiftSize * blockSize);
			}
			void copyData(uint32_t memoryIndex, void* data) {
				memcpy(memory + (memoryIndex * blockSize), data, blockSize);
			}
		};


		enum Buffer_Enum : uint8_t {
			Buffer_Index,
			Buffer_UV,
			Buffer_Size,
		};
	protected:
		BufferInterface indexBuffer;
		BufferInterface uvBuffer;

		//uint32_t memBlockCount{ 0 }; //this is the count of data blocks
		uint32_t instanceCount{ 0 };
		uint32_t size;
		uint16_t width;
		uint16_t height;

		TileID* tileData;
		uint32_t* posData;

		TileSet& tileSet;

	public:
		TileContainer(uint16_t width, uint16_t height, EWEBuffer* indexBufferPtr, EWEBuffer* uvBufferPtr, TileSet& tileSet);
		~TileContainer();

		uint32_t getSize() const {
			return size;
		}
		uint32_t getInstanceCount() const {
			return instanceCount;
		}
		TileID* getTileBuffer() const {
			return tileData;
		}

		//returns data block index
		int64_t find(uint32_t selectedTilePos);
		void removeTile(uint32_t selectedTilePos);
		void changeTile(uint32_t selectedTilePos, TileID changeTile);
		bool tryChangeTile(uint32_t selectedTilePos, TileID changeTile);

		void reset();
		void flipTile(uint32_t selectedTile, TileReadFlag flipFlag);

		void interpretLoadData(uint32_t* buffer);
		void reinterpretData();
		void bucketFill(uint32_t selectedTilePos, TileID fillTile);
		void selection(TileID* selectionData, uint32_t selTilePos);

	protected:
		//internal use only
		//checking should be done before entering to ensure the block being added does not currently exist
		void addTile(uint32_t selectedTilePos, bool flush);

		//buffer functions
		void insertTileInBuffers(uint32_t memPosition, bool flush);
		void addTileToBuffers(uint32_t memPosition, bool flush);
		void changeTileUVs(uint32_t tilePos, uint32_t memPosition);
		void changeTileUVNoFlush(uint32_t memPosition);
		void removeTileFromBuffers(uint32_t memPos);
		//int64_t findMemBlock(uint32_t selectedTilePos);

		void floodFillScanlineStack(int x, int y, TileID newTile, TileID oldTile);

		enum Borders : TileID {
			//if it has bordering tilesi n all directions its 0
			//if it only has tile to the right its 1, and so on
			B_none = 0,
			B_right = 1,
			B_bottom = 2,
			B_left = 4,
			B_top = 8,

			B_nLeft = 14,
			B_nTop = 13,
			B_nRight = 11,
			B_nBottom = 7,
			B_Full = 15,
		};
	};
}