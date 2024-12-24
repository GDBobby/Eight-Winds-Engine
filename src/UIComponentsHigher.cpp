#include "EWEngine/GUI/UIComponentsHigher.h"

namespace EWE {

	// ~~~~~~~~~~~~~~~~~~~~~ VARIABLE CONTROL ~~~~~~~~~~~~~~~~~~~~
	VariableControl* VariableControl::variableCtrlPtr;

	VariableControl::VariableControl(GLFWwindow* windowPtr, float posX, float posY, float width, std::string dataLabelString, void* dataPointer, UIComp::VariableType dataType, uint8_t dataCount, void* steps)
		: dataPtr{ dataPointer }, dataType{ dataType }, dataCount{ dataCount }, windowPtr{ windowPtr }, width{ width }
	{
		assert(dataCount > 0);
		size_t variableSize = UIComp::GetVariableSize(dataType);
		dataLabel = TextStruct{ dataLabelString, posX, posY, TA_left, 1.f };
		//assert(sizeof(T) == variableSize);

		this->steps = malloc(variableSize * 3);

		memcpy(this->steps, steps, variableSize * 3);

		float ratioWidth = width * VK::Object->screenWidth / DEFAULT_WIDTH;

		glm::ivec2 buttonScreen;
		glm::vec2 buttonTranslation;
		float verticalSpacing = 26.6f * VK::Object->screenHeight / DEFAULT_HEIGHT;


		for (int i = 0; i < dataCount; i++) {
			TypeBox& typeRef = typeBoxes.emplace_back(GetVariableString(this->dataPtr, i, this->dataType), this->dataLabel.x, this->dataLabel.y + (verticalSpacing * (i + 1)), TA_left, 1.f);
			typeRef.inputType = UIComp::InputType_numeric;

			//only supporting align_left right now
			typeRef.clickBox.z = static_cast<int>(typeRef.clickBox.x + ratioWidth);
			typeRef.transform.translation.x = ((typeRef.clickBox.x + ratioWidth / 2) - (VK::Object->screenWidth / 2.f)) / (VK::Object->screenWidth / 2.f);
			typeRef.transform.scale.x = (ratioWidth / (DEFAULT_WIDTH / 2.f));

			buttonScreen = glm::ivec2(dataLabel.x + ratioWidth, (typeRef.clickBox.y + typeRef.clickBox.w) / 2); //? this is lining up the buttons with the top of textbox
			UIComp::ConvertScreenTo2D(buttonScreen, buttonTranslation, VK::Object->screenWidth, VK::Object->screenHeight);

			std::pair<Button, Button>& buttonRef = buttons.emplace_back(std::piecewise_construct, std::make_tuple(buttonTranslation, VK::Object->screenWidth, VK::Object->screenHeight), std::make_tuple(buttonTranslation, VK::Object->screenWidth, VK::Object->screenHeight));
			buttonRef.first.transform.translation.x += buttonRef.first.transform.scale.x / 2.f;
			buttonRef.second.transform.translation.x += buttonRef.second.transform.scale.x * 1.55f;
			buttonRef.first.transform.scale *= .8f;
			UIComp::ConvertTransformToClickBox(buttonRef.first.transform, buttonRef.first.clickBox, VK::Object->screenWidth, VK::Object->screenHeight);
			buttonRef.second.transform.scale *= .8f;
			UIComp::ConvertTransformToClickBox(buttonRef.second.transform, buttonRef.second.clickBox, VK::Object->screenWidth, VK::Object->screenHeight);

			buttonRef.second.transform.scale.y *= -1.f;
		}
	}

