#pragma once

#include <string>
#include <vector>
#include <cstdint>

typedef uint16_t VariableType;

namespace EWE {
	namespace Shader {
		enum VariableType_Enum : VariableType {
			GLSL_Type_float = 0,
			GLSL_Type_vec2,
			GLSL_Type_vec3,
			GLSL_Type_vec4,
			GLSL_Type_mat3,
			GLSL_Type_mat4,
			GLSL_Type_int,

			Type_Struct, //this will be written in ShaderStructures.h
		};

		const std::vector<std::string> VariableTypeString = {
			"float",
			"vec2",
			"vec3",
			"vec4",
			"mat3",
			"mat4",
			"int",
		};


		struct Shader_Variable {
			VariableType type;
			std::string name;
		};
	}
}