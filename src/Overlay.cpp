#include "EWEngine/GUI/Overlay.h"

namespace EWE {
	OverlayBase::OverlayBase(float screenWidth, float screenHeight) : screenWidth{ screenWidth }, screenHeight{screenHeight} {

	}
	void OverlayBase::DrawText() {
		if (isActive) {
			for (int i = 0; i < textStructs.size(); i++) {
				TextOverlay::StaticAddText(textStructs[i]);
			}
		}
	}
	void OverlayBase::ResizeWindow() {
		float nextWidth = VK::Object->screenWidth;
		float nextHeight = VK::Object->screenHeight;
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