#include "EWEngine/GUI/UICompFunctions.h"

#define FLOAT_PTR_PRECISION 4 //desired precision + 1

namespace EWE {
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
			//screen to world?
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
						return;
					}
					//fallthrough
				}
				case InputType_alpha: { //fallthrough to lower as well
					if (codepoint >= 65 && codepoint <= 90) {
						outputString += codepoint; 
						return;
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
					if ((codepoint >= 48) && (codepoint <= 57)) {
						outputString.insert(outputString.begin() + stringSelectionIndex, (char)codepoint);
					}
					else if (codepoint == 45) {
						if (outputString.find_first_of("-") != outputString.npos) {
							outputString = outputString.erase(0, 1);
						}
						else {
							outputString.insert(0, "-");
						}
						return;
					}
					return;
				}
				case InputType_float: {
					if ((codepoint >= 48) && (codepoint <= 57)) {
						outputString.insert(outputString.begin() + stringSelectionIndex, (char)codepoint);
						
					}
					else if (codepoint == 46) {
						if (outputString.find_first_of('.') == outputString.npos) {
							outputString.insert(outputString.begin() + stringSelectionIndex, (char)codepoint);
						}
					}
					else if (codepoint == 45) {
						if (outputString.find_first_of("-") != outputString.npos) {
							outputString = outputString.erase(0, 1);
						}
						else {
							outputString.insert(0, "-");
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

		glm::ivec2 convertWorldCoordinatesToScreenCoordinates(glm::vec2 worldCoord, float screenWidth, float screenHeight) {
			glm::ivec2 ret{ static_cast<int>(screenWidth / 2.f), static_cast<int>(screenHeight / 2.f)};

			//1 adds a quarter, 2 adds a half

			ret.x += static_cast<int>(worldCoord.x * screenWidth / 4.f);
			ret.y += static_cast<int>(worldCoord.y * screenHeight / 4.f);

			return ret;
		}


	}
}