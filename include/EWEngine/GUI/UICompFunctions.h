#pragma once

#include "EWEngine/GameObject2D.h"
#include "EWEngine/Graphics/TextOverlay.h"
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
		size_t GetVariableSize(VariableType vType);
		std::string GetVariableString(void* data, int offset, VariableType vType);
		void SetVariableFromString(void* data, int offset, std::string& inString, VariableType vType);
		void AddVariables(void* firstData, int32_t firstOffset, void* secondData, int32_t secondOffset, VariableType vType);
		void SubtractVariables(void* firstData, int32_t firstOffset, void* secondData, int32_t secondOffset, VariableType vType);

		void TypeToString(std::string& outputString, uint16_t maxStringLength, int codePoint, InputType inputType, uint8_t stringSelectionIndex);


		//2d to screen conversions
		void ConvertTransformToClickBox(Transform2D& transform, glm::ivec4& clickBox, float screenWidth, float screenHeight);

		bool CheckClickBox(glm::ivec4& clickBox, double mouseX, double mouseY);

		void TextToTransform(Transform2D& transform, TextStruct& textStruct, glm::ivec4& clickBox, float screenWidth, float screenHeight);

		void ConvertScreenTo2D(glm::ivec2 screen, glm::vec2& coord2D, float screenWidth, float screenHeight);
		void PrintClickBox(glm::ivec4& clickBox);

		void ConvertClickToTransform(glm::ivec4& clickBox, Transform2D& transform, float screenWidth, float screenHeight);

		glm::ivec2 ConvertWorldCoordinatesToScreenCoordinates(glm::vec2 worldCoord, float screenWidth, float screenHeight);

	};
}