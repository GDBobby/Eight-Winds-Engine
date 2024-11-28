#include "TileContainer.h"

#include <stack>

namespace EWE {
	TileContainer::TileContainer(uint16_t width, uint16_t height, EWEBuffer* indexBufferPtr, EWEBuffer* uvBufferPtr, TileSet& tileSet) : 
		width{ width }, height{ height }, size{ static_cast<uint32_t>(width) * static_cast<uint32_t>(height) },
		indexBuffer{ indexBufferPtr, sizeof(uint32_t) * 4}, 
		uvBuffer{uvBufferPtr, sizeof(glm::vec2) * 4}, 
		tileSet{tileSet},
		tileData{ new TileID[size] },
		posData{ new uint32_t[size]}
	{

		for (uint32_t i = 0; i < size; i++) {
			tileData[i] = TILE_VOID_FLAG;
		}
	}

	TileContainer::~TileContainer() {
		delete[] tileData;
		delete[] posData;
	}
	void TileContainer::reset() {
		//memBlockCount = 0;
		instanceCount = 0;
		for (uint32_t i = 0; i < size; i++) {
			tileData[i] = TILE_VOID_FLAG;
		}
	}

	void TileContainer::removeTile(uint32_t selectedTilePos) {
		int64_t foundTileMemPos = find(selectedTilePos);
		if (foundTileMemPos == -1) {
			printf("removing a tile that doesn't exist \n");
			return;
		}
		//memory needs to be searched, it will be done in this function
		if ((instanceCount - foundTileMemPos) > 1) {
			memmove(&posData[foundTileMemPos], &posData[foundTileMemPos + 1], sizeof(uint32_t) * (instanceCount - foundTileMemPos));
		}
		removeTileFromBuffers(foundTileMemPos);
		tileData[selectedTilePos] = TILE_VOID_FLAG;
	}

	void TileContainer::changeTile(uint32_t selectedTilePos, TileID changeTile) {

		//tileData[selectedTilePos] = TILE_VOID_FLAG;
		int64_t foundTileMemPos = find(selectedTilePos);
		tileData[selectedTilePos] = changeTile;
		if (foundTileMemPos == -1) {
			printf("adding tile, tilePos:memPos - %u:%ld \n", selectedTilePos, foundTileMemPos);
			addTile(selectedTilePos, true);
			return;
		}
		printf("changing tile, foundTileMemPos : %u \n", foundTileMemPos);
		tileData[selectedTilePos] = changeTile;
		changeTileUVs(selectedTilePos, foundTileMemPos);
		//updateData(selectedTilePos);
	}
	
	bool TileContainer::tryChangeTile(uint32_t selectedTilePos, TileID changeTile) {
		int64_t foundTileMemPos = find(selectedTilePos);
		if (foundTileMemPos == -1) {
			return false;
		}
		tileData[selectedTilePos] = changeTile;
		changeTileUVs(selectedTilePos, foundTileMemPos);
		return true;
		//buffers[Buffer_UV].update(tileID);

		//updateData(selectedTilePos);
	}

	void TileContainer::addTile(uint32_t selectedTilePos, bool flush) {
		
		for (uint32_t i = 0; i < instanceCount; i++) {
			if (posData[i] > selectedTilePos) {
				memmove(&posData[i + 1], &posData[i], sizeof(uint32_t) * (instanceCount - i));
				posData[i] = selectedTilePos;
				insertTileInBuffers(i, flush);
				
				return;
			}
		}
		posData[instanceCount] = selectedTilePos;
		addTileToBuffers(instanceCount, flush);
	}

	void TileContainer::changeTileUVs(uint32_t tilePos, uint32_t memPosition) {

		auto uvOffsets = tileSet.getUVOffset(tileData[tilePos]);
		if (memcmp(uvOffsets.data(), (uvBuffer.memory) + (memPosition * uvBuffer.blockSize), uvBuffer.blockSize) != 0) {
			uvBuffer.copyData(memPosition, uvOffsets.data());
			uvBuffer.flush();
		}
	}
	void TileContainer::changeTileUVNoFlush(uint32_t memPosition) {
		auto uvOffsets = tileSet.getUVOffset(tileData[posData[memPosition]]);
		if (memcmp(uvOffsets.data(), (uvBuffer.memory) + (memPosition * uvBuffer.blockSize), uvBuffer.blockSize) != 0) {
			uvBuffer.copyData(memPosition, uvOffsets.data());
		}
	}

