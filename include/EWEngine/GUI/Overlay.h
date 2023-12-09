#pragma once

#include "../GameObject2D.h"
#include "../Graphics/TextOverlay.h"

namespace EWE {
	class OverlayBase {
	public:
		OverlayBase(std::shared_ptr<TextOverlay> textOverlay, float screenWidth, float screenHeight);

		virtual void drawText();

		bool getActive() {
			return isActive;
		}
		void setActive(bool activity) {
			isActive = activity;
		}

		virtual void drawObjects(std::pair<VkCommandBuffer, uint8_t> cmdIndexPair) = 0;
		void resizeWindow(std::pair<uint32_t, uint32_t> nextDims);
		
	protected:
		bool isActive = false;
		float screenWidth;
		float screenHeight;
		std::shared_ptr<TextOverlay> textOverlay{};
		std::vector<TextStruct> textStructs{};
		std::vector<GameObject2D> gameObjects{};
	};
}