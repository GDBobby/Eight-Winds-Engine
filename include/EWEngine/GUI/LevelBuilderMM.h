#pragma once
#include "MenuModule.h"

namespace EWE {
	class LevelBuilderMM : public MenuModule {
	public:
		LevelBuilderMM(float screenWidth, float screenHeight, GLFWwindow* windowPtr);

		void processClick(double xpos, double ypos);
	};
}