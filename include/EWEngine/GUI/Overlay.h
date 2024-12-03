#pragma once

#include "EWEngine/GameObject2D.h"
#include "EWEngine/Graphics/TextOverlay.h"

namespace EWE {
	class OverlayBase {
	public:
		OverlayBase(float screenWidth, float screenHeight);

		virtual void DrawText();

		bool GetActive() {
			return isActive;
		}
		void SetActive(bool activity) {
			isActive = activity;
		}

		virtual void DrawObjects() = 0;
		void ResizeWindow();
		
	protected:
		bool isActive = false;
		float screenWidth;
		float screenHeight;
		std::vector<TextStruct> textStructs{};
		std::vector<GameObject2D> gameObjects{};
	};
}