	bool VariableControl::Clicked(double xpos, double ypos) {
		for (int j = 0; j < typeBoxes.size(); j++) {
			if (typeBoxes[j].Clicked(xpos, ypos)) {
				selectedTypeBox = j;
				variableCtrlPtr = this;
				typeBoxes[selectedTypeBox].readyForInput = true;
				typeBoxes[selectedTypeBox].stringSelectionIndex = static_cast<int>(typeBoxes[j].textStruct.string.length());
				typeBoxes[selectedTypeBox].textStruct.string += '|';

				glfwSetCharCallback(windowPtr, typeCallback);
				glfwSetKeyCallback(windowPtr, KeyCallback);
				glfwSetMouseButtonCallback(windowPtr, MouseCallback);

				return true;
			}
		}
		for (int j = 0; j < buttons.size(); j++) {
			if (buttons[j].first.Clicked(xpos, ypos)) {
				//if holding shift largstep
				//if holding ctrl smallStep
				//*variables[j] += mediumStep;
				printf("adding \n");
				int stepOffset = 1 - glfwGetKey(windowPtr, GLFW_KEY_LEFT_CONTROL) + glfwGetKey(windowPtr, GLFW_KEY_LEFT_SHIFT);
				AddVariables(dataPtr, j, steps, stepOffset, dataType);
				typeBoxes[j].textStruct.string = GetVariableString(dataPtr, j, dataType);
				return false;
			}
			else if (buttons[j].second.Clicked(xpos, ypos)) {
				//if holding shift largstep
				//if holding ctrl smallStep
				//*variables[j] -= mediumStep;
				printf("subtracting \n");
				int stepOffset = 1 - glfwGetKey(windowPtr, GLFW_KEY_LEFT_CONTROL) + glfwGetKey(windowPtr, GLFW_KEY_LEFT_SHIFT);
				SubtractVariables(dataPtr, j, steps, stepOffset, dataType);
				typeBoxes[j].textStruct.string = GetVariableString(dataPtr, j, dataType);
				return false;
			}
		}
		return false;
	}
	void VariableControl::ResizeWindow(glm::vec2 resizeRatio) {

		for (int i = 0; i < buttons.size(); i++) {
			buttons[i].first.clickBox.x = static_cast<int>(static_cast<float>(buttons[i].first.clickBox.x) * resizeRatio.x);
			buttons[i].first.clickBox.z = static_cast<int>(static_cast<float>(buttons[i].first.clickBox.z) * resizeRatio.x);
			buttons[i].first.clickBox.y = static_cast<int>(static_cast<float>(buttons[i].first.clickBox.y) * resizeRatio.y);
			buttons[i].first.clickBox.w = static_cast<int>(static_cast<float>(buttons[i].first.clickBox.w) * resizeRatio.y);

			buttons[i].second.clickBox.x = static_cast<int>(static_cast<float>(buttons[i].second.clickBox.x) * resizeRatio.x);
			buttons[i].second.clickBox.z = static_cast<int>(static_cast<float>(buttons[i].second.clickBox.z) * resizeRatio.x);
			buttons[i].second.clickBox.y = static_cast<int>(static_cast<float>(buttons[i].second.clickBox.y) * resizeRatio.y);
			buttons[i].second.clickBox.w = static_cast<int>(static_cast<float>(buttons[i].second.clickBox.w) * resizeRatio.y);

			//UIComp::convertClickToTransform(buttons[i].first.clickBox, buttons[i].first.transform, screenWidth, screenHeight);
			//UIComp::convertClickToTransform(buttons[i].second.clickBox, buttons[i].second.transform, screenWidth, screenHeight);
			//buttons[i].first.transform.scale *= .8f;
			//buttons[i].second.transform.scale.y *= -0.8f;
			//buttons[i].second.transform.scale.x *= 0.8f;
		}
		for (int i = 0; i < typeBoxes.size(); i++) {

			typeBoxes[i].textStruct.x *= resizeRatio.x;
			typeBoxes[i].textStruct.y *= resizeRatio.y;
			typeBoxes[i].clickBox.x = static_cast<int>(static_cast<float>(typeBoxes[i].clickBox.x) * resizeRatio.x);
			typeBoxes[i].clickBox.z = static_cast<int>(static_cast<float>(typeBoxes[i].clickBox.z) * resizeRatio.x);
			typeBoxes[i].clickBox.y = static_cast<int>(static_cast<float>(typeBoxes[i].clickBox.y) * resizeRatio.y);
			typeBoxes[i].clickBox.w = static_cast<int>(static_cast<float>(typeBoxes[i].clickBox.w) * resizeRatio.y);

			//UIComp::convertClickToTransform(typeBoxes[i].clickBox, typeBoxes[i].transform, screenWidth, screenHeight);
		}

		dataLabel.x *= resizeRatio.x;
		dataLabel.y *= resizeRatio.y;
		//for (int i = 0; i < variableNames.size(); i++) {
		//	variableNames[i].x *= xRatio;
		//	variableNames[i].y *= yRatio;
		//}
		//dragBox.x *= xRatio;
		//dragBox.z *= xRatio;
		//dragBox.y *= yRatio;
		//dragBox.w *= yRatio;
		//shouldnt need to resize the transform?
	}

