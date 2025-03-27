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
	void OverlayBase::ResizeWindow(SettingsInfo::ScreenDimensions nextDimensions) {
		glm::vec2 rescalingRatio{
			static_cast<float>(nextDimensions.width) / VK::Object->screenWidth,
			static_cast<float>(nextDimensions.height) / VK::Object->screenHeight
		};

		for (auto& textS : textStructs) {

			textS.x *= rescalingRatio.x;
			textS.y *= rescalingRatio.y;
		}
	}
}