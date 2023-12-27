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
			void populateFromPreviouslyDefinedStructures() {}

			int32_t addStructure(std::string& structure_name, std::vector<Shader_Variable>& members);

			void writeStructureDefinition(std::string& outString, VariableType variableType);
			void changeStructureName(std::string nextName, VariableType variableType);
			void writeType(std::string& outString, VariableType variableType);

			std::string getStructureName(VariableType variableType);
			std::vector<std::string> const& getAllVariableNames();
		private:
			std::vector<std::string> all_variable_names;
			std::vector<Shader_Structure> shader_structures;

		};
	}
}