	void VariableControl::giveGLFWCallbacks(GLFWmousebuttonfun mouseReturnFunction, GLFWkeyfun keyReturnFunction) {
		printf("giving glfw callbacks \n");
		mouseReturnPointer = mouseReturnFunction;
		keyReturnPointer = keyReturnFunction;
	}

	void VariableControl::moveBox(int freshX, int freshY) {
		int movementX = freshX - lastPos.first;
		int movementY = freshY - lastPos.second;

		lastPos.first = freshX;
		lastPos.second = freshY;

		dataLabel.x += static_cast<float>(movementX);
		dataLabel.y += static_cast<float>(movementY);
		//for (int i = 0; i < variableNames.size(); i++) {
		//	variableNames[i].x += movementX;
		//	variableNames[i].y += movementY;
		//}
		for (int i = 0; i < typeBoxes.size(); i++) {
			typeBoxes[i].textStruct.x += static_cast<float>(movementX);
			typeBoxes[i].textStruct.y += static_cast<float>(movementY);
			typeBoxes[i].clickBox.x += movementX;
			typeBoxes[i].clickBox.z += movementX;
			typeBoxes[i].clickBox.y += movementY;
			typeBoxes[i].clickBox.w += movementY;

			UIComp::ConvertClickToTransform(typeBoxes[i].clickBox, typeBoxes[i].transform, VK::Object->screenWidth, VK::Object->screenHeight);
		}
		for (int i = 0; i < buttons.size(); i++) {
			buttons[i].first.clickBox.x += movementX;
			buttons[i].first.clickBox.z += movementX;
			buttons[i].first.clickBox.y += movementY;
			buttons[i].first.clickBox.w += movementY;

			buttons[i].second.clickBox.x += movementX;
			buttons[i].second.clickBox.z += movementX;
			buttons[i].second.clickBox.y += movementY;
			buttons[i].second.clickBox.w += movementY;

			UIComp::ConvertClickToTransform(buttons[i].first.clickBox, buttons[i].first.transform, VK::Object->screenWidth, VK::Object->screenHeight);
			UIComp::ConvertClickToTransform(buttons[i].second.clickBox, buttons[i].second.transform, VK::Object->screenWidth, VK::Object->screenHeight);
			//buttons[i].first.transform.scale *= 1.f;
			buttons[i].second.transform.scale.y *= -1.0f;
			buttons[i].second.transform.scale.x *= 1.f;
		}
		//dragBox.x += movementX;
		//dragBox.z += movementX;
		//dragBox.y += movementY;
		//dragBox.w = typeBoxes.back().clickBox.w;
		//UIComp::convertClickToTransform(dragBox, transform, screenWidth, screenHeight);
		//dragBox.w = dragBox.y + label.scale * 26.f;

		/*
		if (glfwGetMouseButton(windowPtr, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) {
			glfwSetCursorPosCallback(windowPtr, NULL);
			mouseDragging = false;
		}
		*/
	}

