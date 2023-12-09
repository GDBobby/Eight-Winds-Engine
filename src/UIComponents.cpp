#include "gui/UIComponents.h"
namespace EWE {

#define FLOAT_PTR_PRECISION 4 //desired precision + 1


	namespace UIComp {
		void convertTransformToClickBox(Transform2dComponent& transform, glm::ivec4& clickBox, float screenWidth, float screenHeight) {
			clickBox.x = static_cast<int>(screenWidth + ((screenWidth / 2) * (transform.translation.x - 1.f)) - (screenWidth * transform.scale.x / 4));
			clickBox.y = static_cast<int>(screenHeight + ((screenHeight / 2) * (transform.translation.y - 1.f)) - (screenHeight * transform.scale.y / 4));
			clickBox.z = static_cast<int>(screenWidth + ((screenWidth / 2) * (transform.translation.x - 1.f)) + (screenWidth * transform.scale.x / 4));
			clickBox.w = static_cast<int>(screenHeight + ((screenHeight / 2) * (transform.translation.y - 1.f)) + (screenHeight * transform.scale.y / 4));
		}
		bool checkClickBox(glm::ivec4& clickBox, double mouseX, double mouseY) {
			return (mouseX > clickBox.x) && (mouseX < clickBox.z) && (mouseY > clickBox.y) && (mouseY < clickBox.w);
		}
		void printClickBox(glm::ivec4& clickBox) {
			printf("print click box - hori(%d:%d), vert(%d:%d) \n", clickBox.x, clickBox.z, clickBox.y, clickBox.w);
		}

		void TextToTransform(Transform2dComponent& transform, TextStruct& textStruct, glm::ivec4& clickBox, float screenWidth, float screenHeight) {
			//std::cout << "bounds of tempPRinter : " << tempPrinter.x << ":" << tempPrinter.y << ":" << tempPrinter.z << ":" << tempPrinter.w << std::endl;
			transform.scale.x = textStruct.getWidth(screenWidth) * screenWidth / DEFAULT_WIDTH;
			if (transform.scale.x < 0.0f) {
				printf("text struct width returned less than 0, string : %s \n", textStruct.string.c_str());
				printf("\t x, y, screenW, screenH %.1f, %.1f, %.1f, %.1f \n", textStruct.x, textStruct.y, screenWidth, screenHeight);
			}

			transform.translation.x = (textStruct.x - screenWidth / 2) / (screenWidth / 2);
			transform.scale.y = textStruct.scale / 26.66f;
			transform.translation.y = (transform.scale.y / 2.f) - 1.f + 2.f * (textStruct.y - textStruct.scale) / screenHeight;
			

			if (textStruct.align == TA_center) {
			}
			if (textStruct.align == TA_left) {
				transform.translation.x += transform.scale.x / 2;
			}
			if (textStruct.align == TA_right) {
				transform.translation.x -= transform.scale.x / 2;
			}
			//clickBox.y = textStruct.y - textStruct.scale;
			//clickBox.w = clickBox.y + (textStruct.scale * screenHeight / 40.f); //scale /20 and screenHeight / 2 combined to /40.f

			convertTransformToClickBox(transform, clickBox, screenWidth, screenHeight);

		}
		void convertScreenTo2D(glm::ivec2 const screen, glm::vec2& coord2D, float screenWidth, float screenHeight) {
			coord2D.x = (screen.x - (screenWidth / 2.f)) / (screenWidth / 2.f);
			coord2D.y = (screen.y - (screenHeight / 2.f)) / (screenHeight / 2.f);
		}
		void convertClickToTransform(glm::ivec4& clickBox, Transform2dComponent& transform, float screenWidth, float screenHeight) {

			transform.translation.x = (((clickBox.x + clickBox.z) / 2) - (screenWidth / 2.f)) / (screenWidth / 2.f);
			transform.translation.y = (((clickBox.y + clickBox.w) / 2) - (screenHeight / 2.f)) / (screenHeight / 2.f);

			transform.scale.x = (clickBox.z - clickBox.x) * 2.f / screenWidth;
			transform.scale.y = (clickBox.w - clickBox.y) * 2.f / screenHeight;
		}

