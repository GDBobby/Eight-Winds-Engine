#pragma once
#include <string>
#include <vector>
#include "node.h"
#include "edge.h"

namespace EWE {
	class Graph {
		Node inputNode;
		Node outputNode;

		//id liek to spell it out first, then make the systems to handle it

		std::string mainEntry = "int main(){"; //this is implicit, and will be in every shader.
		std::string mainExit = "}";

		void AddStructures(std::string& buildString) {
			//buildString += necessaryStructures;
		}

		void AddInput(std::string& buildString) {
			//inputNode.writeToString(buildString);
			//buildString += input;
		}
		void AddOutput(std::string& buildString) {
			//buildString += output;
		}
		void AddDescriptorInfo(std::string& buildString) {
			//buildString += descriptorInfo;
		}
		void AddFunctions(std::string& buildString) {
			//buildString += functions;
		}
		void AddMainFunctionWork(std::string& buildString) {

		}

		std::string build() {
			std::string builtString;

			AddStructures(builtString);
			AddInput(builtString);
			AddOutput(builtString);
			AddDescriptorInfo(builtString);

			AddFunctions(builtString);
			builtString += mainEntry;
		
			AddMainFunctionWork(builtString);

			builtString += mainExit;

		}
	};
}
