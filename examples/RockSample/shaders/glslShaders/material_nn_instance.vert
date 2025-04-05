#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out int instanceIndex;

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
	vec4 positionWorld = transformMatrices[gl_InstanceIndex] * vec4(position, 1.0);
	gl_Position = ubo.projView * positionWorld;
	fragPosWorld = positionWorld.xyz;

	//fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
	fragNormalWorld = normalize(transpose(inverse(mat3(transformMatrices[gl_InstanceIndex]))) * normal);
	
	fragTexCoord = uv;
	instanceIndex = gl_InstanceIndex;
}