#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragTangentWorld;
layout(location = 4) out float instanceIndex;

struct PointLight{
	vec4 position; //ignore w
	vec4 color; //w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo{
	mat4 projView;
	vec4 cameraPos;
} ubo;

layout(set = 0, binding = 1) uniform GPUSceneData{
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
	PointLight pointLights[10]; //max lights in frameinfo header
	int numLights;
} gpuScene;

layout(set = 0, binding = 2) buffer TransformData{
	mat4 transformMatrices[];
};

void main(){
	const vec4 positionWorld = transformMatrices[gl_InstanceIndex] * vec4(position, 1.0);
	gl_Position = ubo.projView * positionWorld;
	fragPosWorld = positionWorld.xyz;

	//fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
	const mat3 trans3 = mat3(transformMatrices[gl_InstanceIndex]);
	fragNormalWorld = normalize(transpose(inverse(trans3)) * normal);
	
	fragTexCoord = uv;
	fragTangentWorld = normalize(trans3 * tangent);
	
	instanceIndex = float(gl_InstanceIndex);
}