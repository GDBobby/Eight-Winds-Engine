#pragma once
#include "VariableType.h"

#include <vector>
#include <string>
#include <utility>


namespace EWE {
	namespace Shader {

		struct Shader_Structure {
			std::string name{};
			std::vector<Shader_Variable> members{};
			Shader_Structure() = default;
			Shader_Structure(std::string& name, std::vector<Shader_Variable>& members) : name{ std::move(name) }, members{ std::move(members) } {}
		};

		class ShaderStructureManager {
		public:
			static void populateFromPreviouslyDefinedStructures() {}

			static int32_t addStructure(std::string& structure_name, std::vector<Shader_Variable>& members);

			static void writeStructure(std::string& outString, VariableType variableType);
			static void changeStructureName(std::string nextName, VariableType variableType);

			static std::string getStructureName(VariableType variableType);
			static std::vector<std::string> const& getAllVariableNames();
		private:
			static std::vector<std::string> all_variable_names;
			static std::vector<Shader_Structure> shader_structures;

		};
	}
}

