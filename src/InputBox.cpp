#include "EWEngine/ShaderGraph/InputBox.h"

namespace EWE {
	namespace Shader {
		void InputOutputVariable::writeToString(std::string& outString, bool inTrueOutFalse, uint16_t position) {
			outString += "layout(location=" + std::to_string(position) + ')';
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
			removeThis{ {0.f,0.f}, screenWidth, screenHeight }
		{
			glm::vec2 screen{ xPos + 400.f * screenWidth / DEFAULT_WIDTH, yPos + 18.f * screenHeight / DEFAULT_HEIGHT };
			UIComp::convertScreenTo2D(screen, removeThis.transform.translation, screenWidth, screenHeight);
			removeThis.resizeWindow(screenWidth, screenHeight);
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

		void InputOutputData::move(float xDiff, float yDiff, float screenWidth, float screenHeight) {
			name.textStruct.x += xDiff;
			name.textStruct.y += yDiff;
			name.clickBox.x += xDiff;
			name.clickBox.z += xDiff;
			name.clickBox.y += yDiff;
			name.clickBox.w += yDiff;
			UIComp::convertClickToTransform(name.clickBox, name.transform, screenWidth, screenHeight);

			removeThis.clickBox.x += xDiff;
			removeThis.clickBox.z += xDiff;
			removeThis.clickBox.y += yDiff;
			removeThis.clickBox.w += yDiff;
			UIComp::convertClickToTransform(removeThis.clickBox, removeThis.transform, screenWidth, screenHeight);

			variableCombo.move(xDiff, yDiff, screenWidth, screenHeight);
		}






		InputBox* InputBox::inputBoxPtr;
		GLFWmousebuttonfun InputBox::mouseReturnPointer;
		GLFWkeyfun InputBox::keyReturnPointer;

		bool InputBox::Clicked(double xpos, double ypos) {
			for (int j = 0; j < variables.size(); j++) {
				if (variables[j].name.Clicked(xpos, ypos)) {
					selectedTypeBox = j;
					inputBoxPtr = this;
					readyForInput = true;
					stringSelectionIndex = static_cast<int>(variables[j].name.textStruct.string.length());
					variables[j].name.textStruct.string += '|';

					readyForInput = true;
					printf("setting callbacks 1 \n");
					glfwSetCharCallback(windowPtr, InputBox::typeCallback);
					printf("setting callbacks 2 \n");
					glfwSetKeyCallback(windowPtr, InputBox::KeyCallback);
					printf("setting callbacks 3 \n");
					glfwSetMouseButtonCallback(windowPtr, InputBox::MouseCallback);
					printf("after setting callbacks \n");

					return true;
				}
				if (variables[j].variableCombo.Clicked(xpos, ypos)) {
					if (variables[j].variableCombo.currentlySelected >= 0) {
						variables[j].variable.typeID = variables[j].variableCombo.currentlySelected;
					}
					else {
						printf("why less than 1 selected? \n");
					}
					printf("variable comboclicked : %d \n", j);
					return false;
				}
				if(variables[j].removeThis.Clicked(xpos, ypos)) {
					variables.erase(variables.begin() + j);
					float heightDiff = -36.f * screenHeight / DEFAULT_HEIGHT;
					for (int k = j; k < variables.size(); k++) {
						variables[k].move(0.f, heightDiff, screenWidth, screenHeight);
					}
					addVariable.clickBox.y += heightDiff;
					addVariable.clickBox.w += heightDiff;
					UIComp::convertClickToTransform(addVariable.clickBox, addVariable.transform, screenWidth, screenHeight);
					backgroundScreen.w -= 36.f;
					UIComp::convertClickToTransform(backgroundScreen, background, screenWidth, screenHeight);

					return false;
				}
			}
		
			if (addVariable.Clicked(xpos, ypos)) {
				printf("adding variable to InputBox \n");
				InputOutputVariable variable = { 0, "newVariable" };
				//(float xPos, float yPos, float screenWidth, float screenHeight)
				variables.emplace_back(name.x, name.y + ((48.f + 36.f * variables.size()) * screenHeight / DEFAULT_HEIGHT), screenWidth, screenHeight);
				addVariable.transform.translation.y += (72.f / DEFAULT_HEIGHT);
				addVariable.clickBox.y += 36.f;
				addVariable.clickBox.w += 36.f;
				backgroundScreen.w += 36.f;
				UIComp::convertClickToTransform(backgroundScreen, background, screenWidth, screenHeight);
				return false;
			}

			return false;
		}

		InputBox::InputBox(GLFWwindow* windowPtr, bool inputTrueOutputFalse, float xPos, float yPos, float screenWidth, float screenHeight)
			: inputTrueOutputFalse{ inputTrueOutputFalse },
			windowPtr{ windowPtr },
			screenWidth{ screenWidth },
			screenHeight{ screenHeight },
			name{ "", xPos, yPos, TA_left, 1.5f },
			addVariable{ glm::vec2{0.f}, screenWidth, screenHeight }
		{
			glm::ivec2 screen = { static_cast<int>(xPos + 400.f * screenWidth / DEFAULT_WIDTH), static_cast<int>(yPos + 108.f * screenHeight / DEFAULT_HEIGHT) };
			UIComp::convertScreenTo2D(screen, addVariable.transform.translation, screenWidth, screenHeight);
			addVariable.resizeWindow(screenWidth, screenHeight);
			addVariable.transform.scale.y *= -1.f;
			if (inputTrueOutputFalse) {
				name.string = "Input";
			}
			else {
				name.string = "Output";
			}
			backgroundScreen.x = static_cast<int>(xPos - 20.f * screenWidth / DEFAULT_WIDTH);
			backgroundScreen.z = static_cast<int>(xPos + 420.f * screenWidth / DEFAULT_WIDTH);
			backgroundScreen.y = static_cast<int>(yPos - 20.f * screenHeight / DEFAULT_HEIGHT);
			backgroundScreen.w = static_cast<int>(yPos + 128.f * screenHeight / DEFAULT_HEIGHT);
			UIComp::convertClickToTransform(backgroundScreen, background, screenWidth, screenHeight);
		}

		void InputBox::MouseCallback(GLFWwindow* window, int button, int action, int mods) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				if (action == GLFW_PRESS) {
					printf("selectedTypeBox : %d \n", inputBoxPtr->selectedTypeBox);
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
								printf("swpaping textboxes :% d \n", inputBoxPtr->selectedTypeBox);
								std::string& stringRef = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
								size_t indexPos = stringRef.find_first_of('|');
								if (indexPos != stringRef.npos) {
									stringRef.erase(indexPos, 1);
									inputBoxPtr->stringSelectionIndex -= inputBoxPtr->stringSelectionIndex > indexPos;
								}
								inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].variable.variableName = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
								inputBoxPtr->selectedTypeBox = i;
								printf("textbox swpaped : %d:%d \n", i, inputBoxPtr->selectedTypeBox);
							}
							printf("clicked a type box? \n");
							inputBoxPtr->stringSelectionIndex = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.getSelectionIndex(xpos, inputBoxPtr->screenWidth);
							printf("after selection index : %d \n", inputBoxPtr->stringSelectionIndex);
							std::string& stringRef = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
							size_t indexPos = stringRef.find_first_of('|');
							printf("stringRef, indexPos - %s:%lu:%lu\n", stringRef.c_str(), indexPos, stringRef.npos);
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
						printf("didnt click a box, going to return callbacks \n");
						std::string& stringRef = inputBoxPtr->variables[inputBoxPtr->selectedTypeBox].name.textStruct.string;
						size_t indexPos = stringRef.find_first_of('|');
						if (indexPos != stringRef.npos) {
							stringRef.erase(indexPos, 1);
							inputBoxPtr->stringSelectionIndex -= inputBoxPtr->stringSelectionIndex > indexPos;
						}

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
		//						   GLFWwindow* window, int key, int scancode, int action, int mods
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

		void InputBox::render(Simple2DPushConstantData& push, uint8_t drawID) {

			if (drawID == 0) {
				//push.color = glm::vec3{ .125f, .0875f, .0625f };
				push.color.r = .05f;
				push.color.g = .05f;
				push.color.b = .05f;
				push.scaleOffset = glm::vec4(background.scale, background.translation);
				Dimension2::pushAndDraw(push);
			}
			else {
				push.color = glm::vec3{ 1.f, 1.f, 1.f };

				for (int i = 0; i < variables.size(); i++) {
					variables[i].removeThis.render(push);
				}
				//printf("rendering add variable - %.2f:%.2f \n", addVariable.transform.translation.x, addVariable.transform.translation.y);
				addVariable.render(push);
			}
		}

		void InputBox::render(NineUIPushConstantData& push) {

			Dimension2::bindTexture9(MenuModule::textureIDs[MT_NineUI]);
			push.color = glm::vec3{ .5f, .35f, .25f };

			if (variables.size() > 0) {
				push.color = glm::vec3{ .5f, .35f, .25f };
				for (int i = 0; i < variables.size(); i++) {
					push.color = glm::vec3{ .5f, .35f, .25f };

					push.offset = glm::vec4(variables[i].variableCombo.activeOption.transform.translation, 1.f, 1.f);
					//need color array
					push.scale = variables[i].variableCombo.activeOption.transform.scale;
					Dimension2::pushAndDraw(push);

					variables[i].variableCombo.render(push);

					variables[i].name.render(push);

				}
			}

		}
		void InputBox::drawText() {
			TextOverlay::staticAddText(name);
			for (auto& variable : variables) {
				TextOverlay::staticAddText(variable.name.textStruct);
				TextOverlay::staticAddText(variable.variableCombo.activeOption.textStruct);
				if (variable.variableCombo.currentlyDropped) {
					for (auto cOption : variable.variableCombo.comboOptions) {
						TextOverlay::staticAddText(cOption.textStruct);
					}
				}
			}
		}
	}
}
