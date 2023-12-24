#include "ShaderGenerationMM.h"

namespace EWE {
	ShaderGenerationMM::ShaderGenerationMM(GLFWwindow* windowPtr, float screenWidth, float screenHeight) :
		inputBox{ windowPtr, true, 100.f, 100.f, screenWidth, screenHeight }
	{
		float widthRescaling = screenWidth / DEFAULT_WIDTH;
		float heightRescaling = screenHeight / DEFAULT_HEIGHT;

		auto& menuBar = menuBars.emplace_back(0, 0, screenWidth, 40.f * heightRescaling, screenWidth, screenHeight);
		std::vector<std::string> options = { "New", "Save", "Save As", "Load", "Export", "Compile", "Exit" };
		menuBar.pushDropper("File", options, screenWidth, screenHeight);
		menuBar.init(screenWidth, screenHeight);
		//menuBar.dropBoxes.back().init(screenWidth, screenHeight);
		//clickText.emplace_back("Exit", 0.f, 860.f * heightRescaling, TA_left, 3.f, screenWidth, screenHeight);

		//benchmarking
		

	}

	void ShaderGenerationMM::processClick(double xpos, double ypos) {
		std::pair<UIComponentTypes, int16_t> returnValues = MenuModule::checkClick(xpos, ypos);
		printf("check click return in shader gen - %d:%d \n", returnValues.first, returnValues.second);

		if (returnValues.first == UIT_VariableTypeBox) {
			printf("UIT_VTB - returnValues.second : %d \n", returnValues.second);
			//just give the returns off the jump, and let it handle it all the way
			//menuMap[menu_main].controlBoxes[returnValues.second].giveGLFWCallbacks(staticMouseCallback, staticKeyCallback);
		}
		else if (returnValues.first = UIT_MenuBar) {
			switch (returnValues.second) {
			case 6: {
				printf("return to main menu pls \n");
				clickReturns.push(MCR_swapToMainMenu);
				break;
			}
			}
		}

		//return MCR_none;

	}
	void ShaderGenerationMM::drawText(TextOverlay* textOverlay) {
		MenuModule::drawText(textOverlay);
		textOverlay->addText(inputBox.name);
	}
	void ShaderGenerationMM::drawNewNine() {
		MenuModule::drawNewNine();
		NineUIPushConstantData push{};
		inputBox.render(push);
	}


}