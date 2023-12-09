#pragma once
#include "MenuModule.h"

namespace EWE {
	class GraphicsMM : public MenuModule {
	public:
		GraphicsMM(float screenWidth, float screenHeight);

		void processClick(double xpos, double ypos);

	};
}