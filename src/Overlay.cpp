#include "EWEngine/GUI/Overlay.h"

namespace EWE {
	OverlayBase::OverlayBase(float screenWidth, float screenHeight) : screenWidth{ screenWidth }, screenHeight{screenHeight} {

	}
	void OverlayBase::drawText() {
		if (isActive) {
			for (int i = 0; i < textStructs.size(); i++) {
				TextOverlay::StaticAddText(textStructs[i]);
			}
		}
	}
	void OverlayBase::resizeWindow(std::pair<uint32_t, uint32_t> nextDims) {
		float nextWidth = static_cast<float>(nextDims.first);
		float nextHeight = static_cast<float>(nextDims.second);
		for (auto& textS : textStructs) {

			printf("resizing overlay, before - %.1f:%.1f : %.1f\n", nextWidth, screenWidth, textS.x);
			textS.x *= nextWidth / screenWidth;
			textS.y *= nextHeight / screenHeight;
			printf("resizing overlay, after - %.1f\n", textS.x);
		}
		screenWidth = nextWidth;
		screenHeight = nextHeight;
	}
}