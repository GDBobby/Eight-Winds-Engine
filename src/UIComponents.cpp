#include "EWEngine/gui/UIComponents.h"
namespace EWE {



	
	//end uicomp

	//  ~~~~~~~~~~~~~~~~~~~ CLICKBOX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ClickTextBox::ClickTextBox(std::string string, float x, float y, unsigned char align, float scale, float screenW, float screenH) : textStruct{ string, x, y, align, scale } {
		UIComp::TextToTransform(transform, textStruct, clickBox, screenW, screenH);
	}
	ClickTextBox::ClickTextBox(TextStruct textStruct, float screenW, float screenH) : textStruct{ textStruct } {
		UIComp::TextToTransform(transform, textStruct, clickBox, screenW, screenH);
	}
	void ClickTextBox::resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) {
		printf("click text resize \n");
		textStruct.x *= rszWidth / oldWidth;
		textStruct.y *= rszHeight / oldHeight;
		UIComp::TextToTransform(transform, textStruct, clickBox, rszWidth, rszHeight);
	}
	bool ClickTextBox::Clicked(double xpos, double ypos) {
		return UIComp::checkClickBox(clickBox, xpos, ypos);
	}
	
	// ~~~~~~~~~~~~~~~~~~~~~ TYPE BOX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	TypeBox* TypeBox::textBoxPointer;

	void TypeBox::MouseCallback(GLFWwindow* window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS) {
				double xpos = 0;
				double ypos = 0;
				glfwGetCursorPos(window, &xpos, &ypos);
				if (UIComp::checkClickBox(textBoxPointer->clickBox, xpos, ypos)) {
					printf("ready for input \n");
					textBoxPointer->readyForInput = true;
					textBoxPointer->mouseDragging = true;
				}
				else {
					textBoxPointer->readyForInput = false;
					printf("returning callbacks \n");
					glfwSetMouseButtonCallback(window, textBoxPointer->mouseReturnPointer);
					glfwSetKeyCallback(window, textBoxPointer->keyReturnPointer);
				}
			}
			else {
				textBoxPointer->mouseDragging = false;
			}
		}
	}
	void TypeBox::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		printf("key callback \n");
		if (textBoxPointer->readyForInput && (action != GLFW_RELEASE)) {
			if (key == GLFW_KEY_BACKSPACE && textBoxPointer->textStruct.string.length() > 0) {
				printf("bqackspace \n");
				textBoxPointer->textStruct.string = textBoxPointer->textStruct.string.substr(0, textBoxPointer->textStruct.string.length() - 1);
			}
			else if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER) {
				printf("escape \n");
				textBoxPointer->readyForInput = false;
				glfwSetMouseButtonCallback(window, textBoxPointer->mouseReturnPointer);
				glfwSetKeyCallback(window, textBoxPointer->keyReturnPointer);
			}

			printf("pressed key - %d:%d \n", key, scancode);
			//textBoxPointer->textStruct.string += glfwGetKeyName(key, scancode);
		}
	}
	void TypeBox::typeCallback(GLFWwindow* window, unsigned int codepoint) {
		printf("key typed? - %ud \n", codepoint);

		UIComp::TypeToString(textBoxPointer->textStruct.string, textBoxPointer->maxStringLength, codepoint, textBoxPointer->inputType, static_cast<uint8_t>(textBoxPointer->textStruct.string.length()));
	}
	TypeBox::TypeBox(TextStruct textStruct, float screenW, float screenHeight) : textStruct{ textStruct } {
		UIComp::TextToTransform(transform, textStruct, clickBox, screenW, screenHeight);
	}
	TypeBox::TypeBox(std::string string, float x, float y, uint8_t alignment, float scale, float screenWidth, float screenHeight) : textStruct{ string, x, y, alignment, scale } {
		UIComp::TextToTransform(transform, textStruct, clickBox, screenWidth, screenHeight);
	}
	void TypeBox::giveGLFWCallbacks(GLFWwindow* windowPtr, GLFWmousebuttonfun mouseReturnFunction, GLFWkeyfun keyReturnFunction) {
		readyForInput = true;
		textBoxPointer = this;
		glfwSetCharCallback(windowPtr, typeCallback);
		glfwSetKeyCallback(windowPtr, KeyCallback);
		glfwSetMouseButtonCallback(windowPtr, MouseCallback);
		mouseReturnPointer = mouseReturnFunction;
		keyReturnPointer = keyReturnFunction;
	}

	void TypeBox::resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) {
		printf("click text resize \n");
		textStruct.x *= rszWidth / oldWidth;
		textStruct.y *= rszHeight / oldHeight;
		UIComp::TextToTransform(transform, textStruct, clickBox, rszWidth, rszHeight);
	}
	
	
	// ~~~~~~~~~~~~~~~~~~~~~ SLIDER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void Slider::setTransform(glm::vec2 newTrans) {
		bracket.translation = newTrans;
		slider.translation = newTrans;
		bracketButtons.first.translation = newTrans;
		bracketButtons.second.translation = newTrans;
		bracketButtons.first.translation.x -= 0.1f;
		bracketButtons.second.translation.x += 0.1f;
	}
	void Slider::Init(glm::vec2 initTrans, float screenHeight, float screenWidth, uint8_t currentSens) {
		VolumeTrueSensFalse = false;
		mySens = currentSens;
		Init(initTrans, screenHeight, screenWidth, static_cast<float>(currentSens) / 100.f);
	}
	void Slider::Init(glm::vec2 initTrans, float screenHeight, float screenWidth, float currentVolume) {
		this->screenWidth = screenWidth;
		this->screenHeight = screenHeight;
		bracket.translation = initTrans;

		bracketButtons.first.scale = { 0.1f, 0.2f };
		bracketButtons.second.scale = { 0.1f, 0.2f };
		bracket.scale = { .5f, .125f };
		slider.scale = { 0.05f, 0.1f };

		bracketButtons.first.translation = initTrans;
		bracketButtons.first.translation.x -= (bracket.scale.x / 2.f) + (bracketButtons.first.scale.x / 2.f);
		bracketButtons.second.translation = initTrans;
		bracketButtons.second.translation.x += (bracket.scale.x / 2.f) + (bracketButtons.second.scale.x / 2.f); //first and second should have idenctical scale

		UIComp::convertTransformToClickBox(bracketButtons.first, click[0], screenWidth, screenHeight);
		UIComp::convertTransformToClickBox(bracketButtons.second, click[2], screenWidth, screenHeight);

		//now on to the slider
		slidePosition = currentVolume;
		//get space between transforms, thats movement per pixel / 99
		spaceBetween = (bracketButtons.second.translation.x - (bracketButtons.second.scale.x / 2)) - (bracketButtons.first.translation.x + (bracketButtons.first.scale.x / 2)) - slider.scale.x;
		//printf("space between, which volume ~ %.2f : %.2f \n", spaceBetween, currentVolume);
		slider.translation.x = bracket.translation.x + (spaceBetween * (slidePosition - 0.5f));
		slider.translation.y = initTrans.y;
		UIComp::convertTransformToClickBox(slider, click[1], screenWidth, screenHeight);

		//setActivity(false);
	}
	void Slider::setSliderPosition(float sliderPos) {
		slidePosition = sliderPos;
		spaceBetween = (bracketButtons.second.translation.x - (bracketButtons.second.scale.x / 2)) - (bracketButtons.first.translation.x + (bracketButtons.first.scale.x / 2)) - slider.scale.x;
		//printf("space between, which volume ~ %.2f : %.2f \n", spaceBetween, currentVolume);
		slider.translation.x = bracket.translation.x + (spaceBetween * (slidePosition - 0.5f));
		UIComp::convertTransformToClickBox(slider, click[1], screenWidth, screenHeight);
	}
	void Slider::MoveSlider(int movedAmount) {
		//printf("moved amount %d \n", movedAmount);
		if (VolumeTrueSensFalse) {
			//printf("slide move? %.5f \n", static_cast<float>(movedAmount) / 100.f);
			slidePosition += static_cast<float>(movedAmount) / 100.f;
			if (slidePosition < 0.0f) {
				//printf("yo slide position to 0? \n");
				slidePosition = 0.0f;
			}
			else if (slidePosition > 1.f) {
				//printf("yo slide position to 1? \n");
				slidePosition = 1.f;
			}
			slider.translation.x = bracket.translation.x + (spaceBetween * (slidePosition - 0.5f));
			UIComp::convertTransformToClickBox(slider, click[1], screenWidth, screenHeight);
		}
		else {
			//printf("sens move? \n");
			if (movedAmount < 0 && (movedAmount * -1 > mySens)) {
				mySens = 0;
			}
			else if (movedAmount + mySens > 100) {
				mySens = 100;
			}
			else {
				mySens += movedAmount;
			}
			slidePosition = static_cast<float>(mySens) / 100.f;
			slider.translation.x = bracket.translation.x + (spaceBetween * (slidePosition - 0.5f));
			UIComp::convertTransformToClickBox(slider, click[1], screenWidth, screenHeight);
		}
		//printf("slide position : %.2f \n", slidePosition);
	}

	void Slider::resizeWindow(float newWidth, float newHeight) {
		//printf("slider resize window \n");
		screenWidth = newWidth;
		screenHeight = newHeight;
		UIComp::convertTransformToClickBox(bracketButtons.first, click[0], screenWidth, screenHeight);
		UIComp::convertTransformToClickBox(slider, click[1], screenWidth, screenHeight);
		UIComp::convertTransformToClickBox(bracketButtons.second, click[2], screenWidth, screenHeight);
	}
	void Slider::giveSens(uint8_t currentSens) {
		VolumeTrueSensFalse = false;
		mySens = currentSens;
		slidePosition = mySens / 100.f;
		slider.translation.x = bracket.translation.x + (spaceBetween * (slidePosition - 0.5f));
		UIComp::convertTransformToClickBox(slider, click[1], screenWidth, screenHeight);
		//MoveSlider();
	}
	int8_t Slider::Clicked(double xpos, double ypos) {
		//printf("active click check ~ %.3f : %.3f \n", xpos, ypos);
		//printf("check the click box, xzyw ~ %d:%d:%d:%d \n", click[0].x, click[0].y, click[0].z, click[0].w);
		if (UIComp::checkClickBox(click[0], xpos, ypos)) {
			buttonClicked(false);
			return 0;
		}
		else if (UIComp::checkClickBox(click[1], xpos, ypos)) {
			return 1;
		}
		else if (UIComp::checkClickBox(click[2], xpos, ypos)) {
			buttonClicked(true);
			return 2;
		}

		return -1;
	}
	void Slider::buttonClicked(bool leftFalseRightTrue) {
		if (leftFalseRightTrue) {
			//right button clicked
			slidePosition += 0.01f;
			if (slidePosition > 1.f) {
				//printf("yo slide position to 1? \n");
				slidePosition = 1.f;
			}
		}
		else {
			slidePosition -= .01f;
			if (slidePosition < 0.0f) {
				//printf("yo slide position to 0? \n");
				slidePosition = 0.0f;
			}
		}
		slider.translation.x = bracket.translation.x + (spaceBetween * (slidePosition - 0.5f));
		UIComp::convertTransformToClickBox(slider, click[1], screenWidth, screenHeight);
	}

	// ~~~~~~~~~~~~~~~~~~~~~ COMBOBOX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	ComboBox::ComboBox(TextStruct textStruct, float screenWidth, float screenHeight) {
		//printf("COMBOBOX CONSTRUCTION GIVEN ? %s  %.2f:%.2f\n", textStruct.string.c_str(), textStruct.x, textStruct.y);
		activeOption.textStruct = textStruct;
		UIComp::TextToTransform(activeOption.transform, activeOption.textStruct, activeOption.clickBox, screenWidth, screenHeight);
		//printf("COMBOBOX CONSTRUCTION CONSTRUCTED : %s  %.2f:%.2f\n", activeOption.textStruct.string.c_str(), activeOption.textStruct.x, activeOption.textStruct.y);
		scale = activeOption.textStruct.scale;
	}

	void ComboBox::pushOption(std::string string, float screenWidth, float screenHeight) {
		//printf("pushing option, comboX:comboY:scale - %.3f:%.3f:%.1f \n", activeOption.textStruct.x, activeOption.textStruct.y, scale);
		comboOptions.emplace_back(TextStruct{ string, activeOption.textStruct.x, activeOption.textStruct.y + (comboOptions.size() + 1) * (19.f * scale) * screenHeight / DEFAULT_HEIGHT, align, scale }, screenWidth, screenHeight);
		//comboOptions.back().clickBox = activeOption.clickBox;
		if (comboOptions.back().clickBox.z - comboOptions.back().clickBox.x > activeOption.clickBox.z - activeOption.clickBox.x) {
			activeOption.clickBox.z = activeOption.clickBox.x + comboOptions.back().clickBox.z - comboOptions.back().clickBox.x;
			for (int i = 0; i < comboOptions.size() - 1; i++) {
				comboOptions[i].clickBox.z = activeOption.clickBox.z;
			}
		}

		comboOptions.back().clickBox.x = activeOption.clickBox.x;
		comboOptions.back().clickBox.z = activeOption.clickBox.z;
		UIComp::convertScreenTo2D(glm::ivec2{ (comboOptions.back().clickBox.x + comboOptions.back().clickBox.z) / 2 ,
			(comboOptions.back().clickBox.y + comboOptions.back().clickBox.w) / 2 }, comboOptions.back().transform.translation, screenWidth, screenHeight);
		comboOptions.back().transform.scale.x = activeOption.transform.scale.x;
	}

	void ComboBox::setSelection(int8_t selection) {
		currentlySelected = selection;
		activeOption.textStruct.string = comboOptions[currentlySelected].textStruct.string;
	}

	bool ComboBox::Clicked(double xpos, double ypos) {
		if (!currentlyDropped) {
			if (UIComp::checkClickBox(activeOption.clickBox, xpos, ypos)) {
				UIComp::printClickBox(activeOption.clickBox);
				currentlyDropped = true;
				return true;
			}
		}
		else {
			for (int i = 0; i < comboOptions.size(); i++) {
				if (UIComp::checkClickBox(comboOptions[i].clickBox, xpos, ypos)) {
					UIComp::printClickBox(comboOptions[i].clickBox);
					currentlyDropped = false;
					activeOption = comboOptions[i];
					currentlySelected = i;
					return true;
				}
			}
			currentlyDropped = false;
		}
		return false;
	}
	void ComboBox::resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) {

		activeOption.resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		for (int i = 0; i < comboOptions.size(); i++) {
			comboOptions[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
			comboOptions[i].textStruct.x = activeOption.textStruct.x;
			comboOptions[i].textStruct.y = activeOption.textStruct.y + (i + 1) * (19.f * scale) * rszHeight / DEFAULT_HEIGHT;

			if ((comboOptions[i].clickBox.z - comboOptions[i].clickBox.x) > (activeOption.clickBox.z - activeOption.clickBox.x)) {
				activeOption.clickBox.z = activeOption.clickBox.x + comboOptions[i].clickBox.z - comboOptions[i].clickBox.x;
				for (int j = 0; j < i; j++) {
					comboOptions[j].clickBox.z = activeOption.clickBox.z;
					UIComp::convertScreenTo2D(glm::ivec2{ (comboOptions[j].clickBox.x + comboOptions[j].clickBox.z) / 2 ,
						(comboOptions[j].clickBox.y + comboOptions[j].clickBox.w) / 2 }, comboOptions[j].transform.translation, rszWidth, rszHeight);
				}
			}
			else {
				comboOptions[i].clickBox.z = activeOption.clickBox.z;
			}
		}
	}

	// ~~~~~~~~~~~~~~~~~~~~~~ DROPBOX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void DropBox::pushOption(std::string pushString) {
		dropOptions.push_back(dropper.textStruct);
		dropOptions.back().string = pushString;
	}
	void DropBox::pushOption(std::string pushString, float screenWidth, float screenHeight) {
		dropOptions.push_back(dropper.textStruct);
		dropOptions.back().string = pushString;
		init(screenWidth, screenHeight);
	}
	void DropBox::init(float screenWidth, float screenHeight) {
		//if dropOptions.size == 0, this will crash

		if (dropOptions.size() != clickBoxes.size() && (dropOptions.size() != 0)) {
			clickBoxes.resize(dropOptions.size());
		}
		UIComp::TextToTransform(dropper.transform, dropper.textStruct, dropper.clickBox, screenWidth, screenHeight);

		float biggestWidth = 0.f;
		for (int i = 0; i < dropOptions.size(); i++) {
			dropOptions[i].x = dropper.textStruct.x;
			dropOptions[i].y = dropper.textStruct.y + 13.f + (i + 1) * (26.f * scale) * screenHeight / DEFAULT_HEIGHT;

			float tempWidth = dropOptions[i].getWidth(screenWidth);
			if (tempWidth > biggestWidth) { biggestWidth = tempWidth; }

			clickBoxes[i].y = static_cast<int>(dropOptions[i].y);
			clickBoxes[i].w = static_cast<int>(dropOptions[i].y + (26.f * scale) * screenHeight / DEFAULT_HEIGHT);
		}
		for (int i = 0; i < dropOptions.size(); i++) {
			if (align == TA_center) {
				clickBoxes[i].x = static_cast<int>(dropper.textStruct.x - biggestWidth * screenWidth / 4.f);
				clickBoxes[i].z = static_cast<int>(dropper.textStruct.x + biggestWidth * screenWidth / 4.f);
			}
			else if (align == TA_left) {
				clickBoxes[i].x = static_cast<int>(dropper.textStruct.x);
				clickBoxes[i].z = static_cast<int>(dropper.textStruct.x + biggestWidth * screenWidth / 2.f);
				//printf("x z clickbox - %d:%d \n", clickBoxes[i].x, clickBoxes[i].z);
			}
			else if (align == TA_right) {
				clickBoxes[i].x = static_cast<int>(dropper.textStruct.x - biggestWidth * screenWidth / 2.f);
				clickBoxes[i].z = static_cast<int>(dropper.textStruct.x);
			}
		}

		glm::ivec4 bigBox;
		bigBox.x = clickBoxes[0].x;
		bigBox.z = clickBoxes[0].z;

		bigBox.y = clickBoxes[0].y;
		bigBox.w = clickBoxes.back().w;

		glm::ivec2 screenPosition;
		screenPosition.x = (bigBox.x + bigBox.z) / 2;
		screenPosition.y = (bigBox.y + bigBox.w) / 2;

		UIComp::convertScreenTo2D(screenPosition, dropBackground.translation, screenWidth, screenHeight);
		printf("translation - %.2f : %.2f \n", dropBackground.translation.x, dropBackground.translation.y);

		dropBackground.scale = glm::vec2(biggestWidth * screenWidth / DEFAULT_WIDTH, scale * clickBoxes.size() / 19.f);
	}

	int8_t DropBox::Clicked(double xpos, double ypos) {
		printf("checking dropbox click \n");
		if (!currentlyDropped) {
			UIComp::printClickBox(dropper.clickBox);
			if (UIComp::checkClickBox(dropper.clickBox, xpos, ypos)) {
				UIComp::printClickBox(dropper.clickBox);
				printf("clicked the dropper\n");
				currentlyDropped = true;
				return -1;
			}
		}
		else {
			for (int i = 0; i < clickBoxes.size(); i++) {
				if (UIComp::checkClickBox(clickBoxes[i], xpos, ypos)) {
					//UIComp::printClickBox(clickBoxes[i]);
					currentlyDropped = false;
					return i;
				}
			}
			currentlyDropped = false;
		}
		return -2;
	}

	// ~~~~~~~~~~~~~~~~~~~~~ BUTTON ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Button::Button(glm::vec2 translation, float screenWidth, float screenHeight) {
		transform.translation = translation;
		transform.scale.y = 1.f / 20.f;
		transform.scale.x = transform.scale.y * screenHeight / screenWidth;
		UIComp::convertTransformToClickBox(transform, clickBox, screenWidth, screenHeight);
	}

	// ~~~~~~~~~~~~~~~~~~~~ CHECKBOX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Checkbox::Checkbox(std::string labelString, glm::vec2 translation, DefaultOffsets labelOffset, float screenW, float screenH) : button{ translation, screenW, screenH } {
		TextAlign alignment = TA_center;

		glm::vec2 textPos = glm::vec2{ (button.clickBox.x + button.clickBox.z) / 2, button.clickBox.y };
		switch (labelOffset) {
		case DO_left: {
			textPos.x = static_cast<float>(button.clickBox.x);
			alignment = TA_right;
			break;
		}
		case DO_above: {
			textPos.y = static_cast<float>(button.clickBox.y) + 20.f;
			alignment = TA_center;
			break;
		}
		case DO_right: {
			textPos.x = static_cast<float>(button.clickBox.z);
			alignment = TA_left;
			break;
		}
		case DO_below: {
			textPos.y = static_cast<float>(button.clickBox.w) - 20.f;
			alignment = TA_center;
			break;
		}
		}

		label = TextStruct{ labelString, textPos.x, textPos.y, alignment, 1.f };
	}
	Checkbox::Checkbox(std::string labelString, glm::vec2 translation, glm::vec2 labelOffset, TextAlign alignment, float screenW, float screenH) : button{ translation, screenW, screenH } {
		label = TextStruct{ labelString, button.transform.translation.x + labelOffset.x, button.transform.translation.y + labelOffset.y, (unsigned char)alignment, 1.f };
	}
	void Checkbox::resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) {
		label.x *= rszWidth / oldWidth;
		label.y *= rszHeight / oldHeight;
		button.resizeWindow(rszWidth, rszHeight);
	}

	bool Checkbox::Clicked(double xpos, double ypos) {
		if (button.Clicked(xpos, ypos)) {
			isChecked = !isChecked;
			return true;
		}
		return false;
	}


}