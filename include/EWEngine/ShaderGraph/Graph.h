#pragma once
#include <string>
#include <vector>
#include "Node.h"
#include "Edge.h"
#include "InputBox.h"

namespace EWE {
	namespace Shader {
		class Graph {
			Node inputNode;
			Node outputNode;
			InputBox inputBox;
			InputBox outputBox;

		public:
			Graph(GLFWwindow* windowPtr, float screenWidth, float screenHeight);
			void render(Simple2DPushConstantData& push, uint8_t drawID);
			void render(NineUIPushConstantData& push);
			bool Clicked(double xpos, double ypos);
			void drawText();

			//id liek to spell it out first, then make the systems to handle it

			std::string mainEntry = "int main(){"; //this is implicit, and will be in every shader.
			std::string mainExit = "}";

			void AddStructures(std::string& buildString) {
				//buildString += necessaryStructures;
			}

			void AddDescriptorInfo(std::string& buildString) {
				//buildString += descriptorInfo;
			}
			void AddFunctions(std::string& buildString) {
				//buildString += functions;
			}
			void AddMainFunctionWork(std::string& buildString) {

			}
			bool buildToFile(std::string fileLocation);

			std::string build(bool exporting);
		};
	}
}
