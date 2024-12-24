#pragma once

#include "EWEngine/GameObject2D.h"
#include "EWEngine/Graphics/TextOverlay.h"
#include "UICompFunctions.h"
#include "EWEngine/Systems/Rendering/Pipelines/Dimension2.h"


namespace EWE {
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

		ClickTextBox(std::string string, float x, float y, unsigned char align, float scale);
		ClickTextBox(){}
		ClickTextBox(TextStruct textStruct);
		void ResizeWindow(glm::vec2 rescalingRatio);
		bool Clicked(double xpos, double ypos);

		void Render(Array2DPushConstantData& push);

	};
	struct TypeBox { //keybinds
		TextStruct textStruct;
		glm::ivec4 clickBox{ 0 };
		Transform2dComponent transform;
		bool isActive = false;
		bool mouseDragging = false;
		bool readyForInput = false;
		uint8_t maxStringLength = 69;
		int stringSelectionIndex = -1;

		UIComp::InputType inputType = UIComp::InputType_alpha;

		static TypeBox* textBoxPointer;

		GLFWmousebuttonfun mouseReturnPointer;
		GLFWkeyfun keyReturnPointer;

		static void MouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void TypeCallback(GLFWwindow* window, uint32_t codepoint);

		TypeBox(TextStruct textStruct);
		TypeBox(std::string string, float x, float y, uint8_t alignment, float scale);

		void GiveGLFWCallbacks(GLFWwindow* windowPtr, GLFWmousebuttonfun mouseReturnFunction, GLFWkeyfun keyReturnFunction);

		bool Clicked(double xpos, double ypos) { return UIComp::CheckClickBox(clickBox, xpos, ypos); }

		void ResizeWindow(glm::vec2 rescalingRatio);

		void Render(Array2DPushConstantData& push);
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
		Slider() {}
		void SetTransform(glm::vec2 newTrans);
		void Init(glm::vec2 initTrans, uint8_t currentSens);

		void Init(glm::vec2 initTrans, float currentVolume);
		void SetSliderPosition(float sliderPos);

		void MoveSlider(int movedAmount);
		void ResizeWindow();
		void GiveSens(uint8_t currentSens);
		int8_t Clicked(double xpos, double ypos);
		void ButtonClicked(bool leftFalseRightTrue);

		void Render(Array2DPushConstantData& push, uint8_t drawID);
	};

	struct ComboBox {
		bool isActive = false;
		bool currentlyDropped = false;

		unsigned char align = TA_left;
		float scale = 1.f;

		ClickTextBox activeOption;
		std::vector<ClickTextBox> comboOptions;
		int8_t currentlySelected = 0;

		ComboBox(TextStruct textStruct);

		void PushOption(std::string string);

		void SetSelection(int8_t selection);

		bool Clicked(double xpos, double ypos);
		void ResizeWindow(glm::vec2 rescalingRatio);

		void Render(Array2DPushConstantData& push);
		void Move(float xDiff, float yDiff);
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

		void PushOption(std::string pushString);

		void Init();

		void ResizeWindow(glm::vec2 rescalingRatio) {
			Init(); 
		}

		int8_t Clicked(double xpos, double ypos);

		void Render(Array2DPushConstantData& push);
	};
	/*
	struct SideList {
		SideList() {}
		bool isActive = false;
		bool currentlyExpanded = false;
		bool draw = false;

		unsigned char align = TA_left;
		float scale = 1.f;
		ClickTextBox dropper; //file, new, save, load, return to main, exit

		std::vector<TextStruct> listOptions;
		std::vector<glm::ivec4> clickBoxes;

		Transform2dComponent dropBackground;

		void pushOption(std::string pushString);
		void pushOption(std::string pushString);

		void init(float screenWidth, float screenHeight);

		void resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) {
			init(rszWidth, rszHeight);
		}

		int8_t Clicked(double xpos, double ypos);
	};
	*/

	struct Button {
		Transform2dComponent transform;
		glm::ivec4 clickBox;

		Button(glm::vec2 translation);

		void ResizeWindow() { UIComp::ConvertTransformToClickBox(transform, clickBox, VK::Object->screenWidth, VK::Object->screenHeight); }

		bool Clicked(double xpos, double ypos) 
			{ return UIComp::CheckClickBox(clickBox, xpos, ypos); }

		void Render(Array2DPushConstantData& push);
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

		Checkbox(std::string labelString, glm::vec2 translation, DefaultOffsets labelOffset);

		Checkbox(std::string labelString, glm::vec2 translation, glm::vec2 labelOffset, TextAlign alignment);

		void ResizeWindow(glm::vec2 rescalingRatio);

		bool Clicked(double xpos, double ypos);

		void Render(Array2DPushConstantData& push);
	};


// ~~~~~~~~~~ HIGHER LEVEL CONTROLS ~~~~~~~~~~~~~~

}
