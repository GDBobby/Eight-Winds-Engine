#include "EWEngine/GUI/Overlay.h"

namespace EWE {
	OverlayBase::OverlayBase() {

	}
	void OverlayBase::DrawText() {
		if (isActive) {
			for (int i = 0; i < textStructs.size(); i++) {
				TextOverlay::StaticAddText(textStructs[i]);
			}
		}
	}
	void OverlayBase::ResizeWindow(glm::vec2 rescalingRatio) {
		float nextWidth = VK::Object->screenWidth;
		float nextHeight = VK::Object->screenHeight;
		for (auto& textS : textStructs) {

			textS.x *= rescalingRatio.x;
			textS.y *= rescalingRatio.y;
		}
	}
}