		// ~~~~~~~~~ VARIABLE ABSTRACTION ~~~~~~~~~~~~~~~~~~~~~~
		void TypeToString(std::string& outputString, uint16_t maxStringLength, int codepoint, InputType inputType, uint8_t stringSelectionIndex) {
			if (outputString.length() < maxStringLength) {
				switch (inputType) {
				case InputType_none: {return; }
				case InputType_alphanumeric: {
					if ((codepoint >= 48) && (codepoint <= 57)) {
						outputString.insert(outputString.begin() + stringSelectionIndex, (char)codepoint);
					}
					return;
				}
				case InputType_alpha: { //fallthrough to lower as well
					if (codepoint >= 65 && codepoint <= 90) { 
						outputString += codepoint; return; 
					}
					//[[fallthrough]];
				}
				case InputType_alphaLower: {
					if (codepoint >= 97 && codepoint <= 122) {
						outputString.insert(outputString.begin() + stringSelectionIndex, (char)codepoint);
					}
					return;
				}
				case InputType_numeric: {
					if ((codepoint >= 48) && (codepoint <= 57) || (codepoint == 45)) {
						if (codepoint == 45) {
							if (outputString.find_first_of("-") != outputString.npos) {
								outputString = outputString.erase(0, 1);
							}
							else {
								outputString.insert(0, "-");
							}
							return;
						}
						outputString.insert(outputString.begin() + stringSelectionIndex, (char)codepoint);
					}
					return;
				}
				case InputType_float: {
					if (((codepoint >= 48) && (codepoint <= 57)) || (codepoint == 46) || (codepoint == 45)) {
						if (!((codepoint == 46) && (outputString.find_first_of(".") != outputString.npos))) {
							if (codepoint == 45) {
								if (outputString.find_first_of("-") != outputString.npos) {
									outputString = outputString.erase(0, 1);
								}
								else {
									outputString.insert(0, "-");
								}
								return;
							}
							outputString.insert(outputString.begin() + stringSelectionIndex, (char)codepoint);
						}
					}
					return;
				}
				default: {
					//?
					break;
				}
				}
			}
		}
		size_t getVariableSize(VariableType vType) {
			switch (vType) {
				case VT_int64: {return sizeof(int64_t); }
				case VT_int32: {return sizeof(int32_t); }
				case VT_int16: {return sizeof(int16_t); }
				case VT_int8: {return sizeof(int8_t); }
				case VT_float: {return sizeof(float); }
				case VT_double: {return sizeof(double); }
				default: {
					throw std::runtime_error("invalid type in getvariablesize");
				}
			}
		}
		std::string getVariableString(void* data, int offset, VariableType vType) {

			switch (vType) {
			case VT_int64: {
				return std::to_string(*((int64_t*)data + offset));
			}
			case VT_int32: {
				return std::to_string(*((int32_t*)data + offset));
			}
			case VT_int16: {
				return std::to_string(*((int16_t*)data + offset));
			}
			case VT_int8: {
				return std::to_string(*((int8_t*)data + offset));
			}
			case VT_float: {
				std::string bufferString = std::to_string(*((float*)data + offset));
				auto pos = bufferString.find('.');
				if (pos != std::string::npos && (pos + FLOAT_PTR_PRECISION) < bufferString.length()) {
					//printf("what is this value? : %.5f - %s \n", *((float*)data + offset), std::to_string(*((float*)data + offset)).c_str());

					return bufferString.substr(0, pos + FLOAT_PTR_PRECISION);
				}
				else {
					printf("error detected??? y tho \n");
					*((float*)((int16_t*)data + offset)) = 0.f;
					return "0.0";
				}
			}
			case VT_double: {
				std::string bufferString = std::to_string(*((double*)data + offset));
				auto pos = bufferString.find('.');
				if (pos != std::string::npos && (pos + FLOAT_PTR_PRECISION) < bufferString.length()) {
					printf("what is this value? : %.5f - %s \n", *((double*)data + offset), std::to_string(*((double*)data + offset)).c_str());

					return bufferString.substr(0, pos + FLOAT_PTR_PRECISION);
				}
				else {
					printf("error detected??? y tho \n");
					*((double*)((int16_t*)data + offset)) = 0.f;
					return "0.0";
				}
			}
			default: {
				throw std::runtime_error("invalid type in getvariablesize");
			}
			}
		}

