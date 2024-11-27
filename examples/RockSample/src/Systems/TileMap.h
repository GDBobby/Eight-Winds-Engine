#pragma once

#include <EWEngine/Graphics/Model/Model.h>

#include "TileSet.h"
#include <array>


namespace EWE {
	class TileMap {
	public:
		uint16_t width;
		uint16_t height;
		TileSet tileSet;
		TextureDesc texture;
		std::vector<TileFlag> tileFlags;

		TileMap(EWEDevice& device, std::string fileLocation, TileSet::TileSet_Enum tileSetID);
		TileMap(EWEDevice& device, std::vector<glm::vec4>& outVertices, std::vector<uint32_t>& indices, std::vector<glm::vec2>& tileUVs, TileSet::TileSet_Enum tileSetID);
		TileMap(EWEDevice& device, TileSet::TileSet_Enum tileSetID);

		virtual void buildTileMap(EWEDevice& device, std::string const& fileLocation,
			std::vector<glm::vec4>& outVertices, std::vector<uint32_t>& indices);
		virtual void buildTileMapByVertex(EWEDevice& device, std::vector<glm::vec4>& outVertices, std::vector<uint32_t>& indices, std::vector<glm::vec2>& tileUVs);

		void renderTiles(FrameInfo const& frameInfo);
	protected:
		std::unique_ptr<EWEBuffer> tileVertexBuffer{ nullptr };
		std::unique_ptr<EWEBuffer> tileIndexBuffer{ nullptr };
		std::unique_ptr<EWEBuffer> tileUVBuffer{ nullptr };
		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

		//virtual void buildTileSquare(uint32_t& tileID, TransformComponent& transform, glm::mat4& ret, glm::vec2& uvOffset);

		static std::array<uint32_t, 4> getIndices(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
		static std::array<uint32_t, 4> getIndices(uint32_t tilePosition, uint16_t width, uint16_t height);


		static void createTileVertices(std::vector<glm::vec4>& outVertices, int width, int height, float tileScale);
		static void createTileIndices(std::vector<uint32_t>& indices, int width, int height);
		static void findTileFlags(std::vector<TileFlag>& tileFlags, int width, int height);
		static void createTileUVs(std::vector<glm::vec2>& tileUVs, int width, int height, uint32_t tileWidth, uint32_t tileHeight);

		friend class TileMapDevelopment;
		friend class TileContainer;
	};
}
