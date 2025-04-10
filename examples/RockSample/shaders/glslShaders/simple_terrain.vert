#version 450
#extension GL_GOOGLE_include_directive : enable

#include "SimplexNoise.glsl"

layout(set = 0, binding = 0) uniform GlobalUbo{
	mat4 projView;
	vec4 cameraPos;
} ubo;

layout(set = 0, binding = 2) uniform TescBO{
    mat4 projection;
    mat4 view;
    vec4 frustumPlanes[6];
    vec2 viewportDim;
    float displacementFactor;
    float tessFactor;
    float tessEdgeSize;
	int octaves;
	float worldPosNoiseScaling;

    float sandHeight;
    float grassHeight;
} tbo;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outPos;



void main(void) {
    vec3 worldPos = inPos;
    worldPos.x += ubo.cameraPos.x;
    worldPos.z += ubo.cameraPos.z;

    worldPos.y = -NoiseWithOctaves(worldPos.xz / tbo.worldPosNoiseScaling, tbo.octaves) * tbo.displacementFactor;

	gl_Position = ubo.projView * vec4(worldPos, 1.0);
	outUV = inUV;
	outNormal = inNormal;
	outPos = worldPos;
}