	void TileContainer::insertTileInBuffers(uint32_t memPosition, bool flush) {
		printf("adding tile to buffers, memPosition, instanceCount - %u:%u \n", memPosition, instanceCount);
		if (instanceCount > memPosition) {
			indexBuffer.shiftRight(memPosition, 1, instanceCount - memPosition);
			uvBuffer.shiftRight(memPosition, 1, instanceCount - memPosition);
		}
		//the main theme is that if its at the end, no shift is required
		addTileToBuffers(memPosition, flush);

	}
	void TileContainer::addTileToBuffers(uint32_t memPosition, bool flush) {
		instanceCount++;

		auto indices = TileMap::getIndices(posData[memPosition], width, height);
		indexBuffer.copyData(memPosition, indices.data());
		auto uvOffsets = tileSet.getUVOffset(tileData[posData[memPosition]]);
		uvBuffer.copyData(memPosition, uvOffsets.data());
		if (flush) {
			indexBuffer.flush();
			uvBuffer.flush();
		}
	}

	int64_t TileContainer::find(uint32_t selectedTilePos) {
		printf("finding tile info \n");

		if (tileData[selectedTilePos] == TILE_VOID_FLAG) {
			return -1;
		}			
		for (uint32_t i = 0; i < instanceCount; i++) {
			if (posData[i] == selectedTilePos) {
				return i;
			}
		}
		printf("failed to find tile \n");
		assert(false && "failed to find element that is not TILE_VOID_FLAG");
		return -1;
	}

	void TileContainer::removeTileFromBuffers(uint32_t memPos) {
		//ive covered that it exists before entering this function

		if ((instanceCount - memPos) > 1) {
			indexBuffer.shiftLeft(memPos, 1, instanceCount - memPos);
			indexBuffer.flush();

			uvBuffer.shiftLeft(memPos, 1, instanceCount - memPos);
			uvBuffer.flush();
		}

		instanceCount--;

	}
	void TileContainer::flipTile(uint32_t selectedTile, TileReadFlag flipFlag) {
		tileData[selectedTile] ^= flipFlag;
		changeTile(selectedTile, tileData[selectedTile]);

	}
	void TileContainer::interpretLoadData(uint32_t* buffer) {
		memcpy(tileData, buffer, size * 4);
		reinterpretData();
	}
	void TileContainer::reinterpretData() {
		uint32_t currentPos = 0;
		uint32_t memPos = 0;
		instanceCount = 0;

		while (currentPos < size) {
			if ((tileData[currentPos] & TILE_VOID_FLAG) == 0) {
				posData[memPos] = currentPos;
				addTileToBuffers(memPos, false);
				memPos++;
			}

			currentPos++;
		}
		indexBuffer.flush();
		uvBuffer.flush();
	}


