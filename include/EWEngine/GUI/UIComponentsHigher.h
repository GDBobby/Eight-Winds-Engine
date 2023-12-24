#pragma once

#include "UIComponents.h"

namespace EWE {
	enum HigherLevelControl_ClickReturn {
		HLC_none,
		HLC_Clicked,
		HLC_WantCallback,
	};


	class VariableControl {
	public:
		enum NamingConvention {
			NC_numeric,
			NC_xyzw,
			NC_rgba,
		};

		TextStruct dataLabel;
		void* dataPtr;
		UIComp::VariableType dataType;
		uint8_t dataCount;
		void* steps; //small, medium, large
		NamingConvention nameConv = NC_numeric;
		float width = 100.f;

		std::pair<float, float> position;
		float screenWidth;
		float screenHeight;

		std::vector<std::pair<Button, Button>> buttons; //add subtract
		std::vector<TypeBox> typeBoxes;

		//VariableControl() {}

		//void setPosition(float x, float y, float screenWidth, float screenHeight);


		VariableControl(GLFWwindow* windowPtr, float posX, float posY, float width, float screenWidth, float screenHeight, std::string dataLabelString, void* dataPointer, UIComp::VariableType dataType, uint8_t dataCount, void* steps);
		~VariableControl()
		{
			free(steps);
		}

		bool Clicked(double xpos, double ypos);
		bool isSelected(int8_t checkSelection)
		{
			return checkSelection == selectedTypeBox;
		}

		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight);

		void moveBox(int freshX, int freshY);
		void giveGLFWCallbacks(GLFWmousebuttonfun mouseReturnFunction, GLFWkeyfun keyReturnFunction);
		void setLastPos(std::pair<int, int>& lastPos)
		{
			this->lastPos = lastPos;
		}


	private:
		static VariableControl* variableCtrlPtr;
		int8_t selectedTypeBox = -1;
		//bool mouseDragging = false; //variable drag not currently supported
		uint8_t maxStringLength = 18;
		int stringSelectionIndex = -1;
		bool readyForInput;

		GLFWmousebuttonfun mouseReturnPointer;
		GLFWkeyfun keyReturnPointer;
		GLFWwindow* windowPtr;

		std::pair<int, int> lastPos; //xy
		std::pair<int, int> totalMovement;

		static void MouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void typeCallback(GLFWwindow* window, unsigned int codepoint);


	};

	class ControlBox {
	public:

		float width = 100.f;

		TextStruct label;
		Transform2dComponent transform; //the background box
		glm::ivec4 dragBox; //height is about the size of the label string, width is the width of the full background box

		ControlBox(GLFWwindow* windowPtr, std::string labelString, float x, float y, float width, float screenWidth, float screenHeight);

		void emplaceVariableControl(std::string dataLabel, void* dataPointer, UIComp::VariableType dataType, uint8_t dataCount, void* steps);

		//~ControlBox() { }

		bool Clicked(double xpos, double ypos);

		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight);

		std::vector<VariableControl> variableControls;
		void giveGLFWCallbacks(GLFWmousebuttonfun mouseReturnFunction, GLFWkeyfun keyReturnFunction);
		GLFWmousebuttonfun mouseReturnPointer;
		GLFWkeyfun keyReturnPointer;

		void render(Simple2DPushConstantData& push);

		void render(NineUIPushConstantData& push);

	private:
		//UIComp::VariableType variableType;
		static ControlBox* ctrlBoxPtr; //should be renamed to ControlPointer
		//void* variables;
		//size_t variableCount;
		//void* steps; //small, medium, large
		bool mouseDragging = false;
		GLFWwindow* windowPtr;
		std::pair<int, int> lastPos;

		static void DragCallback(GLFWwindow* window, double xpos, double ypos);
		float screenWidth;
		float screenHeight;
		float ratioWidth = 1.f;
		float verticalSpacing = 26.f;
	};

	class MenuBar {
	public:
		MenuBar(float x, float y, float width, float height, float screenWidth, float screenHeight);
		std::pair<float, float> screenCoordinates;
		std::pair<float, float> screenDimensions;
		Transform2dComponent transform;

		void pushDropper(std::string dropperName, std::vector<std::string>& options, float screenWidth, float screenHeight);
		std::vector<DropBox> dropBoxes{};

		void init(float screenWidth, float screenHeight);

		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) {
			init(rszWidth, rszHeight);
		}

		int16_t Clicked(double xpos, double ypos);
		void render(NineUIPushConstantData& push, uint8_t drawID);
	};
}
