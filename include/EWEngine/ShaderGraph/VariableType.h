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
			GLSL_Type_mat2,
			GLSL_Type_mat3,
			GLSL_Type_mat4,

			GLSL_Type_double,
			GLSL_Type_dvec2,
			GLSL_Type_dvec3,
			GLSL_Type_dvec4,
			GLSL_Type_dmat2,
			GLSL_Type_dmat3,
			GLSL_Type_dmat4,

			GLSL_Type_int,
			GLSL_Type_ivec2,
			GLSL_Type_ivec3,
			GLSL_Type_ivec4,

			GLSL_Type_uint,
			GLSL_Type_uvec2,
			GLSL_Type_uvec3,
			GLSL_Type_uvec4,

			GLSL_Type_bool,
			GLSL_Type_bvec2,
			GLSL_Type_bvec3,
			GLSL_Type_bvec4,

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