	void TileContainer::bucketFill(uint32_t selectedTilePosition, TileID fillTile) {
		TileID oldTile = tileData[selectedTilePosition];
		printf("oldTile:fillTile - %u : %u\n", oldTile, fillTile);
		if (oldTile == fillTile) return;

		int x = selectedTilePosition % width;
		int y = (selectedTilePosition - (selectedTilePosition % width)) / width;
		floodFillScanlineStack(x, y, fillTile, oldTile);
		//printf("after filling %d \n", amountChanged);
		//uint32_t memPos = 0;

		if ((oldTile & TILE_VOID_FLAG) == 0) {

			//
			for (uint32_t i = 0; i < instanceCount; i++) {
				if (tileData[posData[i]] == fillTile) {
					changeTileUVNoFlush(i);
				}
			}
			uvBuffer.flush();
		}
		else {

			reinterpretData();

		}
	}
	void TileContainer::selection(TileID* selectionData, uint32_t selTilePos) {
		TileID selectTile = tileData[selTilePos];
		int x = selTilePos % width;
		int y = (selTilePos - (selTilePos % width)) / width;

		selectionData[y * width + x] = B_none;
		
		
		auto push = [](std::stack<int>& stack, int x, int y) {
			stack.push(x);
			stack.push(y);
			};
		auto pop = [](std::stack<int>& stack, int& x, int& y) {
			if (stack.size() < 2) return false; // it's empty
			y = stack.top();
			stack.pop();
			x = stack.top();
			stack.pop();
			return true;
			};

		int x1;
		bool spanAbove, spanBelow;

		std::stack<int> theStack;
		push(theStack, x, y);
		while (pop(theStack, x, y)) {
			x1 = x;
			while ((x1 >= 0) && (tileData[y * width + x1] == selectTile)) {
				x1--;
			}
			x1++;

			spanBelow = 0;
			spanAbove = 0;
			while ((x1 < width) && (tileData[y * width + x1] == selectTile)) {
				//tileData[y * width + x1] = fillTile;
				TileID& currentSelectTile{ selectionData[y * width + x1] };
				if (currentSelectTile == TILE_VOID_FLAG) {
					currentSelectTile = B_none;
				}

				if (x1 >= 1 && tileData[y * width + x1 - 1] == selectTile) {
					currentSelectTile |= B_left;
					selectionData[y * width + x1 - 1] |= B_right;
				}


				if (y > 0) {
					if (selectionData[(y - 1) * width + x1] == TILE_VOID_FLAG) {
						if (tileData[(y - 1) * width + x1] == selectTile) {
							if (!spanAbove) {
								push(theStack, x1, y - 1);
								spanAbove = true;
							}
							selectionData[(y - 1) * width + x1] = B_bottom;
							currentSelectTile |= B_top;
						}
						else if (spanAbove && (tileData[(y - 1) * width + x1] != selectTile)) {
							spanAbove = false;
						}
					}
					else {
						selectionData[(y - 1) * width + x1] |= B_bottom;
					}
				}

				if (y < (height - 1)){
					if (selectionData[(y + 1) * width + x1] == TILE_VOID_FLAG) {
						if (tileData[(y + 1) * width + x1] == selectTile) {
							if (!spanBelow) {
								push(theStack, x1, y + 1);
								spanBelow = true;
							}
							selectionData[(y + 1) * width + x1] = B_top;
							currentSelectTile |= B_bottom;
						}
						else if (spanBelow && (tileData[(y + 1) * width + x1] != selectTile)) {
							spanBelow = false;
						}
					}
					else {
						selectionData[(y + 1) * width + x1] |= B_top;
					}
				}
				x1++;
			}
		}
		
	}

	void TileContainer::floodFillScanlineStack(int x, int y, TileID fillTile, TileID oldTile) {

		auto push = [](std::stack<int>& stack, int x, int y) {
				stack.push(x);
				stack.push(y);
			};
		auto pop = [](std::stack<int>& stack, int& x, int& y) {
				if (stack.size() < 2) return false; // it's empty
				y = stack.top();
				stack.pop();
				x = stack.top();
				stack.pop();
				return true;
			};

		int x1;
		bool spanAbove, spanBelow;

		std::stack<int> theStack;
		push(theStack, x, y);
		while (pop(theStack, x, y)) {
			x1 = x;
			while ((x1 >= 0) && (tileData[y * width + x1] == oldTile)) {
				x1--;
			}
			x1++;
			spanBelow = 0;
			spanAbove = 0;
			while ((x1 < width) && (tileData[y * width + x1] == oldTile)) {
				tileData[y * width + x1] = fillTile;

				if (!spanAbove && y > 0 && tileData[(y - 1) * width + x1] == oldTile) {
					push(theStack, x1, y - 1);
					spanAbove = true;
				}
				else if (spanAbove && y > 0 && tileData[(y - 1) * width + x1] != oldTile) {
					spanAbove = false;
				}
				if (!spanBelow && (y < (height - 1)) && (tileData[(y + 1) * width + x1] == oldTile)) {
					push(theStack, x1, y + 1);
					spanBelow = true;
				}
				else if (spanBelow && (y < (height - 1)) && (tileData[(y + 1) * width + x1] != oldTile)) {
					spanBelow = false;
				}
				x1++;
			}
		}
	}
}