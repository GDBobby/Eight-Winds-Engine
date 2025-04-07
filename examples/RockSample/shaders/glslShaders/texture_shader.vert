#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv; //tex coord?

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec2 fragTexCoord;

struct PointLight{
	vec4 position; //ignore w
	vec4 color; //w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo{
	mat4 projView;
	vec4 cameraPos;
	//vec4 ambientLightColor;
} ubo;

layout(set = 0, binding = 1) uniform GPUSceneData{
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
	PointLight pointLights[10]; //max lights in frameinfo header
	int numLights;
} gpuScene;

layout(push_constant) uniform Push {
	mat4 modelMatrix; 
	mat3 normalMatrix;
} push;

void main(){
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
	gl_Position = ubo.projView * positionWorld;

	fragNormalWorld = normalize(push.normalMatrix * normal);
	fragPosWorld = positionWorld.xyz;
	
	fragTexCoord = uv;
}