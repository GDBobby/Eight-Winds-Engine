#pragma once
#include <EWEngine/GUI/MenuModule.h>
#include "EWEngine/ShaderGraph/Graph.h"
#include "MenuEnums.h"

namespace EWE {
	class ShaderGenerationMM : public MenuModule {
	public:
		ShaderGenerationMM(GLFWwindow* windowPtr, float screenWidth, float screenHeight);

		void processClick(double xpos, double ypos) override;
		void drawText(TextOverlay* textOverlay) override;

		void drawNewObjects() override;
		void drawNewNine() override;
	private:
		Shader::Graph shaderGraph;
	};
}