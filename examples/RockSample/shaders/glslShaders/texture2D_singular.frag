#version 460

layout (location = 0) in vec2 fragTexCoord;
layout (location = 0) out vec4 outColor;
//this is getting forced to include "spvSparseResidency"
//i dont quite know what it means or why its doing that, so im going to write this in GLSL

layout(set = 0, binding = 0) uniform sampler2D tex;

layout(push_constant) uniform Push {
    mat3 transform;
} push;

void main() {
    ivec2 textureSizeAtLOD0 = textureSize(tex, 0);
	ivec2 texelCoords;
	texelCoords.x = int(float(textureSizeAtLOD0.x) * fragTexCoord.x);
	texelCoords.y = int(float(textureSizeAtLOD0.y) * fragTexCoord.y);

	vec4 texColor = texelFetch(tex, texelCoords, 0).rgba;
    
    outColor = vec4(texColor.rgb, texColor.a);
    
}