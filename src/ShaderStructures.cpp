#include "EWEngine/ShaderGraph/ShaderStructures.h"

#include <stdexcept>
#include <cstdint>

namespace EWE {
	namespace Shader {
		std::vector<Shader_Structure> ShaderStructureManager::shader_structures{};
		std::vector<std::string> ShaderStructureManager::all_variable_names{};

		int32_t ShaderStructureManager::addStructure(std::string& structure_name, std::vector<Shader_Variable>& members) {
			if (shader_structures.size() >= INT32_MAX) {
				printf("impressive amount of shaders \n");
				throw std::runtime_error("shader count overflow. who has 2 billion shaders ???");
			}

			for (auto& structure : shader_structures) {
				if (structure.name == structure_name) {
					return -1;
				}
			}
			shader_structures.emplace_back(structure_name, members);
			if (all_variable_names.size() == 0) {
				for (auto& vName : VariableTypeString) {
					all_variable_names.emplace_back(vName);
				}
			}
			all_variable_names.emplace_back(structure_name);

			return true;

		}

		void ShaderStructureManager::writeType(std::string& outString, VariableType variableType) {
			if (variableType >= Type_Struct) {
				outString += shader_structures[variableType - Type_Struct].name + ' ';
			}
			else {
				outString += VariableTypeString[variableType] + ' ';
			}
		}
		void ShaderStructureManager::writeStructureDefinition(std::string& outString, VariableType variableType) {
			auto& structure = shader_structures[variableType - Type_Struct];

			outString += "struct " + structure.name + '{';
			for (auto& member : structure.members) {
				if (member.type >= Type_Struct) {
					outString += shader_structures[member.type - Type_Struct].name;
				}
				else {
					outString += VariableTypeString[member.type];
				}
				outString += member.name + ';';
			}
			outString += "};";
		}

		std::string ShaderStructureManager::getStructureName(VariableType variableType) {
			return shader_structures[variableType - Type_Struct].name;
		}
		void ShaderStructureManager::changeStructureName(std::string nextName, VariableType variableType) {
			all_variable_names.at(variableType) = nextName;
			shader_structures.at(variableType - Type_Struct).name = nextName;
		}

		std::vector<std::string> const& ShaderStructureManager::getAllVariableNames() {
			if (all_variable_names.size() == 0) {
				for (auto& vName : VariableTypeString) {
					all_variable_names.emplace_back(vName);
				}
			}
			return all_variable_names;
		}
	}
}