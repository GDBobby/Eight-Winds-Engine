#pragma once

#include "../GameObject2D.h"
#include "../graphics/TextOverlay.h"


namespace EWE {
	namespace UIComp {
		enum InputType {
			InputType_none,
			InputType_alpha,
			InputType_alphaLower,
			InputType_alphanumeric,
			InputType_numeric,
			InputType_float,
		};
		enum VariableType {
			VT_int64,
			VT_int16,
			VT_int32,
			VT_int8,
			VT_float,
			VT_double,

			VT_error,
		};

		//type abstraction
		size_t getVariableSize(VariableType vType);
		std::string getVariableString(void* data, int offset, VariableType vType);
		void SetVariableFromString(void* data, int offset, std::string& inString, VariableType vType);
		void addVariables(void* firstData, int32_t firstOffset, void* secondData, int32_t secondOffset, VariableType vType);
		void subtractVariables(void* firstData, int32_t firstOffset, void* secondData, int32_t secondOffset, VariableType vType);

		void TypeToString(std::string& outputString, uint16_t maxStringLength, int codePoint, InputType inputType, uint8_t stringSelectionIndex);


		//2d to screen conversions
		void convertTransformToClickBox(Transform2dComponent& transform, glm::ivec4& clickBox, float screenWidth, float screenHeight);

		bool checkClickBox(glm::ivec4& clickBox, double mouseX, double mouseY);

		void TextToTransform(Transform2dComponent& transform, TextStruct& textStruct, glm::ivec4& clickBox, float screenWidth, float screenHeight);

		void convertScreenTo2D(glm::ivec2 screen, glm::vec2& coord2D, float screenWidth, float screenHeight);
		void printClickBox(glm::ivec4& clickBox);

		void convertClickToTransform(glm::ivec4& clickBox, Transform2dComponent& transform, float screenWidth, float screenHeight);

	}

	enum UIComponentTypes {
		UIT_Label,
		UIT_Slider,
		UIT_Combobox,
		UIT_Dropbox,
		UIT_Checkbox,
		UIT_ClickTextBox,
		UIT_TypeBox,
		UIT_VariableTypeBox,
		UIT_MenuBar,
		UIT_ImageButton, //these should be anywhere from 64x64 to 16x16

		UIT_none,
	};

	struct ClickTextBox { //i.e. menu options
		TextStruct textStruct{};
		glm::ivec4 clickBox{ 0 };
		Transform2dComponent transform{};
		bool isActive = false;

