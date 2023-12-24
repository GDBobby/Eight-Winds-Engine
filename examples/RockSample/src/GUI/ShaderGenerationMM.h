#pragma once
#include <EWEngine/GUI/MenuModule.h>
#include "MenuEnums.h"

namespace EWE {
	class ShaderGenerationMM : public MenuModule {
	public:
		ShaderGenerationMM(float screenWidth, float screenHeight);

		void processClick(double xpos, double ypos);
	};
}