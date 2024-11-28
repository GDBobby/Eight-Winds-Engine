#include "TileSet.h"

#include <EWEngine/Graphics/Model/Basic_Model.h>
#include <EWEngine/Graphics/Texture/Image_Manager.h>


namespace EWE {
	TileSet::TileSet(TileSet_Enum map_id) : setID{ map_id } {
		switch (map_id) {
			case TS_First: {
				tileSize = 32;
				width = 64;
				height = 19;
				tileScale = 0.5f;
				tileSetImage = Image_Manager::GetCreateImageID("textures/tileSet.png", false);
				
				bool notReady = true;

#if EWE_DEBUG
				uint64_t loopCount = 0;
#endif
				while (notReady) {
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					auto* descInfo = Image_Manager::GetDescriptorImageInfo(tileSetImage);
					if (descInfo->imageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
						notReady = false;
					}
#if EWE_DEBUG
					printf("looping : %zu\n", loopCount++);
#endif
				}

				grassTiles.push_back(476);
				break;
			}
			case TS_Selection: {
				tileSize = 16;
				width = 4;
				height = 4;
				tileScale = 0.5f;
				tileSetImage = Image_Manager::GetCreateImageID( "textures/tileCreation/borders.png", false);
				break;
			}
			default: {
				printf("trying to get variables for a map that doesn't exist \n");
				assert(false && "invalid map id");
				break;
			}
		}
		tileModel = Basic_Model::TileQuad3D(Queue::transfer, glm::vec2{ 1.f / width, 1.f / height });

	}
	TileSet::~TileSet() {
		Deconstruct(tileModel);
	}

	std::array<glm::vec2, 4> TileSet::getUVOffset(TileID tileID) {
		//clockwise, from top left

		std::array<glm::vec2, 4> offsets;

		//uint32_t pixelWidth = tileSize * width;
		//uint32_t pixelHeight = tileSize * height;
		uint32_t pixelLeft = tileSize * (tileID % width);
		uint32_t pixelRight = pixelLeft + tileSize;
		uint32_t pixelTop = tileSize * ((tileID - (tileID % width))) / width;
		uint32_t pixelBottom = pixelTop + tileSize;


		offsets[0].x = static_cast<float>(pixelLeft);
		offsets[0].y = static_cast<float>(pixelTop);

		offsets[2].x = static_cast<float>(pixelRight);
		offsets[2].y = static_cast<float>(pixelBottom);

		offsets[1].x = static_cast<float>(offsets[2].x);
		offsets[1].y = static_cast<float>(offsets[0].y);

		offsets[3].x = static_cast<float>(offsets[0].x);
		offsets[3].y = static_cast<float>(offsets[2].y);


		if (tileID & FLIPPED_HORIZONTALLY_FLAG) {
			std::swap(offsets[0], offsets[1]);
			std::swap(offsets[3], offsets[2]);
		}
		if ((tileID & FLIPPED_DIAGONALLY_FLAG)) {
			//1->4 or 0 -> 5? doesnt matter i guess
			std::swap(offsets[1], offsets[3]);
		}
		if (tileID & FLIPPED_VERTICALLY_FLAG) {
			std::swap(offsets[0], offsets[3]);
			std::swap(offsets[1], offsets[2]);
		}

		printf("uv offsets : (%.2f:%.2f)(%.2f:%.2f)(%.2f:%.2f)(%.2f:%.2f) \n", offsets[0].x, offsets[0].y, offsets[1].x, offsets[1].y, offsets[2].x, offsets[2].y, offsets[3].x, offsets[3].y);

		return offsets;
	}

	TileFlag TileSet::getTileFlag(TileID tileID) {
		switch (tileID) {
		case 477: {
			return TileFlag_solid;
		}
		case 86: {
			return TileFlag_exit1;
		}
		default: {
			return TileFlag_none;
		}
		}
	}
}