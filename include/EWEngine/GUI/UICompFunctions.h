#pragma once

#include "EWEngine/GameObject2D.h"
#include "EWEngine/graphics/TextOverlay.h"
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

		glm::ivec2 convertWorldCoordinatesToScreenCoordinates(glm::vec2 worldCoord, float screenWidth, float screenHeight);

	};
}