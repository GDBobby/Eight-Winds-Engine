#pragma once
#include <EWEngine/GUI/MenuModule.h>
#include "EWEngine/ShaderGraph/InputBox.h"
#include "MenuEnums.h"

namespace EWE {
	class ShaderGenerationMM : public MenuModule {
	public:
		ShaderGenerationMM(GLFWwindow* windowPtr, float screenWidth, float screenHeight);

		void processClick(double xpos, double ypos);
		void drawText(TextOverlay* textOverlay);
		void drawNewNine() override;
	private:
		Shader::InputBox inputBox;
	};
}