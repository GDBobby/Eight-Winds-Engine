/*
#pragma once

#include "EWEngine/ShaderGraph/ShaderStructures.h"

typedef uint16_t FunctionID;
#include <unordered_map>

namespace EWE {
	namespace Shader {
		static constexpr uint16_t VOID_RETURN_TYPE = UINT16_MAX;

		struct Parameter_Variable {
			VariableType type;
			std::string variableName;
		};

		struct Shader_Function_Definitions{
			VariableType returnVar = VOID_RETURN_TYPE;
			std::vector<Parameter_Variable> parameters{};
			std::string name{"functionName"};
			std::string action{};

			void writeToString(std::string& outString, ShaderStructureManager& structureManager) {
				if (returnVar == VOID_RETURN_TYPE) {
					outString += "void ";
				}
				else {
					structureManager.writeType(outString, returnVar);
				}
				outString += name + '(';


				for (auto& param : parameters) {
					structureManager.writeType(outString, param.type);
					outString += " " + param.variableName + ", ";
				}
				outString += "){";

				outString += action;

				outString += '}';
			}
		};
		struct Shader_Functions {
			std::vector<VariableType> parameterTypes{};
			std::string name{""};
		};

		struct GLSL_Functions {
			std::vector<Shader_Functions> glslFunctions{};

			GLSL_Functions() {
				for (int i = 0; i < 100; i++) {
					auto& functionBack = glslFunctions.emplace_back();
					functionBack.name = "abs";
					//functionBack.parameterTypes = gen(iType)
				}
			}
		};



	}
}

*/