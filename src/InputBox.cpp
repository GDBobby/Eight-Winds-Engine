#include "EWEngine/ShaderGraph/InputBox.h"

namespace EWE {
	namespace Shader {
		void InputOutputVariable::writeToString(std::string& outString, bool inTrueOutFalse, uint16_t position) {
			outString += "layout(location = " + std::to_string(position) + ')';
			if (inTrueOutFalse) {
				outString += "in ";
			}
			else {
				outString += "out ";
			}

			if (typeID >= Type_Struct) {
				outString += ShaderStructureManager::getStructureName(typeID);
				outString += ' ' + variableName + ';';
			}
			else {
				outString += VariableTypeString[typeID] + ' ' + variableName + ';';
			}
		}
		InputOutputData::InputOutputData(float xPos, float yPos, float screenWidth, float screenHeight)
			: variable{ 0, "newVariable" },
			variableCombo{ TextStruct{VariableTypeString[GLSL_Type_float], xPos, yPos, TA_left, 1.f}, screenWidth, screenHeight },
			name{ variable.variableName, xPos + 100.f * screenWidth / DEFAULT_WIDTH, yPos, TA_left, 1.f, screenWidth, screenHeight },
			removeThis{ {xPos,yPos}, screenWidth + 200.f * screenWidth / DEFAULT_WIDTH, screenHeight }
		{
			populateCombo(screenWidth, screenHeight);
			variableCombo.setSelection(0);
		}
		InputOutputData::InputOutputData(VariableType vType, std::string variableName, float xPos, float yPos, float screenWidth, float screenHeight)
			: variable{ vType, variableName },
			variableCombo{ TextStruct{VariableTypeString[GLSL_Type_float], xPos, yPos, TA_left, 1.f}, screenWidth, screenHeight },
			name{ variable.variableName, xPos + 100.f * screenWidth / DEFAULT_WIDTH, yPos, TA_left, 1.f, screenWidth, screenHeight },
			removeThis{ {xPos,yPos}, screenWidth, screenHeight }
		{
			populateCombo(screenWidth, screenHeight);
		}

		void InputOutputData::populateCombo(float screenWidth, float screenHeight) {
			auto const& variableNames = ShaderStructureManager::getAllVariableNames();
			for (auto name : variableNames) {
				variableCombo.pushOption(name, screenWidth, screenHeight);
			}
		}



		InputBox* InputBox::inputBoxPtr;

		bool InputBox::Clicked(double xpos, double ypos) {
			for (int j = 0; j < variables.size(); j++) {
				if (variables[j].name.Clicked(xpos, ypos)) {
					selectedTypeBox = j;
					inputBoxPtr = this;
					readyForInput = true;
					stringSelectionIndex = static_cast<int>(variables[j].name.textStruct.string.length());
					variables[j].name.textStruct.string += '|';

					readyForInput = true;
					glfwSetCharCallback(windowPtr, typeCallback);
					glfwSetKeyCallback(windowPtr, KeyCallback);
					glfwSetMouseButtonCallback(windowPtr, MouseCallback);

					return true;
				}
			}

			if (addVariable.Clicked(xpos, ypos)) {
				printf("adding variable to InputBox \n");
				InputOutputVariable variable = { 0, "newVariable" };
				//(float xPos, float yPos, float screenWidth, float screenHeight)
				variables.emplace_back(name.x, name.y + ((48.f + 36.f * variables.size()) * screenHeight / DEFAULT_HEIGHT), screenWidth, screenHeight);
				return false;
			}

			return false;
		}

		InputBox::InputBox(GLFWwindow* windpwPtr, bool inputTrueOutputFalse, float xPos, float yPos, float screenWidth, float screenHeight)
			: inputTrueOutputFalse{ inputTrueOutputFalse },
			windowPtr{ windowPtr },
			screenWidth{ screenWidth },
			screenHeight{ screenHeight },
			name{ "", xPos, yPos, TA_left, 1.5f },
			addVariable{ {xPos, yPos}, screenWidth, screenHeight }
		{
			if (inputTrueOutputFalse) {
				name.string = "Input";
			}
			else {
				name.string = "Output";
			}
		}

