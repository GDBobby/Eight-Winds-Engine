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


			InputOutputData(float xPos, float yPos, float screenWidth, float screenHeight);
			InputOutputData(VariableType vType, std::string variableName, float xPos, float yPos, float screenWidth, float screenHeight);
		private:
			void populateCombo(float screenWidth, float screenHeight);
		};

		class InputBox {
		private:
			static InputBox* inputBoxPtr;
			uint16_t selectedTypeBox;
			bool readyForInput = false;
			int stringSelectionIndex = -1;
			uint32_t maxStringLength = 20;

			GLFWwindow* windowPtr;
			float screenWidth, screenHeight;
			GLFWmousebuttonfun mouseReturnPointer;
			GLFWkeyfun keyReturnPointer;

		public:
			InputBox(GLFWwindow* windpwPtr, bool inputTrueOutputFalse, float xPos, float yPos, float screenWidth, float screenHeight);
			void setCallbackReturnPointers(GLFWmousebuttonfun mouseReturnPointer, GLFWkeyfun keyReturnPointer) {
				this->mouseReturnPointer = mouseReturnPointer;
				this->keyReturnPointer = keyReturnPointer;
			}
				

			bool isActive = false;
			bool currentlyExposed = true;
			Transform2dComponent background;

			bool inputTrueOutputFalse;
			TextStruct name;

			std::vector<InputOutputData> variables{};

			Button addVariable;

			//DragBar, check VariableControl for this. I think i had that functionality in there.
			void writeToString(std::string& outString) {
				for (uint16_t i = 0; i < variables.size(); i++) {
					variables[i].variable.writeToString(outString, inputTrueOutputFalse, i);
				}
			}

			//drag if selecting the top, 
			bool Clicked(double xpos, double ypos);

			void render(NineUIPushConstantData& push) {

				push.color = glm::vec3{ .5f, .35f, .25f };
				Dimension2::bindTexture9(MenuModule::textureIDs[MT_NineUI]);

				if (variables.size() > 0) {
					push.color = glm::vec3{ .5f, .35f, .25f };
					for (int i = 0; i < variables.size(); i++) {
						push.color = glm::vec3{ .5f, .35f, .25f };

						push.offset = glm::vec4(variables[i].variableCombo.activeOption.transform.translation, 1.f, 1.f);
						//need color array
						push.scale = variables[i].variableCombo.activeOption.transform.scale;
						Dimension2::pushAndDraw(push);

						variables[i].variableCombo.render(push);
					}
				}
				
			}

			static void MouseCallback(GLFWwindow* window, int button, int action, int mods);
			static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
			static void typeCallback(GLFWwindow* window, unsigned int codepoint);

			
		};
	}
}

