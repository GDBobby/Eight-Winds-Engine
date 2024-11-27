#pragma once
#include "TileMap.h"
#include "../pipelines/PipelineEnum.h"
#include "TileContainer.h"

#include <queue>

namespace EWE {
	class TileMapDevelopment {
	public:
		TileMapDevelopment(EWEDevice& device, uint16_t width, uint16_t height);
		~TileMapDevelopment();

		void refreshMap(uint16_t width, uint16_t height);
		void renderTiles();

		std::array<float, 6> const& getScreenCoordinates(float screenWidth, float screenHeight) {
			if (refreshedMap) {
				refreshedMap = false;
				return updateScreenCoordinates(screenWidth, screenHeight);
			}
			printf("screenCoordinates : \n\t");
			for (int i = 0; i < screenCoordinates.size(); i++) {
				printf("(%.2f)", screenCoordinates[i]);
			}
			printf("\n");
			return screenCoordinates;
		}

		int64_t getClickedTile(float clickX, float clickY, float screenWidth, float screenHeight);

		void changeTile(uint32_t tilePosition, TileID tileID);
		void removeTile(uint32_t tilePosition);

		void fitToScreen();
		PushTileConstantData pushTile{};

		bool saveMap(std::string saveLocation);
		bool loadMap(std::string loadLocation);

		void clearSelection();
		void colorSelection(uint32_t selectPosition);
		void bucketFill(uint32_t clickedTilePosition, TileID selectedTile) {
			tileContainer->bucketFill(clickedTilePosition, selectedTile);
		}

		bool refreshedMap = true;
	protected:
		//std::vector<glm::vec4> vertices{}; //i dont think i need to store vertices, but ok

		TileSet tileSet;
		TileSet selectionTileSet;

		struct TileInfo {
			uint64_t memoryLocation;
			TileID tileID;
			TileInfo(uint64_t memLoc, TileID tileID) : memoryLocation(memLoc), tileID(tileID) {}
		};

		TileContainer* tileContainer;
		TileContainer* selectionContainer;
		//std::map<uint32_t, TileInfo> tileIndexingMap{}; //tilePositionID, indices/uv memLocation

		//uint64_t instanceCount = 0;
		uint16_t width, height;

		std::array<float, 6> screenCoordinates{};

		EWEBuffer* tileVertexBuffer{ nullptr };
		EWEBuffer* tileIndexBuffer{ nullptr };
		EWEBuffer* tileUVBuffer{ nullptr };
		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

		EWEBuffer* selectionIndexBuffer{ nullptr };
		EWEBuffer* selectionUVBuffer{ nullptr };
		VkDescriptorSet selectionDescSet{ VK_NULL_HANDLE };


		void init(EWEDevice& device);
		void constructVertices(EWEDevice& device, float tileScale);
		void constructUVsAndIndices(EWEDevice& device);
		void deconstructMap();
		void constructDescriptor();

		std::array<float, 6> const& updateScreenCoordinates(float screenWidth, float screenHeight);

		std::unique_ptr<EWEBuffer> modelIndexBuffer;

		uint32_t borders = 0;


		EWEDevice& device;

	};
}

