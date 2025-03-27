#pragma once

#include <string>
#include <vector>

namespace VertexShaderText {
	const std::vector<std::string> vertexTangentInput = {
		"layout(location = 0) in vec3 position;",
		"layout(location = 1) in vec3 normal;",
		"layout(location = 2) in vec2 uv;",
		"layout(location = 3) in vec3 tangent;",
	};
	const std::vector<std::string> hasBoneTangentInput = {
		"layout(location = 4) in ivec4 boneIds;",
		"layout(location = 5) in vec4 weights;",
	};
	const std::vector<std::string> vertexNNInput = {
		"layout(location = 0) in vec3 position;",
		"layout(location = 1) in vec3 normal;",
		"layout(location = 2) in vec2 uv;",
		"const int MAX_BONE_INFLUENCE = 4;",
	};
	const std::vector<std::string> hasBoneNNInput = {
		"layout(location = 3) in ivec4 boneIds;",
		"layout(location = 4) in vec4 weights;",
		"const int MAX_BONE_INFLUENCE = 4;",
	};

	const std::vector<std::string> vertexTangentOutput = {
		"layout(location = 0) out vec3 fragPosWorld;",
		"layout(location = 1) out vec3 fragNormalWorld;",
		"layout(location = 2) out vec2 fragTexCoord;",
		"layout(location = 3) out vec3 fragTangentWorld;",
	};
	const std::vector<std::string> vertexNNOutput = {
		"layout(location = 0) out vec3 fragPosWorld;",
		"layout(location = 1) out vec3 fragNormalWorld;",
		"layout(location = 2) out vec2 fragTexCoord;",
		"layout(location = 3) out vec3 fragTangentWorld;",
	};

	const std::vector<std::string> vertexEntry = {

		"struct PointLight{vec4 position;vec4 color;};",

		"layout(set = 0, binding = 0) uniform GlobalUbo {",
		"mat4 projView;vec4 cameraPos;} ubo;",

		"layout(set = 0, binding = 1) uniform GPUSceneData {",
		"vec4 ambientColor;vec4 sunlightDirection;vec4 sunlightColor;",
		"PointLight pointLights[10];int numLights;",
		"} gpuScene;",
	};
	//when ready to swap out pipelines for smaller actor counts
	const std::vector<std::string> vertexSmallInstanceBuffersFirstHalf = {
		"layout(set = 0, binding = 2) uniform ModelMatrices { mat4 modelMatrix[]; }; ",
		"layout (set = 0, binding = 3) readonly buffer JointMatrices {mat4 finalBonesMatrices[];};"
		"void main(){int boneIndex = gl_InstanceIndex *",
	};

	const std::vector<std::string> vertexInstanceBuffersFirstHalf = {
		"layout(set = 0, binding = 2) readonly buffer ModelMatrices { mat4 modelMatrix[]; }; ",
		"layout (set = 0, binding = 3) readonly buffer JointMatrices {mat4 finalBonesMatrices[];};"
		"void main(){int boneIndex = gl_InstanceIndex *",
	};
	const std::vector<std::string> vertexInstanceBuffersSecondHalf = {
		"mat4 skinMat = finalBonesMatrices[boneIds[0] + boneIndex] * weights[0] + finalBonesMatrices[boneIds[1] + boneIndex] * weights[1]",
		"+ finalBonesMatrices[boneIds[2] + boneIndex] * weights[2] + finalBonesMatrices[boneIds[3] + boneIndex] * weights[3]; ",
		"vec4 positionWorld = modelMatrix[gl_InstanceIndex] * skinMat * vec4(position, 1.0f);",
		"gl_Position = ubo.projView * positionWorld;"
	};

	const std::vector<std::string> vertexNoInstanceBuffers = {
		"layout (set = 0, binding = 2) uniform JointMatrices {mat4 finalBonesMatrices[];};"
		"layout(push_constant) uniform Push {mat4 modelMatrix;int index_boneCount;} push; ",
		"void main(){"
		"mat4 skinMat = finalBonesMatrices[boneIds[0] + push.index_boneCount] * weights[0] + finalBonesMatrices[boneIds[1] + push.index_boneCount] * weights[1]",
		"+ finalBonesMatrices[boneIds[2] + push.index_boneCount] * weights[2] + finalBonesMatrices[boneIds[3] + push.index_boneCount] * weights[3];",
		"vec4 positionWorld = push.modelMatrix * skinMat * vec4(position, 1.0f);",
		"gl_Position = ubo.projView * positionWorld;",
	};

	const std::vector<std::string> vertexTangentInstancingMainExit = {
		"const mat3 trans3 = mat3(modelMatrix[gl_InstanceIndex] * skinMat);"
		"fragNormalWorld = normalize(transpose(inverse(trans3)) * normal);",
		"fragTangentWorld = normalize(trans3 * tangent);",

		"fragPosWorld = positionWorld.xyz;fragTexCoord = uv;}"
	};

	const std::vector<std::string> vertexTangentNoInstancingMainExit = {
		"const mat3 trans3 = mat3(push.modelMatrix * skinMat);"
		"fragNormalWorld = normalize(transpose(inverse(trans3)) * normal);",
		"fragTangentWorld = normalize(trans3 * tangent);",

		"fragPosWorld = positionWorld.xyz;fragTexCoord = uv;}",
	};

	const std::vector<std::string> vertexNNInstancingMainExit = {
		"fragNormalWorld = normalize(transpose(inverse(mat3(modelMatrix[gl_InstanceIndex] * skinMat))) * normal);",
		"fragPosWorld = positionWorld.xyz; fragTexCoord = uv;}",
	};


	const std::vector<std::string> vertexNNNoInstancingMainExit = {
		"fragNormalWorld = normalize(transpose(inverse(mat3(push.modelMatrix * skinMat))) * normal);",
		"fragPosWorld = positionWorld.xyz; fragTexCoord = uv;}",
	};
}