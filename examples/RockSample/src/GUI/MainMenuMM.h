#pragma once
#include <EWEngine/GUI/MenuModule.h>

namespace EWE {
	class MainMenuMM : public MenuModule {
	public:
		MainMenuMM(float screenWidth, float screenHeight);

		void processClick(double xpos, double ypos);
	};
}