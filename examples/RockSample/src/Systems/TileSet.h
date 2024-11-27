#pragma once

#include <EWEngine/Data/EngineDataTypes.h>
#include <EWEngine/Data/TransformInclude.h>
#include <EWEngine/Graphics/Model/Model.h>

#include <stdexcept>
#include <string>
#include <vector>

typedef uint32_t TileID;
enum TileFlag : uint16_t {
	TileFlag_none,
	TileFlag_solid,
	TileFlag_slow,
	TileFlag_exit1,
	TileFlag_exit2,
	TileFlag_exit3,
	TileFlag_exit4,
	TileFlag_exit5,
	TileFlag_exit6,
};

namespace EWE {
	enum TileReadFlag : uint32_t {
		TILE_VOID_FLAG = 0x80000000,
		TILE_FLIPPED_HORIZONTALLY_FLAG = 0x40000000,
		TILE_FLIPPED_VERTICALLY_FLAG = 0x20000000,
		TILE_FLIPPED_DIAGONALLY_FLAG = 0x10000000,
	};

	const uint32_t FLIPPED_HORIZONTALLY_FLAG = 0x40000000;
	const uint32_t FLIPPED_VERTICALLY_FLAG = 0x20000000;
	const uint32_t FLIPPED_DIAGONALLY_FLAG = 0x10000000;

	struct TileSet {
		enum TileSet_Enum {
			TS_First,
			TS_Selection,
			//TS_Start,
		};

		TileSet_Enum setID;
		uint32_t tileSize;
		uint32_t width;
		uint32_t height;
		float tileScale;
		//std::string fileLocation;

		TileSet(TileSet_Enum map_id);
		~TileSet();

		std::array<glm::vec2, 4> getUVOffset(TileID tileID);

		TileFlag getTileFlag(TileID tileID);

		std::vector<uint32_t> grassTiles{};
		std::vector<uint32_t> solidTiles{};
		std::vector<uint32_t> exitTiles{};

		EWEModel* tileModel{ nullptr };
		ImageID tileSetImage = IMAGE_INVALID;
	};
}