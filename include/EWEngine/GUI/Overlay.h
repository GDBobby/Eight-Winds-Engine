#pragma once

#include "EWEngine/Graphics/TextOverlay.h"

#include <LAB/Transform.h>

namespace EWE {
	class OverlayBase {
	public:
		OverlayBase();

		virtual void DrawText();

		bool GetActive() {
			return isActive;
		}
		void SetActive(bool activity) {
			isActive = activity;
		}

		virtual void DrawObjects() = 0;
		void ResizeWindow(SettingsInfo::ScreenDimensions nextDimensions);
		
	protected:
		bool isActive = false;
		std::vector<TextStruct> textStructs{};
		std::vector<lab::Transform<float, 2>> gameObjects{};
	};
}