		void SetVariableFromString(void* data, int offset, std::string& inString, VariableType vType) {
			if (inString.length() == 0) {
				inString = getVariableString(data, offset, vType);
				return;
			}

			switch (vType) {
			case VT_int64: {

				if (inString.length() >= 19) {
					//doesnt care if  the length is 19, but the value is below int64_max
					//if it has a negative sign, the max length is 18, and it doesnt care if below int64_t
					//are those values even relevant?
					inString = std::to_string(*((int64_t*)data + offset));
					return;
				}
				int64_t buffer = std::stoll(inString);

				buffer = (INT64_MAX <= buffer) * INT64_MAX + (INT64_MAX > buffer) * buffer;
				buffer = (INT64_MIN > buffer) * INT64_MIN + (INT64_MIN <= buffer) * buffer;
				*((int64_t*)data + offset) = std::stoll(inString);
				break;
			}
			case VT_int32: {
				//instill a max string input length
				0;
				if (inString.length() >= 11) {
					//doesnt care if  the length is 19, but the value is below int64_max
					//if it has a negative sign, the max length is 18, and it doesnt care if below int64_t
					//are those values even relevant?
					inString = std::to_string(*((int32_t*)data + offset));
					return;
				}
				int64_t buffer = std::stoll(inString);
				buffer = (INT32_MAX <= buffer) * INT32_MAX + (INT32_MAX > buffer) * buffer;
				buffer = (INT32_MIN > buffer) * INT32_MIN + (INT32_MIN <= buffer) * buffer;
				*((int32_t*)data + offset) = static_cast<int32_t>(buffer);
				break;
			}
			case VT_int16: {
				if (inString.length() >= 6) {
					inString = std::to_string(*((int16_t*)data + offset));
					return;
				}
				int32_t buffer = std::stoi(inString);
				buffer = (INT16_MAX <= buffer) * INT16_MAX + (INT16_MAX > buffer) * buffer;
				buffer = (INT16_MIN > buffer) * INT16_MIN + (INT16_MIN <= buffer) * buffer;

				*((int16_t*)data + offset) = buffer;
				break;
			}
			case VT_int8: {
				if (inString.length() >= 4) {
					inString = std::to_string(*((int8_t*)data + offset));
					return;
				}
				*((int8_t*)data + offset) = std::stoi(inString);
				break;
			}
			case VT_float: {
				if (inString.length() > 20) {
					inString = std::to_string(*((float*)data + offset));
					return;
				}

				*((float*)data + offset) = std::stof(inString);
				break;

			}
			case VT_double: {
				if (inString.length() > 20) {
					inString = std::to_string(*((double*)data + offset));
					return;
				}
				*((double*)data + offset) = std::stod(inString);
			}
			default: {
				throw std::runtime_error("invalid type in getvariablesize");
			}
			}
		}

		void addVariables(void* firstData, int32_t firstOffset, void* secondData, int32_t secondOffset, VariableType vType) {
			switch (vType) {
			case VT_int64: {
				(*(((int64_t*)firstData + firstOffset))) += (*((int64_t*)secondData + secondOffset));
				break;
			}
			case VT_int32: {
				(*(((int32_t*)firstData + firstOffset))) += (*((int32_t*)secondData + secondOffset));
				break;
			}
			case VT_int16: {
				(*(((int16_t*)firstData + firstOffset))) += (*((int16_t*)secondData + secondOffset));
				break;
			}
			case VT_int8: {
				(*(((int8_t*)firstData + firstOffset))) += (*((int8_t*)secondData + secondOffset));
				break;
			}
			case VT_float: {
				(*(((float*)firstData + firstOffset))) += (*((float*)secondData + secondOffset));
				break;
			}
			case VT_double: {
				(*(((double*)firstData + firstOffset))) += (*((double*)secondData + secondOffset));
				break;
			}
			default: {
				throw std::runtime_error("invalid type in addvariablesize");
				break; //this break is here just to silence warnings
			}
			}
		}

		void subtractVariables(void* firstData, int32_t firstOffset, void* secondData, int32_t secondOffset, VariableType vType) {
			switch (vType) {
			case VT_int64: {
				(*(((int64_t*)firstData + firstOffset))) -= (*((int64_t*)secondData + secondOffset));
				break;
			}
			case VT_int32: {
				(*(((int32_t*)firstData + firstOffset))) -= (*((int32_t*)secondData + secondOffset));
				break;
			}
			case VT_int16: {
				(*(((int16_t*)firstData + firstOffset))) -= (*((int16_t*)secondData + secondOffset));
				break;
			}
			case VT_int8: {
				(*(((int8_t*)firstData + firstOffset))) -= (*((int8_t*)secondData + secondOffset));
				break;
			}
			case VT_float: {
				(*(((float*)firstData + firstOffset))) -= (*((float*)secondData + secondOffset));
				break;
			}
			case VT_double: {
				(*(((double*)firstData + firstOffset))) -= (*((double*)secondData + secondOffset));
				break;
			}
			default: {
				throw std::runtime_error("invalid type in subtractvariablesize");
				break; //this break is here just to silence warnings
			}
			}
		}

	}
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
			dropOptions[i].y = dropper.textStruct.y + 9.5f + (i + 1) * (19.f * scale) * screenHeight / DEFAULT_HEIGHT;

