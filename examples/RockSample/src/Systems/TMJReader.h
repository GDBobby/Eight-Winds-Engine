#pragma once

#include <string>

namespace EWE {

	typedef uint32_t TileID; //comment this later and move some data around

	struct TileMapData {
		uint64_t height;
		uint64_t width;

		uint8_t layerCount;
		//layer count should be equal to the size of the external vector
		//the size of the internal vector should be equal to wdith * height

		enum Layer_Ordering : uint8_t {
			Tile_Layer,
			Object_Layer,
		};
		std::vector<Layer_Ordering> layerOrdering{};

		std::vector<std::vector<TileID>> tileIDs{};

		struct ObjectData {
			enum ObjectType {
				Grapple_Anchor = 0,
				Trigger_Area,
				Exit_Area,
			};
			ObjectType objectType;
			uint32_t width;
			uint32_t height;
			uint32_t xPos;
			uint32_t yPos;
			float rotation;
		};
		std::vector<std::vector<ObjectData>> objectData{ };

		uint16_t pixelsPerTileWidth;
		uint16_t pixelsPerTileHeight;
		std::array<float, 2> spawnPoint{};
		std::array<float, 4> deathBox{};

	};

	namespace TMJReader {
		TileMapData ReadTMJ(std::string const& filePath);
	}
}

