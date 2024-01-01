/*
#include "EWEngine/ShaderGraph/Graph.h"

namespace EWE {
	namespace Shader {
		void recurseDirectoryCreation(std::string const& directory, size_t currentPos, uint8_t& currentIter, const uint8_t maxIter) {
			if (directory.length() < 2 || currentIter >= maxIter) {
				return;
			}

			currentPos = directory.find_first_of('/', currentPos);
			
			currentPos = std::min(currentPos, directory.find_first_of('\\', currentPos));
			if (currentPos != std::string::npos) {
				if (currentPos != 0) {
					printf("directory substr : %s \n", directory.substr(0, currentPos).c_str());
					std::filesystem::create_directory(directory.substr(0, currentPos));
				}
				recurseDirectoryCreation(directory, currentPos + 1, currentIter, maxIter);

			}
		}

		Graph::Graph(GLFWwindow* windowPtr, float screenWidth, float screenHeight) :
			inputBox{ windowPtr, true, 100.f, 100.f, screenWidth, screenHeight },
			outputBox{ windowPtr, false, 600.f, 100.f, screenWidth, screenHeight } {

		}

		void Graph::render(Simple2DPushConstantData& push, uint8_t drawID) {
			inputBox.render(push, drawID);
			outputBox.render(push, drawID);
		}
		void Graph::render(NineUIPushConstantData& push) {
			inputBox.render(push);
			outputBox.render(push);
		}
		void Graph::drawText() {
			inputBox.drawText();
			outputBox.drawText();
		}

		bool Graph::Clicked(double xpos, double ypos) {
			if (inputBox.Clicked(xpos, ypos)) {
				return true;
			}
			else {
				return outputBox.Clicked(xpos, ypos);
			}
		}

		bool Graph::buildToFile(std::string fileLocation) {
			if (std::filesystem::exists(fileLocation)) {
				printf("shader graph desired file location exists \n");
			}
			else {
				uint8_t currentIter = 0;
				recurseDirectoryCreation(fileLocation, 0, currentIter, 4);
				printf("file depth : %d \n", currentIter);

				if (std::filesystem::exists(fileLocation)) {
					printf("shader graph desired file location exists round2 \n");
				}
			}

			std::ofstream outFile{ fileLocation };
			if (!outFile.is_open()) {
				outFile.open(fileLocation);
				if (!outFile.is_open()) {
					printf("failed to open file for saving shader graph \n");
					return false;
				}
			}
			std::string builtString{ build(true) };
			outFile.write(builtString.c_str(), builtString.length());
			return true;
		}
		std::string Graph::build(bool exporting) {
			std::string builtString{};
			if (exporting) {
				builtString += "#version 450\n";
			}

			AddStructures(builtString);
			inputBox.writeToString(builtString);
			outputBox.writeToString(builtString);

			AddDescriptorInfo(builtString);

			AddFunctions(builtString);
			builtString += mainEntry;

			AddMainFunctionWork(builtString);

			builtString += mainExit;
			return builtString;
		}
	}
}
*/