	//im moving the callbacks to typeBox

	
	void VariableControl::MouseCallback(GLFWwindow* window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS) {
				if (variableCtrlPtr->selectedTypeBox < 0) {
					return;
				}
				double xpos = 0;
				double ypos = 0;
				glfwGetCursorPos(window, &xpos, &ypos);
				bool clickedABox = false;
				for (int i = 0; i < variableCtrlPtr->typeBoxes.size(); i++) {
					if (UIComp::CheckClickBox(variableCtrlPtr->typeBoxes[i].clickBox, xpos, ypos)) {
						if (i != variableCtrlPtr->selectedTypeBox) {
							variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.erase(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.find_first_of('|'), 1);
							printf("remove the | \n");
							variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string = GetVariableString(variableCtrlPtr->dataPtr, variableCtrlPtr->selectedTypeBox, variableCtrlPtr->dataType);
							variableCtrlPtr->selectedTypeBox = i;
						}
						printf("clicked a type box? \n");
						variableCtrlPtr->stringSelectionIndex = variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.GetSelectionIndex(xpos);
						std::string& stringRef = variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string;
						size_t indexPos = stringRef.find_first_of('|');
						if ((indexPos != variableCtrlPtr->stringSelectionIndex) && (indexPos != (variableCtrlPtr->stringSelectionIndex + 1))) {
							if (indexPos != stringRef.npos) {
								stringRef.erase(indexPos, 1);
								variableCtrlPtr->stringSelectionIndex -= variableCtrlPtr->stringSelectionIndex > indexPos;
							}
							stringRef.insert(stringRef.begin() + variableCtrlPtr->stringSelectionIndex, '|');
						}
						printf("string selection index? : %d \n", variableCtrlPtr->stringSelectionIndex);
						variableCtrlPtr->readyForInput = true;
						clickedABox = true;
						break;
					}
				}
				if (!clickedABox) {
					variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string = GetVariableString(variableCtrlPtr->dataPtr, variableCtrlPtr->selectedTypeBox, variableCtrlPtr->dataType);
					variableCtrlPtr->selectedTypeBox = -1;
					variableCtrlPtr->readyForInput = false;
					glfwSetMouseButtonCallback(window, variableCtrlPtr->mouseReturnPointer);
					glfwSetKeyCallback(window, variableCtrlPtr->keyReturnPointer);
					glfwSetCharCallback(window, NULL);
				}
			}
		}
	}

	void VariableControl::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		printf("key callback \n");
		if (variableCtrlPtr->selectedTypeBox < 0) {
			return;
		}
		if (variableCtrlPtr->readyForInput && (action != GLFW_RELEASE)) {
			switch (key) {
			case GLFW_KEY_BACKSPACE: {
				if (variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.length() > 0) {
					printf("bqackspace \n");
					//variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string = 
						//variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.substr(0, variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.length() - 1); 
						//maybe do a pointer to the selected typebox string, replace ifReadyForInput with if(selectedTypeBox == nullptr);
					if (variableCtrlPtr->stringSelectionIndex > 0) {
						printf("erasing at selection index - %d \n", variableCtrlPtr->stringSelectionIndex);
						variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.erase(variableCtrlPtr->stringSelectionIndex - 1, 1);
						variableCtrlPtr->stringSelectionIndex--;
					}
				}
				break;
			}
			case GLFW_KEY_ESCAPE: {
				printf("escape \n");
				variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string = GetVariableString(variableCtrlPtr->dataPtr, variableCtrlPtr->selectedTypeBox, variableCtrlPtr->dataType);
				variableCtrlPtr->readyForInput = false;
				variableCtrlPtr->selectedTypeBox = -1;
				glfwSetMouseButtonCallback(window, variableCtrlPtr->mouseReturnPointer);
				glfwSetKeyCallback(window, variableCtrlPtr->keyReturnPointer);
				break;
			}
			case GLFW_KEY_ENTER: {
				printf("enter \n");
				std::string& stringRef = variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string;
				stringRef.erase(variableCtrlPtr->stringSelectionIndex, 1);
				UIComp::SetVariableFromString(variableCtrlPtr->dataPtr, variableCtrlPtr->selectedTypeBox, stringRef, variableCtrlPtr->dataType);
				stringRef = GetVariableString(variableCtrlPtr->dataPtr, variableCtrlPtr->selectedTypeBox, variableCtrlPtr->dataType);
				//variableCtrlPtr->stringSelectionIndex = stringRef.length();
				//stringRef.push_back('|');
				variableCtrlPtr->readyForInput = false;
				variableCtrlPtr->selectedTypeBox = -1;
				glfwSetMouseButtonCallback(window, variableCtrlPtr->mouseReturnPointer);
				glfwSetKeyCallback(window, variableCtrlPtr->keyReturnPointer);
				printf("after enter \n");
				//glfwSetTypeCallback(window, NULL);
				break;
			}
			case GLFW_KEY_UP: {
				if (variableCtrlPtr->stringSelectionIndex > 0) {
					printf("erasing at selection index - %d \n", variableCtrlPtr->stringSelectionIndex);
					variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.erase(variableCtrlPtr->stringSelectionIndex, 1);
					variableCtrlPtr->stringSelectionIndex = 0;
					variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.insert(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.begin(), '|');
				}
				break;
			}
			case GLFW_KEY_DOWN: {
				std::string& stringRef = variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string;
				if (variableCtrlPtr->stringSelectionIndex < stringRef.length() - 1) {
					printf("erasing at selection index - %d \n", variableCtrlPtr->stringSelectionIndex);
					stringRef.erase(variableCtrlPtr->stringSelectionIndex, 1);
					variableCtrlPtr->stringSelectionIndex = static_cast<int>(stringRef.length());
					stringRef.push_back('|');
				}
				break;
			}
			case GLFW_KEY_LEFT: {
				if (variableCtrlPtr->stringSelectionIndex >= 1) {
					std::swap(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string[variableCtrlPtr->stringSelectionIndex - 1], variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string[variableCtrlPtr->stringSelectionIndex]);
					variableCtrlPtr->stringSelectionIndex--;
				}
				break;
			}
			case GLFW_KEY_RIGHT: {
				std::string& stringRef = variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string;
				if (variableCtrlPtr->stringSelectionIndex < stringRef.length() - 1) {
					std::swap(stringRef[variableCtrlPtr->stringSelectionIndex + 1], stringRef[variableCtrlPtr->stringSelectionIndex]);
					variableCtrlPtr->stringSelectionIndex++;
				}
				break;
			}
			}

			printf("pressed key - %d:%d \n", key, scancode);
			//textBoxPointer->textStruct.string += glfwGetKeyName(key, scancode);
		}
	}

	void VariableControl::typeCallback(GLFWwindow* window, unsigned int codepoint) {
		printf("key typed? - %ud \n", codepoint);

		if (variableCtrlPtr->selectedTypeBox < 0) {
			return;
		}
		if (variableCtrlPtr->readyForInput) {
			int lastLength = static_cast<int>(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.length());

			UIComp::InputType inputType = UIComp::InputType_none;
			if (variableCtrlPtr->dataType == UIComp::VT_float || variableCtrlPtr->dataType == UIComp::VT_double) {
				inputType = UIComp::InputType_float;
			}
			else {
				inputType = UIComp::InputType_numeric;
			}
			UIComp::TypeToString(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string, variableCtrlPtr->maxStringLength, codepoint, inputType, variableCtrlPtr->stringSelectionIndex);
			lastLength = static_cast<int>(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.length()) - lastLength;
			variableCtrlPtr->stringSelectionIndex += lastLength;
		}

	}
	//void VariableControl::render(NineUIPushConstantData& push) {
	//	for (int k = 0; k < typeBoxes.size(); k++) {
	//		if (isSelected(k)) {
	//			push.color = glm::vec3{ .6f, .5f, .4f };
	//		}
	//		else {
	//			push.color = glm::vec3{ .5f, .35f, .25f };
	//		}
	//		push.offset = glm::vec4(typeBoxes[k].transform.translation, 1.f, 1.f);
	//		push.scale = typeBoxes[k].transform.scale;
	//		Dimension2::PushAndDraw(push);
	//	}
	//}
	void VariableControl::Render(Array2DPushConstantData& push) {
		for (int k = 0; k < buttons.size(); k++) {
			buttons[k].first.Render(push);
			buttons[k].second.Render(push);
		}
	}

	// ~~~~~~~~~~~~~~~~~~~~~ CONTROL BOX (For controling other menus ~~~~~~~~~~~~~~~~~~~~~~~
	ControlBox* ControlBox::ctrlBoxPtr;
	//VariableTypeBox::VariableTypeBox(GLFWwindow* windowPtr, std::string labelString, float x, float y, float width, float screenWidth, float screenHeight)
	ControlBox::ControlBox(GLFWwindow* windowPtr, std::string labelString, float x, float y, float width, float screenWidth, float screenHeight)
		: width{ width }, windowPtr{ windowPtr }, screenWidth{ screenWidth }, screenHeight{ screenHeight } {
		//assert(variableCount > 0);
		//assert(data != nullptr && stepData != nullptr);


		//buttons.reserve(variableCount);

		//float verticalSpacing = 26.f * screenHeight / DEFAULT_HEIGHT;
		//float ratioWidth = width * screenWidth / DEFAULT_WIDTH;

		label = { labelString, x * screenWidth / DEFAULT_WIDTH, y * screenHeight / DEFAULT_WIDTH, TA_left, 1.5f };

	}

	void ControlBox::emplaceVariableControl(std::string dataLabel, void* dataPointer, UIComp::VariableType dataType, uint8_t dataCount, void* steps) {
		float startingOffset = 30.f * screenHeight / DEFAULT_HEIGHT;
		verticalSpacing = (30.f * screenHeight / DEFAULT_HEIGHT);
		for (int i = 0; i < variableControls.size(); i++) {
			startingOffset += variableControls[i].dataCount * verticalSpacing;
			startingOffset += 26.f * screenHeight / DEFAULT_HEIGHT;
		}
		printf("STARTING OFFSET : %.2f \n", startingOffset);
		ratioWidth = width * screenWidth / DEFAULT_WIDTH;

		//glm::ivec2 buttonScreen;
		//glm::vec2 buttonTranslation;
		//(GLFWwindow* windowPtr, float posX, float posY, float screenWidth, float screenHeight, TextStruct dataLabel, void* dataPointer, UIComp::VariableType dataType, uint8_t dataCount, void* steps)
		VariableControl& backRef = variableControls.emplace_back(windowPtr, label.x, label.y + startingOffset, width, screenWidth, screenHeight, dataLabel, dataPointer, dataType, dataCount, steps);

		//steps should be temporary data outside of this function, construct an array, pass it here, erase the original array
		dragBox.y = static_cast<int>(label.y);// -label.scale * 13.f;

		dragBox.w = variableControls.back().typeBoxes.back().clickBox.w;
		dragBox.x = static_cast<int>(label.x);
		dragBox.z = dragBox.x + static_cast<int>((ratioWidth + (60.f * screenWidth / DEFAULT_WIDTH)));
		UIComp::ConvertClickToTransform(dragBox, transform, screenWidth, screenHeight);
		dragBox.w = dragBox.y + static_cast<int>(label.scale * verticalSpacing);
	}

	void ControlBox::giveGLFWCallbacks(GLFWmousebuttonfun mouseReturnFunction, GLFWkeyfun keyReturnFunction) {
		printf("giving glfw callbacks \n");
		mouseReturnPointer = mouseReturnFunction;
		keyReturnPointer = keyReturnFunction;
		for (int i = 0; i < variableControls.size(); i++) {
			variableControls[i].giveGLFWCallbacks(mouseReturnFunction, keyReturnFunction);
		}
	}

	bool ControlBox::Clicked(double xpos, double ypos) {
		for (int i = 0; i < variableControls.size(); i++) {
			printf("clicked variable control [%d] \n", i);
			if (variableControls[i].Clicked(xpos, ypos)) {
				return true;
			}
		}

		if (UIComp::CheckClickBox(dragBox, xpos, ypos)) {
			printf("contorl box dragging \n");
			mouseDragging = true;
			double xpos;
			double ypos;
			glfwGetCursorPos(windowPtr, &xpos, &ypos);
			lastPos.first = static_cast<int>(xpos);
			lastPos.second = static_cast<int>(ypos);
			for (int i = 0; i < variableControls.size(); i++) {
				variableControls[i].setLastPos(lastPos);
			}
			ctrlBoxPtr = this;
			glfwSetCursorPosCallback(windowPtr, DragCallback);
			return true;
		}
		return false;
	}
	void ControlBox::DragCallback(GLFWwindow* window, double xpos, double ypos) {
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
			printf("releasing drag? \n");
			printf("returning callbacks in mousecallback??? \n");
			//glfwSetMouseButtonCallback(window, ctrlBoxPtr->mouseReturnPointer);
			//glfwSetKeyCallback(window, ctrlBoxPtr->keyReturnPointer);
			glfwSetCursorPosCallback(window, NULL);
			return;
		}

		int posXI = static_cast<int>(xpos);
		int posYI = static_cast<int>(ypos);
		for (int i = 0; i < ctrlBoxPtr->variableControls.size(); i++) {
			ctrlBoxPtr->variableControls[i].moveBox(posXI, posYI);
		}

		ctrlBoxPtr->label.x += posXI - ctrlBoxPtr->lastPos.first;
		ctrlBoxPtr->label.y += posYI - ctrlBoxPtr->lastPos.second;
		ctrlBoxPtr->lastPos.first = posXI;
		ctrlBoxPtr->lastPos.second = posYI;

		ctrlBoxPtr->dragBox.y = static_cast<int>(ctrlBoxPtr->label.y);// -label.scale * 13.f;

		ctrlBoxPtr->dragBox.w = ctrlBoxPtr->variableControls.back().typeBoxes.back().clickBox.w;
		ctrlBoxPtr->dragBox.x = static_cast<int>(ctrlBoxPtr->label.x);
		ctrlBoxPtr->dragBox.z = ctrlBoxPtr->dragBox.x + static_cast<int>(ctrlBoxPtr->ratioWidth + (60.f * ctrlBoxPtr->screenWidth / DEFAULT_WIDTH));
		UIComp::ConvertClickToTransform(ctrlBoxPtr->dragBox, ctrlBoxPtr->transform, ctrlBoxPtr->screenWidth, ctrlBoxPtr->screenHeight);
		ctrlBoxPtr->dragBox.w = ctrlBoxPtr->dragBox.y + static_cast<int>(ctrlBoxPtr->label.scale * ctrlBoxPtr->verticalSpacing);

		//then readjust the back Controller window
	}

	void ControlBox::ResizeWindow(glm::vec2 resizeRatio) {

		for (int i = 0; i < variableControls.size(); i++) {
			variableControls[i].ResizeWindow(resizeRatio);
		}
	}
	void ControlBox::Render(Array2DPushConstantData& push) {
		for (auto& object : variableControls) {
			object.Render(push);
		}
	}
	//void ControlBox::render(NineUIPushConstantData& push) {
	//	for (auto& object : variableControls) {
	//		object.render(push);
	//	}
	//	push.color = glm::vec3{ .3f, .25f, .15f };
	//	push.offset = glm::vec4(transform.translation, 1.f, 1.f);
	//	push.scale = transform.scale;
	//	Dimension2::PushAndDraw(push);
	//}

	// ~~~~~~~~~~~~~~~~~~~~~ MENU BAR ~~~~~~~~~~~~~~~~~~~~~~~~~~~

	MenuBar::MenuBar(float x, float y, float width, float height, float screenWidth, float screenHeight) {
		//height must be larger than the height of text, which is 26.6 * text.scale

		screenCoordinates.first = x;
		screenCoordinates.second = y;
		screenDimensions.first = width;
		screenDimensions.second = height;
		transform.translation.x = ((x + width / 2.f) - (screenWidth / 2.f)) / (screenWidth / 2.f);
		transform.translation.y = ((y + height / 2.f) - (screenHeight / 2.f)) / (screenHeight / 2.f);
		printf("trans y? : %.2f \n", transform.translation.y);

		transform.scale.x = width * 2.f / DEFAULT_WIDTH;
		transform.scale.y = height * 2.f / DEFAULT_HEIGHT;
	}
	void MenuBar::pushDropper(std::string dropperName, std::vector<std::string>& options, float screenWidth, float screenHeight) {
		printf("before emplace \n");
		DropBox& backRef = dropBoxes.emplace_back();
		printf("after empalce back \n");
		backRef.dropper.textStruct.string = dropperName;
		backRef.dropper.textStruct.x = screenCoordinates.first + 20.f * screenWidth / DEFAULT_WIDTH;


		backRef.dropper.textStruct.y = screenCoordinates.second + (screenCoordinates.second + ((screenDimensions.second - (39.f * screenHeight / DEFAULT_HEIGHT)) / 2.f));
		backRef.dropOptions.resize(options.size());
		for (int i = 0; i < options.size(); i++) {
			backRef.dropOptions[i].string = options[i];
		}
		//backRef.init(screenWidth, screenHeight);
	}
	void MenuBar::init() {
		for (int i = 0; i < dropBoxes.size(); i++) {
			if (i >= 1) {
				dropBoxes[i].dropper.textStruct.x = dropBoxes[i - 1].dropper.clickBox.z + (10.f * VK::Object->screenWidth / DEFAULT_WIDTH);
			}
			dropBoxes[i].dropper.textStruct.y = screenCoordinates.second + (screenDimensions.second / 2.f) - (10.f * VK::Object->screenHeight / DEFAULT_HEIGHT);
			dropBoxes[i].Init();
		}
	}

	int16_t MenuBar::Clicked(double xpos, double ypos) {
		int16_t clickReturn = -2;
		int16_t clickTracker = 0;
		for (int i = 0; i < dropBoxes.size(); i++) {
			clickReturn = dropBoxes[i].Clicked(xpos, ypos);
			if (clickReturn >= -1) {
				printf("clickReturn ? : %d \n", clickReturn);
				for (int j = i + 1; j < dropBoxes.size(); j++) {
					dropBoxes[j].currentlyDropped = false;
				}
				if (clickReturn == -1) {
					return -1;
				}
				return clickTracker + clickReturn;
			}
			clickTracker += static_cast<int16_t>(dropBoxes[i].clickBoxes.size());
			dropBoxes[i].currentlyDropped = false;
		}
		return -1;
	}
	void MenuBar::Render(Array2DPushConstantData& push, uint8_t drawID) {
		if (drawID == 0) {
			if (dropBoxes.size() > 0) { //drawing these here instead of tumblingg these with the earlier drop boxes because i dont want to draw the dropper box
				push.color = glm::vec3{ .5f, .35f, .25f };
				for (int j = 0; j < dropBoxes.size(); j++) {
					push.color = glm::vec3{ .5f, .35f, .25f };
					if (dropBoxes[j].currentlyDropped) {
						push.scaleOffset = glm::vec4(dropBoxes[j].dropBackground.scale, dropBoxes[j].dropBackground.translation);
						//push.offset = glm::vec4(dropBoxes[j].dropBackground.translation, 0.5f, 1.f);
						//push.scale = dropBoxes[j].dropBackground.scale;
						Dimension2::PushAndDraw(push);
						break; //i think only 1 can be dropped
					}
				}
			}
		}
		else {
			push.color = glm::vec3{ .86f, .5f, .5f };
			push.scaleOffset = glm::vec4(transform.scale, transform.translation);
			//push.offset = glm::vec4(transform.translation, 0.1f, 1.f);
			//push.scale = transform.scale;
			Dimension2::PushAndDraw(push);
		}
	}
}