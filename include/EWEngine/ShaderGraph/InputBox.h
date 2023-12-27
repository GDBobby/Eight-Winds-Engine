#pragma once
#include "EWEngine/GUI/UIComponents.h"
#include "EWEngine/GUI/MenuModule.h"

//#include "EWEngine/ShaderGraph/VariableType.h" //this is included in ShaderStructures but im going to explicitly include it anyways
#include "EWEngine/ShaderGraph/ShaderStructures.h"
#include "EWEngine/graphics/EWE_FrameInfo.h"

namespace EWE {
	namespace Shader {

		struct InputOutputVariable {
			VariableType typeID;
			std::string variableName;

			void writeToString(std::string& outString, bool inTrueOutFalse, uint16_t position);
		};
		struct InputOutputData {
			InputOutputVariable variable;
			ComboBox variableCombo;
			TypeBox name;
			Button removeThis;
			ShaderStructureManager& structureManager;


			InputOutputData(ShaderStructureManager& structureManager, float xPos, float yPos, float screenWidth, float screenHeight);
			InputOutputData(ShaderStructureManager& structureManager, VariableType vType, std::string variableName, float xPos, float yPos, float screenWidth, float screenHeight);

			void move(float xDiff, float yDiff, float screenWidth, float screenHeight);
		private:
			void populateCombo(float screenWidth, float screenHeight);
		};

		class InputBox {
		private:
			static InputBox* inputBoxPtr;
			static GLFWmousebuttonfun mouseReturnPointer;
			static GLFWkeyfun keyReturnPointer;
			int16_t selectedTypeBox = -1;
			bool readyForInput = false;
			int stringSelectionIndex = -1;
			uint32_t maxStringLength = 20;

			GLFWwindow* windowPtr;
			float screenWidth, screenHeight;
			glm::ivec4 backgroundScreen;

		public:
			InputBox(GLFWwindow* windowPtr, bool inputTrueOutputFalse, float xPos, float yPos, float screenWidth, float screenHeight);
			static void giveGLFWCallbacks(GLFWmousebuttonfun mouseReturnFunction, GLFWkeyfun keyReturnFunction) {
				InputBox::mouseReturnPointer = mouseReturnFunction;
				InputBox::keyReturnPointer = keyReturnFunction;
			}

			bool isActive = false;
			bool currentlyExposed = true;
			Transform2dComponent background;

			bool inputTrueOutputFalse;
			TextStruct name;

			std::vector<InputOutputData> variables{};

			Button addVariable;

			glm::ivec4 dragBox;

			//DragBar, check VariableControl for this. I think i had that functionality in there.
			void writeToString(std::string& outString) {
				for (uint16_t i = 0; i < variables.size(); i++) {
					variables[i].variable.writeToString(outString, inputTrueOutputFalse, i);
				}
			}

			//drag if selecting the top, 
			bool Clicked(double xpos, double ypos);

			void render(Simple2DPushConstantData& push, uint8_t drawID);

			void render(NineUIPushConstantData& push);

			static void MouseCallback(GLFWwindow* window, int button, int action, int mods);
			static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
			static void typeCallback(GLFWwindow* window, unsigned int codepoint);

			void drawText();
			
		};
	}
}

