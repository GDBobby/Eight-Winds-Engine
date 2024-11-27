#pragma once
#include <EWEngine/imgui/imgui.h>
#include <EWEngine/Data/TransformInclude.h>
#include <EWEngine/Data/EngineDataTypes.h>

#include <EWEngine/Graphics/Device.hpp>

#include "../../Systems/TileMapDevelopment.h"

#include <queue>

namespace EWE {
	class LevelCreationIMGUI {
	public:
		enum Tool_Enum : uint16_t {
			Tool_pencil,
			Tool_eraser,
			Tool_colorSelection,
			Tool_bucketFill,

			Tool_count,
		};
		Tool_Enum selectedTool = Tool_pencil;

		LevelCreationIMGUI(std::queue<uint16_t>& clickReturns, float screenWidth, float screenHeight);

		void render();
		void loadTextures(EWEDevice& device);

        void ShowMainMenuBar();
		void ShowMenuFile();
		void ShowGridControl();
		void ShowTileSet();

		void ShowSaveLevelPrompt();
		void ShowLoadLevelPrompt();
		void ShowNewPrompt();

		void ShowToolControls();

		//void addTextures();

		uint32_t tileSize = 32;

		bool showGrid = false;

		glm::vec2* gridZoom = nullptr;
		glm::vec2* gridScale = nullptr;
		glm::vec2* gridTrans = nullptr;
		float scaleLow = 0.f;
		float scaleHigh = 2.f;
		float transLow = -100.f;
		float transHigh = 100.f;
		float tileSetScale = 128.f;
		float tileSetRatio = 1.f;

		//float tileSetScaleY = 1.f;
		float tileSetScaleLow = 128.f;
		float tileSetScaleHigh = 4096.f;

		float screenWidth;
		float screenHeight;

		TileID selectedTile = 0;
		ImVec2 selectedTileUVBR{0.f,0.f};
		ImVec2 selectedTileUVTL{1.f / 64.f, 1.f / 19.f};

		ImVec2 toolUV;
		ImVec2 toolUVBR;

		ImVec2 tileUVScaling = { 1.f / 64.f, 1.f / 19.f };

		bool hoveringTileSet = false;

		std::queue<uint16_t>& clickReturns;
		uint32_t toolSelectedTile = 0;

		struct ToolStruct {
			VkDescriptorSet texture;
			ImVec4 bgColor{ 0.f,0.f,0.f,1.f };
		};
		std::array<ToolStruct, Tool_count> tools;

		VkDescriptorSet tileSetDescriptor;
		ImVec4 selectedColor{ 1.f,1.f,1.f,1.f };
		ImVec4 idleColor{ 0.f,0.f,0.f,1.f };

		bool gridControlOpen = true;

		void mouseCallback(int button, int action);
		void scrollCallback(double yOffset);

		bool showCreateLevelMenu = false;
		bool showSaveLevelMenu = false;
		bool showLoadLevelMenu = false;

		int widthBuffer = 8;
		int heightBuffer = 8;

		void (*createButtonPtr)(uint16_t, uint16_t);

		char saveLocation[128] = "NewMap.ewm";
		char loadLocation[128] = "NewMap.ewm";

		TileMapDevelopment* tileMapD{ nullptr };

		void toolLeft(uint32_t clickedTilePosition, bool shiftKey, bool ctrlKey);

	};
}

