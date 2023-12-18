#include "EWEngine/gui/LevelBuilderMM.h"

namespace EWE {
	LevelBuilderMM::LevelBuilderMM(float screenWidth, float screenHeight, GLFWwindow* windowPtr) {
		float widthRescaling = screenWidth / DEFAULT_WIDTH;
		float heightRescaling = screenHeight / DEFAULT_HEIGHT;


		MenuBar& backRef = menuBars.emplace_back(0.f, 0.f, screenWidth, 30.f * heightRescaling, screenWidth, screenHeight);

		std::vector<std::string> options = {
			"New",
			"Save",
			"Save As",
			"Load",
			"Return To Menu",
			"Exit"
		};
		backRef.pushDropper("File", options, screenWidth, screenHeight);

		options.clear();
		options = {
			"Undo",
			"Redo",
			"Copy",
			"Paste",
			"Settings"
		};
		backRef.pushDropper("Edit", options, screenWidth, screenHeight);

		options.clear();
		options = {
			"Add Quad",
			"Object Control",
			"Light Settings",
			"Reset Camera Pos"
		};
		backRef.pushDropper("View", options, screenWidth, screenHeight);
		backRef.init(screenWidth, screenHeight);

		printf("bug before control box construction? \n");
		float steps[] = { 0.01f, .1f, 1.f };
		//TypeVariableBox(void* data, size_t variableCount, VariableType vType, float x, float y, float width, float screenWidth, float screenHeight, void* stepData, NamingConvention nameConv = NC_numeric)

		//i need to give this the variable after construction, then call an init function within the VariableTypeBoxes class
		controlBoxes.emplace_back(windowPtr, "Player", 600.f, 300.f, 100.f, screenWidth, screenHeight);

		std::string stringBuffer = "Translation";

		//probably need to call these in the main thread, eightwinds.cpp, before starting the thread loop
		//controlBoxes[0].emplaceVariableControl(stringBuffer, PlayerObject::poVector[0].playerInput.liveState->translation, UIComp::VT_float, 3, (void*)steps);
		stringBuffer = "Rotation";
		//controlBoxes[0].emplaceVariableControl(stringBuffer, &PlayerObject::poVector[0].playerInput.liveState->cameraRotation[1], UIComp::VT_float, 1, (void*)steps);
		//controlBoxes[0].giveGLFWCallbacks(uiHandler.staticMouseCallback, uiHandler.staticKeyCallback);


		printf("bug after control box destruciton \n");
	}

	void LevelBuilderMM::processClick(double xpos, double ypos) {
		std::pair<UIComponentTypes, int8_t> returnValues = MenuModule::checkClick(xpos, ypos);
		

		//return MCR_none;
	}


}