			float tempWidth = dropOptions[i].getWidth(screenWidth);
			if (tempWidth > biggestWidth) { biggestWidth = tempWidth; }

			clickBoxes[i].y = static_cast<int>(dropOptions[i].y);
			clickBoxes[i].w = static_cast<int>(dropOptions[i].y + (19.f * scale) * screenHeight / DEFAULT_HEIGHT);
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

		dropBackground.scale = glm::vec2(biggestWidth * screenWidth / DEFAULT_WIDTH, scale * clickBoxes.size() / 26.66f);
	}

	int8_t DropBox::Clicked(double xpos, double ypos) {
		if (!currentlyDropped) {
			if (UIComp::checkClickBox(dropper.clickBox, xpos, ypos)) {
				UIComp::printClickBox(dropper.clickBox);
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


	// ~~~~~~~~~~~~~~~~~~~~~ VARIABLE CONTROL ~~~~~~~~~~~~~~~~~~~~
	VariableControl* VariableControl::variableCtrlPtr;
	
	VariableControl::VariableControl(GLFWwindow* windowPtr, float posX, float posY, float width, float screenWidth, float screenHeight, std::string dataLabelString, void* dataPointer, UIComp::VariableType dataType, uint8_t dataCount, void* steps)
		: dataPtr{ dataPointer }, dataType{ dataType }, dataCount{ dataCount }, screenWidth{ screenWidth }, screenHeight{ screenHeight }, windowPtr{ windowPtr }, width{ width }
	{
		assert(dataCount > 0);
		size_t variableSize = UIComp::getVariableSize(dataType);
		dataLabel = TextStruct{ dataLabelString, posX, posY, TA_left, 1.f };
		//assert(sizeof(T) == variableSize);

		this->steps = malloc(variableSize * 3);

		memcpy(this->steps, steps, variableSize * 3);

		float ratioWidth = width * screenWidth / DEFAULT_WIDTH;

		glm::ivec2 buttonScreen;
		glm::vec2 buttonTranslation;
		float verticalSpacing = 26.6f * screenHeight / DEFAULT_HEIGHT;


		for (int i = 0; i < dataCount; i++) {
			TypeBox& typeRef = typeBoxes.emplace_back(getVariableString(this->dataPtr, i, this->dataType), this->dataLabel.x, this->dataLabel.y + (verticalSpacing * (i + 1)), TA_left, 1.f, screenWidth, screenHeight);
			typeRef.inputType = UIComp::InputType_numeric;

			//only supporting align_left right now
			typeRef.clickBox.z = static_cast<int>(typeRef.clickBox.x + ratioWidth);
			typeRef.transform.translation.x = ((typeRef.clickBox.x + ratioWidth / 2) - (screenWidth / 2.f)) / (screenWidth / 2.f);
			typeRef.transform.scale.x = (ratioWidth / (DEFAULT_WIDTH / 2.f));

			buttonScreen = glm::ivec2(dataLabel.x + ratioWidth, (typeRef.clickBox.y + typeRef.clickBox.w) / 2); //? this is lining up the buttons with the top of textbox
			UIComp::convertScreenTo2D(buttonScreen, buttonTranslation, screenWidth, screenHeight);

			std::pair<Button, Button>& buttonRef = buttons.emplace_back(std::piecewise_construct, std::make_tuple(buttonTranslation, screenWidth, screenHeight), std::make_tuple(buttonTranslation, screenWidth, screenHeight));
			buttonRef.first.transform.translation.x += buttonRef.first.transform.scale.x / 2.f;
			buttonRef.second.transform.translation.x += buttonRef.second.transform.scale.x * 1.55f;
			buttonRef.first.transform.scale *= .8f;
			UIComp::convertTransformToClickBox(buttonRef.first.transform, buttonRef.first.clickBox, screenWidth, screenHeight);
			buttonRef.second.transform.scale *= .8f;
			UIComp::convertTransformToClickBox(buttonRef.second.transform, buttonRef.second.clickBox, screenWidth, screenHeight);

			buttonRef.second.transform.scale.y *= -1.f;
		}
	}

	bool VariableControl::Clicked(double xpos, double ypos) {
		for (int j = 0; j < typeBoxes.size(); j++) {
			if (typeBoxes[j].Clicked(xpos, ypos)) {
				selectedTypeBox = j;
				variableCtrlPtr = this;
				readyForInput = true;
				stringSelectionIndex = static_cast<int>(typeBoxes[j].textStruct.string.length());
				typeBoxes[j].textStruct.string += '|';

				readyForInput = true;
				variableCtrlPtr = this;
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
				addVariables(dataPtr, j, steps, stepOffset, dataType);
				typeBoxes[j].textStruct.string = getVariableString(dataPtr, j, dataType);
				return false;
			}
			else if (buttons[j].second.Clicked(xpos, ypos)) {
				//if holding shift largstep
				//if holding ctrl smallStep
				//*variables[j] -= mediumStep;
				printf("subtracting \n");
				int stepOffset = 1 - glfwGetKey(windowPtr, GLFW_KEY_LEFT_CONTROL) + glfwGetKey(windowPtr, GLFW_KEY_LEFT_SHIFT);
				subtractVariables(dataPtr, j, steps, stepOffset, dataType);
				typeBoxes[j].textStruct.string = getVariableString(dataPtr, j, dataType);
				return false;
			}
		}
		return false;
	}
	void VariableControl::resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) {

		float xRatio = rszWidth / oldWidth;
		float yRatio = rszHeight / oldHeight;
		for (int i = 0; i < buttons.size(); i++) {
			buttons[i].first.clickBox.x = static_cast<int>(static_cast<float>(buttons[i].first.clickBox.x) * xRatio);
			buttons[i].first.clickBox.z = static_cast<int>(static_cast<float>(buttons[i].first.clickBox.z) * xRatio);
			buttons[i].first.clickBox.y = static_cast<int>(static_cast<float>(buttons[i].first.clickBox.y) * yRatio);
			buttons[i].first.clickBox.w = static_cast<int>(static_cast<float>(buttons[i].first.clickBox.w) * yRatio);

			buttons[i].second.clickBox.x = static_cast<int>(static_cast<float>(buttons[i].second.clickBox.x) * xRatio);
			buttons[i].second.clickBox.z = static_cast<int>(static_cast<float>(buttons[i].second.clickBox.z) * xRatio);
			buttons[i].second.clickBox.y = static_cast<int>(static_cast<float>(buttons[i].second.clickBox.y) * yRatio);
			buttons[i].second.clickBox.w = static_cast<int>(static_cast<float>(buttons[i].second.clickBox.w) * yRatio);

			//UIComp::convertClickToTransform(buttons[i].first.clickBox, buttons[i].first.transform, screenWidth, screenHeight);
			//UIComp::convertClickToTransform(buttons[i].second.clickBox, buttons[i].second.transform, screenWidth, screenHeight);
			//buttons[i].first.transform.scale *= .8f;
			//buttons[i].second.transform.scale.y *= -0.8f;
			//buttons[i].second.transform.scale.x *= 0.8f;
		}
		for (int i = 0; i < typeBoxes.size(); i++) {

			typeBoxes[i].textStruct.x *= xRatio;
			typeBoxes[i].textStruct.y *= yRatio;
			typeBoxes[i].clickBox.x = static_cast<int>(static_cast<float>(typeBoxes[i].clickBox.x) * xRatio);
			typeBoxes[i].clickBox.z = static_cast<int>(static_cast<float>(typeBoxes[i].clickBox.z) * xRatio);
			typeBoxes[i].clickBox.y = static_cast<int>(static_cast<float>(typeBoxes[i].clickBox.y) * yRatio);
			typeBoxes[i].clickBox.w = static_cast<int>(static_cast<float>(typeBoxes[i].clickBox.w) * yRatio);

			//UIComp::convertClickToTransform(typeBoxes[i].clickBox, typeBoxes[i].transform, screenWidth, screenHeight);
		}

		dataLabel.x *= xRatio;
		dataLabel.y *= yRatio;
		//for (int i = 0; i < variableNames.size(); i++) {
		//	variableNames[i].x *= xRatio;
		//	variableNames[i].y *= yRatio;
		//}
		//dragBox.x *= xRatio;
		//dragBox.z *= xRatio;
		//dragBox.y *= yRatio;
		//dragBox.w *= yRatio;
		//shouldnt need to resize the transform?

		screenWidth = rszWidth;
		screenHeight = rszHeight;
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

			UIComp::convertClickToTransform(typeBoxes[i].clickBox, typeBoxes[i].transform, screenWidth, screenHeight);
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

			UIComp::convertClickToTransform(buttons[i].first.clickBox, buttons[i].first.transform, screenWidth, screenHeight);
			UIComp::convertClickToTransform(buttons[i].second.clickBox, buttons[i].second.transform, screenWidth, screenHeight);
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
					if (UIComp::checkClickBox(variableCtrlPtr->typeBoxes[i].clickBox, xpos, ypos)) {
						if (i != variableCtrlPtr->selectedTypeBox) {
							variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.erase(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.find_first_of('|'), 1);
							printf("remove the | \n");
							variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string = getVariableString(variableCtrlPtr->dataPtr, variableCtrlPtr->selectedTypeBox, variableCtrlPtr->dataType);
							variableCtrlPtr->selectedTypeBox = i;
						}
						printf("clicked a type box? \n");
						variableCtrlPtr->stringSelectionIndex = variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.getSelectionIndex(xpos, variableCtrlPtr->screenWidth);
						std::string& stringRef = variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string;
						uint16_t indexPos = static_cast<uint16_t>(stringRef.find_first_of('|'));
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
					variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string = getVariableString(variableCtrlPtr->dataPtr, variableCtrlPtr->selectedTypeBox, variableCtrlPtr->dataType);
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
				variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string = getVariableString(variableCtrlPtr->dataPtr, variableCtrlPtr->selectedTypeBox, variableCtrlPtr->dataType);
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
				stringRef = getVariableString(variableCtrlPtr->dataPtr, variableCtrlPtr->selectedTypeBox, variableCtrlPtr->dataType);
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
			if (variableCtrlPtr->dataType == UIComp::VT_float || variableCtrlPtr->dataType == UIComp::VT_double) {
				int lastLength = static_cast<int>(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.length());
				UIComp::TypeToString(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string, variableCtrlPtr->maxStringLength, codepoint, UIComp::InputType_float, variableCtrlPtr->stringSelectionIndex);
				lastLength = static_cast<int>(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.length()) - lastLength;
				variableCtrlPtr->stringSelectionIndex += lastLength;
			}
			else {
				int lastLength = static_cast<int>(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.length());
				UIComp::TypeToString(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string, variableCtrlPtr->maxStringLength, codepoint, UIComp::InputType_numeric, variableCtrlPtr->stringSelectionIndex);
				lastLength = static_cast<int>(variableCtrlPtr->typeBoxes[variableCtrlPtr->selectedTypeBox].textStruct.string.length()) - lastLength;
				variableCtrlPtr->stringSelectionIndex += lastLength;
			}
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
		UIComp::convertClickToTransform(dragBox, transform, screenWidth, screenHeight);
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

		if (UIComp::checkClickBox(dragBox, xpos, ypos)) {
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
		UIComp::convertClickToTransform(ctrlBoxPtr->dragBox, ctrlBoxPtr->transform, ctrlBoxPtr->screenWidth, ctrlBoxPtr->screenHeight);
		ctrlBoxPtr->dragBox.w = ctrlBoxPtr->dragBox.y + static_cast<int>(ctrlBoxPtr->label.scale * ctrlBoxPtr->verticalSpacing);
		
		//then readjust the back Controller window
	}

	void ControlBox::resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) {

		for (int i = 0; i < variableControls.size(); i++) {
			variableControls[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
	}


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
		backRef.dropper.textStruct.x = screenCoordinates.first + 4.f * screenWidth / DEFAULT_WIDTH;
		backRef.dropper.textStruct.y = screenCoordinates.second + (screenCoordinates.second + ((screenDimensions.second - 26.f) / 2.f));
		backRef.dropOptions.resize(options.size());
		for (int i = 0; i < options.size(); i++) {
			backRef.dropOptions[i].string = options[i];
		}
		//backRef.init(screenWidth, screenHeight);
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

}