		void InputBox::MouseCallback(GLFWwindow* window, int button, int action, int mods) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				if (action == GLFW_PRESS) {
					if (inputBoxPtr->selectedTypeBox < 0) {
						return;
					}
					double xpos = 0;
					double ypos = 0;
					glfwGetCursorPos(window, &xpos, &ypos);
					bool clickedABox = false;
					for (int i = 0; i < inputBoxPtr->variables.size(); i++) {
						if (UIComp::checkClickBox(inputBoxPtr->variables[i].name.clickBox, xpos, ypos)) {
							if (i != inputBoxPtr->selectedTypeBox) {
								inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string.erase(inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string.find_first_of('|'), 1);
								printf("remove the | \n");
								inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].variable.variableName = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
								inputBoxPtr->selectedTypeBox = i;
							}
							printf("clicked a type box? \n");
							inputBoxPtr->stringSelectionIndex = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.getSelectionIndex(xpos, inputBoxPtr->screenWidth);
							std::string& stringRef = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
							uint16_t indexPos = static_cast<uint16_t>(stringRef.find_first_of('|'));
							if ((indexPos != inputBoxPtr->stringSelectionIndex) && (indexPos != (inputBoxPtr->stringSelectionIndex + 1))) {
								if (indexPos != stringRef.npos) {
									stringRef.erase(indexPos, 1);
									inputBoxPtr->stringSelectionIndex -= inputBoxPtr->stringSelectionIndex > indexPos;
								}
								stringRef.insert(stringRef.begin() + inputBoxPtr->stringSelectionIndex, '|');
							}
							printf("string selection index? : %d \n", inputBoxPtr->stringSelectionIndex);
							inputBoxPtr->readyForInput = true;
							clickedABox = true;
							break;
						}
					}
					if (!clickedABox) {
						inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].variable.variableName = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
						inputBoxPtr->selectedTypeBox = -1;
						inputBoxPtr->readyForInput = false;
						glfwSetMouseButtonCallback(window, inputBoxPtr->mouseReturnPointer);
						glfwSetKeyCallback(window, inputBoxPtr->keyReturnPointer);
						glfwSetCharCallback(window, NULL);
					}
				}
			}
		}

		void InputBox::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
			printf("key callback \n");
			if (inputBoxPtr->selectedTypeBox < 0) {
				return;
			}
			if (inputBoxPtr->readyForInput && (action != GLFW_RELEASE)) {
				switch (key) {
				case GLFW_KEY_BACKSPACE: {
					if (inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string.length() > 0) {
						printf("bqackspace \n");
						//inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string = 
							//inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string.substr(0, inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string.length() - 1); 
							//maybe do a pointer to the selected typebox string, replace ifReadyForInput with if(selectedTypeBox == nullptr);
						if (inputBoxPtr->stringSelectionIndex > 0) {
							printf("erasing at selection index - %d \n", inputBoxPtr->stringSelectionIndex);
							inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string.erase(inputBoxPtr->stringSelectionIndex - 1, 1);
							inputBoxPtr->stringSelectionIndex--;
						}
					}
					break;
				}
				case GLFW_KEY_ESCAPE: {
					printf("escape \n");
					inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].variable.variableName = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
					inputBoxPtr->readyForInput = false;
					inputBoxPtr->selectedTypeBox = -1;
					glfwSetMouseButtonCallback(window, inputBoxPtr->mouseReturnPointer);
					glfwSetKeyCallback(window, inputBoxPtr->keyReturnPointer);
					break;
				}
				case GLFW_KEY_ENTER: {
					printf("enter \n");
					std::string& stringRef = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
					stringRef.erase(inputBoxPtr->stringSelectionIndex, 1);
					inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].variable.variableName = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
					//inputBoxPtr->stringSelectionIndex = stringRef.length();
					//stringRef.push_back('|');
					inputBoxPtr->readyForInput = false;
					inputBoxPtr->selectedTypeBox = -1;
					glfwSetMouseButtonCallback(window, inputBoxPtr->mouseReturnPointer);
					glfwSetKeyCallback(window, inputBoxPtr->keyReturnPointer);
					printf("after enter \n");
					//glfwSetTypeCallback(window, NULL);
					break;
				}
				case GLFW_KEY_UP: {
					if (inputBoxPtr->stringSelectionIndex > 0) {
						printf("erasing at selection index - %d \n", inputBoxPtr->stringSelectionIndex);
						inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string.erase(inputBoxPtr->stringSelectionIndex, 1);
						inputBoxPtr->stringSelectionIndex = 0;
						inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string.insert(inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string.begin(), '|');
					}
					break;
				}
				case GLFW_KEY_DOWN: {
					std::string& stringRef = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
					if (inputBoxPtr->stringSelectionIndex < stringRef.length() - 1) {
						printf("erasing at selection index - %d \n", inputBoxPtr->stringSelectionIndex);
						stringRef.erase(inputBoxPtr->stringSelectionIndex, 1);
						inputBoxPtr->stringSelectionIndex = static_cast<int>(stringRef.length());
						stringRef.push_back('|');
					}
					break;
				}
				case GLFW_KEY_LEFT: {
					if (inputBoxPtr->stringSelectionIndex >= 1) {
						std::swap(inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string[inputBoxPtr->stringSelectionIndex - 1], inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string[inputBoxPtr->stringSelectionIndex]);
						inputBoxPtr->stringSelectionIndex--;
					}
					break;
				}
				case GLFW_KEY_RIGHT: {
					std::string& stringRef = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
					if (inputBoxPtr->stringSelectionIndex < stringRef.length() - 1) {
						std::swap(stringRef[inputBoxPtr->stringSelectionIndex + 1], stringRef[inputBoxPtr->stringSelectionIndex]);
						inputBoxPtr->stringSelectionIndex++;
					}
					break;
				}
				}

				printf("pressed key - %d:%d \n", key, scancode);
				//textBoxPointer->textStruct.string += glfwGetKeyName(key, scancode);
			}
		}

		void InputBox::typeCallback(GLFWwindow* window, unsigned int codepoint) {
			printf("key typed? - %ud \n", codepoint);

			if (inputBoxPtr->selectedTypeBox < 0) {
				return;
			}
			if (inputBoxPtr->readyForInput) {
				int lastLength = static_cast<int>(inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string.length());
				UIComp::TypeToString(inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string, inputBoxPtr->maxStringLength, codepoint, UIComp::InputType_alphanumeric, inputBoxPtr->stringSelectionIndex);
				lastLength = static_cast<int>(inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string.length()) - lastLength;
				inputBoxPtr->stringSelectionIndex += lastLength;
			}

		}
	}
}
