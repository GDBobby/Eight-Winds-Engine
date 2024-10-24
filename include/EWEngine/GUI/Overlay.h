#pragma once

#include "EWEngine/GameObject2D.h"
#include "EWEngine/Graphics/TextOverlay.h"

namespace EWE {
	class OverlayBase {
	public:
		OverlayBase(float screenWidth, float screenHeight);

		virtual void drawText();

		bool getActive() {
			return isActive;
		}
		void setActive(bool activity) {
			isActive = activity;
		}

		virtual void drawObjects(FrameInfo const& frameInfo) = 0;
		void resizeWindow(std::pair<uint32_t, uint32_t> nextDims);
		
	protected:
		bool isActive = false;
		float screenWidth;
		float screenHeight;
		std::vector<TextStruct> textStructs{};
		std::vector<GameObject2D> gameObjects{};
	};
}