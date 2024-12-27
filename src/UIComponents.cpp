#include "EWEngine/GUI/UIComponents.h"
namespace EWE {

	
	//end uicomp

	//  ~~~~~~~~~~~~~~~~~~~ CLICKBOX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ClickTextBox::ClickTextBox(std::string string, float x, float y, unsigned char align, float scale) : textStruct{ string, x, y, align, scale } {
		UIComp::TextToTransform(transform, textStruct, clickBox, VK::Object->screenWidth, VK::Object->screenHeight);
	}
	ClickTextBox::ClickTextBox(TextStruct textStruct) : textStruct{ textStruct } {
		UIComp::TextToTransform(transform, textStruct, clickBox, VK::Object->screenWidth, VK::Object->screenHeight);
	}
	void ClickTextBox::ResizeWindow(glm::vec2 rescalingRatio) {
		printf("click text resize \n");
		textStruct.x *= rescalingRatio.x;
		textStruct.y *= rescalingRatio.y;
		UIComp::TextToTransform(transform, textStruct, clickBox, VK::Object->screenWidth, VK::Object->screenHeight);
	}
	bool ClickTextBox::Clicked(double xpos, double ypos) {
		return UIComp::CheckClickBox(clickBox, xpos, ypos);
	}
	void ClickTextBox::Render(Array2DPushConstantData& push) {
		push.scaleOffset.z = transform.translation.x;
		push.scaleOffset.w = transform.translation.y;
		push.scaleOffset.x = transform.scale.x;
		push.scaleOffset.y = transform.scale.y;
		Dimension2::PushAndDraw(push);
	}
	
	// ~~~~~~~~~~~~~~~~~~~~~ TYPE BOX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	TypeBox* TypeBox::textBoxPointer;

	void TypeBox::MouseCallback(GLFWwindow* window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS) {
				double xpos = 0;
				double ypos = 0;
				glfwGetCursorPos(window, &xpos, &ypos);
				if (UIComp::CheckClickBox(textBoxPointer->clickBox, xpos, ypos)) {
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
	void TypeBox::TypeCallback(GLFWwindow* window, uint32_t codepoint) {
		printf("key typed? - %ud \n", codepoint);

		UIComp::TypeToString(textBoxPointer->textStruct.string, textBoxPointer->maxStringLength, codepoint, textBoxPointer->inputType, static_cast<uint8_t>(textBoxPointer->textStruct.string.length()));
	}
	TypeBox::TypeBox(TextStruct textStruct) : textStruct{ textStruct } {
		UIComp::TextToTransform(transform, textStruct, clickBox, VK::Object->screenWidth, VK::Object->screenHeight);
	}
	TypeBox::TypeBox(std::string string, float x, float y, uint8_t alignment, float scale) : textStruct{ string, x, y, alignment, scale } {
		UIComp::TextToTransform(transform, textStruct, clickBox, VK::Object->screenWidth, VK::Object->screenHeight);
	}
	void TypeBox::GiveGLFWCallbacks(GLFWwindow* windowPtr, GLFWmousebuttonfun mouseReturnFunction, GLFWkeyfun keyReturnFunction) {
		readyForInput = true;
		textBoxPointer = this;
		glfwSetCharCallback(windowPtr, TypeCallback);
		glfwSetKeyCallback(windowPtr, KeyCallback);
		glfwSetMouseButtonCallback(windowPtr, MouseCallback);
		mouseReturnPointer = mouseReturnFunction;
		keyReturnPointer = keyReturnFunction;
	}

	void TypeBox::ResizeWindow(glm::vec2 rescalingRatio) {
		printf("click text resize \n");
		textStruct.x *= rescalingRatio.x;
		textStruct.y *= rescalingRatio.y;
		UIComp::TextToTransform(transform, textStruct, clickBox, VK::Object->screenWidth, VK::Object->screenHeight);
	}
	void TypeBox::Render(Array2DPushConstantData& push) {
		push.scaleOffset.z = transform.translation.x;
		push.scaleOffset.w = transform.translation.y;
		//need color array
		push.scaleOffset.x = transform.scale.x;
		push.scaleOffset.y = transform.scale.y;
		Dimension2::PushAndDraw(push);
		//printf("drawing click text \n");
	}
	
	// ~~~~~~~~~~~~~~~~~~~~~ SLIDER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void Slider::SetTransform(glm::vec2 newTrans) {
		bracket.translation = newTrans;
		slider.translation = newTrans;
		bracketButtons.first.translation = newTrans;
		bracketButtons.second.translation = newTrans;
		bracketButtons.first.translation.x -= 0.1f;
		bracketButtons.second.translation.x += 0.1f;
	}
	void Slider::Init(glm::vec2 initTrans, uint8_t currentSens) {
		VolumeTrueSensFalse = false;
		mySens = currentSens;
		Init(initTrans, static_cast<float>(currentSens) / 100.f);
	}
	void Slider::Init(glm::vec2 initTrans, float currentVolume) {
		bracket.translation = initTrans;

		bracketButtons.first.scale = { 0.1f, 0.2f };
		bracketButtons.second.scale = { 0.1f, 0.2f };
		bracket.scale = { .5f, .125f };
		slider.scale = { 0.05f, 0.1f };

		bracketButtons.first.translation = initTrans;
		bracketButtons.first.translation.x -= (bracket.scale.x / 2.f) + (bracketButtons.first.scale.x / 2.f);
		bracketButtons.second.translation = initTrans;
		bracketButtons.second.translation.x += (bracket.scale.x / 2.f) + (bracketButtons.second.scale.x / 2.f); //first and second should have idenctical scale

		UIComp::ConvertTransformToClickBox(bracketButtons.first, click[0], VK::Object->screenWidth, VK::Object->screenHeight);
		UIComp::ConvertTransformToClickBox(bracketButtons.second, click[2], VK::Object->screenWidth, VK::Object->screenHeight);

		//now on to the slider
		slidePosition = currentVolume;
		//get space between transforms, thats movement per pixel / 99
		spaceBetween = (bracketButtons.second.translation.x - (bracketButtons.second.scale.x / 2)) - (bracketButtons.first.translation.x + (bracketButtons.first.scale.x / 2)) - slider.scale.x;
		//printf("space between, which volume ~ %.2f : %.2f \n", spaceBetween, currentVolume);
		slider.translation.x = bracket.translation.x + (spaceBetween * (slidePosition - 0.5f));
		slider.translation.y = initTrans.y;
		UIComp::ConvertTransformToClickBox(slider, click[1], VK::Object->screenWidth, VK::Object->screenHeight);

		//setActivity(false);
	}
	void Slider::SetSliderPosition(float sliderPos) {
		slidePosition = sliderPos;
		spaceBetween = (bracketButtons.second.translation.x - (bracketButtons.second.scale.x / 2)) - (bracketButtons.first.translation.x + (bracketButtons.first.scale.x / 2)) - slider.scale.x;
		//printf("space between, which volume ~ %.2f : %.2f \n", spaceBetween, currentVolume);
		slider.translation.x = bracket.translation.x + (spaceBetween * (slidePosition - 0.5f));
		UIComp::ConvertTransformToClickBox(slider, click[1], VK::Object->screenWidth, VK::Object->screenHeight);
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
			UIComp::ConvertTransformToClickBox(slider, click[1], VK::Object->screenWidth, VK::Object->screenHeight);
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
			UIComp::ConvertTransformToClickBox(slider, click[1], VK::Object->screenWidth, VK::Object->screenHeight);
		}
		//printf("slide position : %.2f \n", slidePosition);
	}

	void Slider::ResizeWindow() {
		//printf("slider resize window \n");
		UIComp::ConvertTransformToClickBox(bracketButtons.first, click[0], VK::Object->screenWidth, VK::Object->screenHeight);
		UIComp::ConvertTransformToClickBox(slider, click[1], VK::Object->screenWidth, VK::Object->screenHeight);
		UIComp::ConvertTransformToClickBox(bracketButtons.second, click[2], VK::Object->screenWidth, VK::Object->screenHeight);
	}
	void Slider::GiveSens(uint8_t currentSens) {
		VolumeTrueSensFalse = false;
		mySens = currentSens;
		slidePosition = mySens / 100.f;
		slider.translation.x = bracket.translation.x + (spaceBetween * (slidePosition - 0.5f));
		UIComp::ConvertTransformToClickBox(slider, click[1], VK::Object->screenWidth, VK::Object->screenHeight);
		//MoveSlider();
	}
	int8_t Slider::Clicked(double xpos, double ypos) {
		//printf("active click check ~ %.3f : %.3f \n", xpos, ypos);
		//printf("check the click box, xzyw ~ %d:%d:%d:%d \n", click[0].x, click[0].y, click[0].z, click[0].w);
		if (UIComp::CheckClickBox(click[0], xpos, ypos)) {
			ButtonClicked(false);
			return 0;
		}
		else if (UIComp::CheckClickBox(click[1], xpos, ypos)) {
			return 1;
		}
		else if (UIComp::CheckClickBox(click[2], xpos, ypos)) {
			ButtonClicked(true);
			return 2;
		}

		return -1;
	}
	void Slider::ButtonClicked(bool leftFalseRightTrue) {
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
		UIComp::ConvertTransformToClickBox(slider, click[1], VK::Object->screenWidth, VK::Object->screenHeight);
	}
	void Slider::Render(Array2DPushConstantData& push, uint8_t drawID) {
		switch (drawID) {
		case 0: {
			push.scaleOffset = glm::vec4(slider.scale, slider.translation);
			break;
		}
		case 1: {
			push.scaleOffset = glm::vec4(bracketButtons.first.scale, bracketButtons.first.translation);
			Dimension2::PushAndDraw(push);
			push.scaleOffset = glm::vec4(bracketButtons.second.scale, bracketButtons.second.translation);
			break;
		}
		case 2: {
			push.scaleOffset = glm::vec4(bracket.scale, bracket.translation);
			break;
		}
		}
		Dimension2::PushAndDraw(push);
	}

	// ~~~~~~~~~~~~~~~~~~~~~ COMBOBOX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	ComboBox::ComboBox(TextStruct textStruct) {
		//printf("COMBOBOX CONSTRUCTION GIVEN ? %s  %.2f:%.2f\n", textStruct.string.c_str(), textStruct.x, textStruct.y);
		activeOption.textStruct = textStruct;
		UIComp::TextToTransform(activeOption.transform, activeOption.textStruct, activeOption.clickBox, VK::Object->screenWidth, VK::Object->screenHeight);
		//printf("COMBOBOX CONSTRUCTION CONSTRUCTED : %s  %.2f:%.2f\n", activeOption.textStruct.string.c_str(), activeOption.textStruct.x, activeOption.textStruct.y);
		scale = activeOption.textStruct.scale;
	}

	void ComboBox::PushOption(std::string string) {
		//printf("pushing option, comboX:comboY:scale - %.3f:%.3f:%.1f \n", activeOption.textStruct.x, activeOption.textStruct.y, scale);
		comboOptions.emplace_back(TextStruct{ string, activeOption.textStruct.x, activeOption.textStruct.y + (comboOptions.size() + 1) * (19.f * scale) * VK::Object->screenHeight / DEFAULT_HEIGHT, align, scale });
		//comboOptions.back().clickBox = activeOption.clickBox;
		if ((comboOptions.back().clickBox.z - comboOptions.back().clickBox.x) > (activeOption.clickBox.z - activeOption.clickBox.x)) {
			activeOption.clickBox.z = activeOption.clickBox.x + comboOptions.back().clickBox.z - comboOptions.back().clickBox.x;
			for (int i = 0; i < comboOptions.size() - 1; i++) {
				comboOptions[i].clickBox.z = activeOption.clickBox.z;
			}
		}

		comboOptions.back().clickBox.x = activeOption.clickBox.x;
		comboOptions.back().clickBox.z = activeOption.clickBox.z;
		UIComp::ConvertScreenTo2D(glm::ivec2{ (comboOptions.back().clickBox.x + comboOptions.back().clickBox.z) / 2 ,
			(comboOptions.back().clickBox.y + comboOptions.back().clickBox.w) / 2 }, comboOptions.back().transform.translation, VK::Object->screenWidth, VK::Object->screenHeight);
		comboOptions.back().transform.scale.x = activeOption.transform.scale.x;
	}

	void ComboBox::SetSelection(int8_t selection) {
		currentlySelected = selection;
		activeOption.textStruct.string = comboOptions[currentlySelected].textStruct.string;
	}

	bool ComboBox::Clicked(double xpos, double ypos) {
		if (!currentlyDropped) {
			if (UIComp::CheckClickBox(activeOption.clickBox, xpos, ypos)) {
				UIComp::PrintClickBox(activeOption.clickBox);
				currentlyDropped = true;
				return true;
			}
		}
		else {
			for (int i = 0; i < comboOptions.size(); i++) {
				if (UIComp::CheckClickBox(comboOptions[i].clickBox, xpos, ypos)) {
					UIComp::PrintClickBox(comboOptions[i].clickBox);
					currentlyDropped = false;
					activeOption.textStruct.string = comboOptions[i].textStruct.string;
					currentlySelected = i;
					return true;
				}
			}
			currentlyDropped = false;
		}
		return false;
	}
	void ComboBox::ResizeWindow(glm::vec2 rescalingRatio) {

		activeOption.ResizeWindow(rescalingRatio);
		for (int i = 0; i < comboOptions.size(); i++) {
			comboOptions[i].ResizeWindow(rescalingRatio);
			comboOptions[i].textStruct.x = activeOption.textStruct.x;
			comboOptions[i].textStruct.y = activeOption.textStruct.y + (i + 1) * (19.f * scale) * VK::Object->screenHeight / DEFAULT_HEIGHT;

			if ((comboOptions[i].clickBox.z - comboOptions[i].clickBox.x) > (activeOption.clickBox.z - activeOption.clickBox.x)) {
				activeOption.clickBox.z = activeOption.clickBox.x + comboOptions[i].clickBox.z - comboOptions[i].clickBox.x;
				for (int j = 0; j < i; j++) {
					comboOptions[j].clickBox.z = activeOption.clickBox.z;
					UIComp::ConvertScreenTo2D(glm::ivec2{ (comboOptions[j].clickBox.x + comboOptions[j].clickBox.z) / 2 ,
						(comboOptions[j].clickBox.y + comboOptions[j].clickBox.w) / 2 }, comboOptions[j].transform.translation, VK::Object->screenWidth, VK::Object->screenHeight);
				}
			}
			else {
				comboOptions[i].clickBox.z = activeOption.clickBox.z;
			}
		}
	}
	void ComboBox::Render(Array2DPushConstantData& push) {
		push.scaleOffset.z = activeOption.transform.translation.x;
		push.scaleOffset.w = activeOption.transform.translation.y;
		//need color array
		push.scaleOffset.x = activeOption.transform.scale.x;
		push.scaleOffset.y = activeOption.transform.scale.y;
		Dimension2::PushAndDraw(push);
		if (currentlyDropped) {
			for (int j = 0; j < comboOptions.size(); j++) {
				push.scaleOffset.z = comboOptions[j].transform.translation.x;
				push.scaleOffset.w = comboOptions[j].transform.translation.y;
				push.scaleOffset.x = comboOptions[j].transform.scale.x;
				push.scaleOffset.y = comboOptions[j].transform.scale.y;
				if (j == currentlySelected) {
					push.color = glm::vec3{ .4f, .4f, 1.f };
					Dimension2::PushAndDraw(push);
					push.color = glm::vec3{ .5f, .35f, .25f };
				}
				else {
					Dimension2::PushAndDraw(push);
				}
			}
		}
	}
	void ComboBox::Move(float xDiff, float yDiff) {
		activeOption.textStruct.x += xDiff;
		activeOption.textStruct.y += yDiff;
		activeOption.clickBox.x += static_cast<int>(xDiff);
		activeOption.clickBox.z += static_cast<int>(xDiff);
		activeOption.clickBox.y += static_cast<int>(yDiff);
		activeOption.clickBox.w += static_cast<int>(yDiff);
		UIComp::ConvertClickToTransform(activeOption.clickBox, activeOption.transform, VK::Object->screenWidth, VK::Object->screenHeight);

		for (auto& cOption : comboOptions) {
			cOption.textStruct.x += xDiff;
			cOption.textStruct.y += yDiff;
			cOption.clickBox.x += static_cast<int>(xDiff);
			cOption.clickBox.z += static_cast<int>(xDiff);
			cOption.clickBox.y += static_cast<int>(yDiff);
			cOption.clickBox.w += static_cast<int>(yDiff);
			UIComp::ConvertClickToTransform(cOption.clickBox, cOption.transform, VK::Object->screenWidth, VK::Object->screenHeight);
		}
	}

	// ~~~~~~~~~~~~~~~~~~~~~~ DROPBOX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void DropBox::PushOption(std::string pushString) {
		dropOptions.push_back(dropper.textStruct);
		dropOptions.back().string = pushString;
		Init();
	}
	void DropBox::Init() {
		//if dropOptions.size == 0, this will crash

		if (dropOptions.size() != clickBoxes.size() && (dropOptions.size() != 0)) {
			clickBoxes.resize(dropOptions.size());
		}
		UIComp::TextToTransform(dropper.transform, dropper.textStruct, dropper.clickBox, VK::Object->screenWidth, VK::Object->screenHeight);

		float biggestWidth = 0.f;
		for (int i = 0; i < dropOptions.size(); i++) {
			dropOptions[i].x = dropper.textStruct.x;
			dropOptions[i].y = dropper.textStruct.y + 13.f + (i + 1) * (26.f * scale) * VK::Object->screenHeight / DEFAULT_HEIGHT;

			float tempWidth = dropOptions[i].GetWidth();
			if (tempWidth > biggestWidth) { biggestWidth = tempWidth; }

			clickBoxes[i].y = static_cast<int>(dropOptions[i].y);
			clickBoxes[i].w = static_cast<int>(dropOptions[i].y + (26.f * scale) * VK::Object->screenHeight / DEFAULT_HEIGHT);
		}
		for (int i = 0; i < dropOptions.size(); i++) {
			if (align == TA_center) {
				clickBoxes[i].x = static_cast<int>(dropper.textStruct.x - biggestWidth * VK::Object->screenWidth / 4.f);
				clickBoxes[i].z = static_cast<int>(dropper.textStruct.x + biggestWidth * VK::Object->screenWidth / 4.f);
			}
			else if (align == TA_left) {
				clickBoxes[i].x = static_cast<int>(dropper.textStruct.x);
				clickBoxes[i].z = static_cast<int>(dropper.textStruct.x + biggestWidth * VK::Object->screenWidth / 2.f);
				//printf("x z clickbox - %d:%d \n", clickBoxes[i].x, clickBoxes[i].z);
			}
			else if (align == TA_right) {
				clickBoxes[i].x = static_cast<int>(dropper.textStruct.x - biggestWidth * VK::Object->screenWidth / 2.f);
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

		UIComp::ConvertScreenTo2D(screenPosition, dropBackground.translation, VK::Object->screenWidth, VK::Object->screenHeight);
		printf("translation - %.2f : %.2f \n", dropBackground.translation.x, dropBackground.translation.y);

		dropBackground.scale = glm::vec2(biggestWidth * VK::Object->screenWidth / DEFAULT_WIDTH, scale * clickBoxes.size() / 19.f);
	}

	int8_t DropBox::Clicked(double xpos, double ypos) {
		printf("checking dropbox click \n");
		if (!currentlyDropped) {
			UIComp::PrintClickBox(dropper.clickBox);
			if (UIComp::CheckClickBox(dropper.clickBox, xpos, ypos)) {
				UIComp::PrintClickBox(dropper.clickBox);
				printf("clicked the dropper\n");
				currentlyDropped = true;
				return -1;
			}
		}
		else {
			for (int i = 0; i < clickBoxes.size(); i++) {
				if (UIComp::CheckClickBox(clickBoxes[i], xpos, ypos)) {
					//UIComp::printClickBox(clickBoxes[i]);
					currentlyDropped = false;
					return i;
				}
			}
			currentlyDropped = false;
		}
		return -2;
	}
	void DropBox::Render(Array2DPushConstantData& push) {
		push.scaleOffset.z = dropper.transform.translation.x;
		push.scaleOffset.w = dropper.transform.translation.y;
		//need color array
		if (currentlyDropped) {
			push.color = glm::vec3{ .75f, .35f, .25f };
		}
		push.scaleOffset.x = dropper.transform.scale.x;
		push.scaleOffset.y = dropper.transform.scale.y;
		Dimension2::PushAndDraw(push);
		push.color = glm::vec3{ .5f, .35f, .25f };
		if (currentlyDropped) {
			push.scaleOffset.z = dropBackground.translation.x;
			push.scaleOffset.w = dropBackground.translation.y;
			push.scaleOffset.x = dropBackground.scale.x;
			push.scaleOffset.y = dropBackground.scale.y;
			Dimension2::PushAndDraw(push);
		}
	}

	// ~~~~~~~~~~~~~~~~~~~~~ BUTTON ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Button::Button(glm::vec2 translation) {
		transform.translation = translation;
		transform.scale.y = 1.f / 20.f;
		transform.scale.x = transform.scale.y * VK::Object->screenHeight / VK::Object->screenWidth;
		UIComp::ConvertTransformToClickBox(transform, clickBox, VK::Object->screenWidth, VK::Object->screenHeight);
	}
	void Button::Render(Array2DPushConstantData& push) {
		push.scaleOffset = glm::vec4(transform.scale, transform.translation);
		Dimension2::PushAndDraw(push);
	}

	// ~~~~~~~~~~~~~~~~~~~~ CHECKBOX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Checkbox::Checkbox(std::string labelString, glm::vec2 translation, DefaultOffsets labelOffset) : button{ translation } {
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
	Checkbox::Checkbox(std::string labelString, glm::vec2 translation, glm::vec2 labelOffset, TextAlign alignment) : button{ translation} {
		label = TextStruct{ labelString, button.transform.translation.x + labelOffset.x, button.transform.translation.y + labelOffset.y, (unsigned char)alignment, 1.f };
	}
	void Checkbox::ResizeWindow(glm::vec2 rescalingRatio) {
		label.x *= rescalingRatio.x;
		label.y *= rescalingRatio.y;
		button.ResizeWindow();
	}

	bool Checkbox::Clicked(double xpos, double ypos) {
		if (button.Clicked(xpos, ypos)) {
			isChecked = !isChecked;
			return true;
		}
		return false;
	}
	void Checkbox::Render(Array2DPushConstantData& push) {
		push.scaleOffset = glm::vec4(button.transform.scale, button.transform.translation);
		Dimension2::PushAndDraw(push);
	}

}