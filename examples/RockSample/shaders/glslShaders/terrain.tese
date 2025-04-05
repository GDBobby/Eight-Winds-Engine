#version 450

https://github.com/SaschaWillems/Vulkan/blob/master/shaders/glsl/terraintessellation/terrain.tese

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
} tbo;

layout (set = 0, binding = 4) uniform sampler2D displacementMap; 

layout(quads, equal_spacing, cw) in;

layout (location = 0) in vec3 inNormal[];
layout (location = 1) in vec2 inUV[];
 
layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;

void main() {
	// Interpolate UV coordinates
	vec2 uv1 = mix(inUV[0], inUV[1], gl_TessCoord.x);
	vec2 uv2 = mix(inUV[3], inUV[2], gl_TessCoord.x);
	outUV = mix(uv1, uv2, gl_TessCoord.y);

	vec3 n1 = mix(inNormal[0], inNormal[1], gl_TessCoord.x);
	vec3 n2 = mix(inNormal[3], inNormal[2], gl_TessCoord.x);
	outNormal = mix(n1, n2, gl_TessCoord.y);

	// Interpolate positions
	vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);
	// Displace
	pos.y -= textureLod(displacementMap, outUV, 0.0).r * tbo.displacementFactor;
	// Perspective projection
	gl_Position = ubo.projView * pos;

	//outWorldPos = pos.xyz;
}