		ClickTextBox(std::string string, float x, float y, unsigned char align, float scale, float screenW, float screenH);
		ClickTextBox(){}
		ClickTextBox(TextStruct textStruct, float screenW, float screenH);
		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight);
		bool Clicked(double xpos, double ypos);

	};
	struct TypeBox { //keybinds
		TextStruct textStruct;
		glm::ivec4 clickBox{ 0 };
		Transform2dComponent transform;
		bool isActive = false;
		bool mouseDragging = false;
		bool readyForInput = false;
		uint8_t maxStringLength = 69;

		UIComp::InputType inputType = UIComp::InputType_alpha;

		static TypeBox* textBoxPointer;

		GLFWmousebuttonfun mouseReturnPointer;
		GLFWkeyfun keyReturnPointer;

		static void MouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void typeCallback(GLFWwindow* window, unsigned int codepoint);

		TypeBox(TextStruct textStruct, float screenW, float screenHeight);
		TypeBox(std::string string, float x, float y, uint8_t alignment, float scale, float screenWidth, float screenHeight);

		void giveGLFWCallbacks(GLFWwindow* windowPtr, GLFWmousebuttonfun mouseReturnFunction, GLFWkeyfun keyReturnFunction);

		bool Clicked(double xpos, double ypos) { return UIComp::checkClickBox(clickBox, xpos, ypos); }

		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight);
	};

	struct Slider {
		std::pair<Transform2dComponent, Transform2dComponent> bracketButtons;
		Transform2dComponent bracket{};
		Transform2dComponent slider{};
		float slidePosition{0.5f}; //can be translated to volume
		glm::ivec4 click[3] = { glm::ivec4{0},glm::ivec4{0},glm::ivec4{0} }; //left, slide, right
		float spaceBetween{};
		bool VolumeTrueSensFalse = true;
		uint8_t mySens = 100; //sens short for sensitivity
		bool isActive = false;
		float screenHeight{};
		float screenWidth{};
		Slider() {}
		void setTransform(glm::vec2 newTrans);
		void Init(glm::vec2 initTrans, float screenHeight, float screenWidth, uint8_t currentSens);

		void Init(glm::vec2 initTrans, float screenHeight, float screenWidth, float currentVolume);
		void setSliderPosition(float sliderPos);

		void MoveSlider(int movedAmount);
		void resizeWindow(float newWidth, float newHeight);
		void giveSens(uint8_t currentSens);
		int8_t Clicked(double xpos, double ypos);
		void buttonClicked(bool leftFalseRightTrue);
	};

	struct ComboBox { //currently scattered throughout uihandler.h as a combomenustruct, vector of textstructs
		bool isActive = false;
		bool currentlyDropped = false;

		unsigned char align = TA_left;
		float scale = 1.f;

		ClickTextBox activeOption;
		std::vector<ClickTextBox> comboOptions;
		int8_t currentlySelected = 0;

		ComboBox(TextStruct textStruct, float screenWidth, float screenHeight);

		void pushOption(std::string string, float screenWidth, float screenHeight);

		void setSelection(int8_t selection);

		bool Clicked(double xpos, double ypos);
		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight);
	};

	struct DropBox {
		DropBox() {}
		bool isActive = false;
		bool currentlyDropped = false;
		bool drawDropperBox = false;

		unsigned char align = TA_left;
		float scale = 1.f;
		ClickTextBox dropper; //file, new, save, load, return to main, exit

		std::vector<TextStruct> dropOptions;
		std::vector<glm::ivec4> clickBoxes;

		Transform2dComponent dropBackground;

		void pushOption(std::string pushString);
		void pushOption(std::string pushString, float screenWidth, float screenHeight);

		void init(float screenWidth, float screenHeight);

		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) 
			{ init(rszWidth, rszHeight); }

		int8_t Clicked(double xpos, double ypos);
	};

	struct Button {
		Transform2dComponent transform;
		glm::ivec4 clickBox;

		Button(glm::vec2 translation, float screenWidth, float screenHeight);

		void resizeWindow(float rszWidth, float rszHeight) 
			{ UIComp::convertTransformToClickBox(transform, clickBox, rszWidth, rszHeight); }

		bool Clicked(double xpos, double ypos) 
			{ return UIComp::checkClickBox(clickBox, xpos, ypos); }
	};

	struct Checkbox {

		enum DefaultOffsets {
			DO_left,
			DO_above,
			DO_right,
			DO_below,
		};

		Button button;
		TextStruct label;
		glm::vec2 labelOffset;
		bool isChecked = false;
		bool isActive = false;

		Checkbox(std::string labelString, glm::vec2 translation, DefaultOffsets labelOffset, float screenW, float screenH);

		Checkbox(std::string labelString, glm::vec2 translation, glm::vec2 labelOffset, TextAlign alignment, float screenW, float screenH);

		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight);

		bool Clicked(double xpos, double ypos);
	};


// ~~~~~~~~~~ HIGHER LEVEL CONTROLS ~~~~~~~~~~~~~~
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
			{ free(steps); }

		bool Clicked(double xpos, double ypos);
		bool isSelected(int8_t checkSelection)
			{return checkSelection == selectedTypeBox;}
		
		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight);

		void moveBox(int freshX, int freshY);
		void giveGLFWCallbacks(GLFWmousebuttonfun mouseReturnFunction, GLFWkeyfun keyReturnFunction);
		void setLastPos(std::pair<int, int>& lastPos)
			{ this->lastPos = lastPos; }


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
		std::vector<DropBox> dropBoxes;

		void init(float screenWidth, float screenHeight) {
			for (int i = 0; i < dropBoxes.size(); i++) {
				if (i >= 1) {
					dropBoxes[i].dropper.textStruct.x = dropBoxes[i - 1].dropper.clickBox.z + (10.f * screenWidth / DEFAULT_WIDTH);
				}
				dropBoxes[i].init(screenWidth, screenHeight);
			}
		}

		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) 
			{ init(rszWidth, rszHeight); }

		int16_t Clicked(double xpos, double ypos);
	};
}
