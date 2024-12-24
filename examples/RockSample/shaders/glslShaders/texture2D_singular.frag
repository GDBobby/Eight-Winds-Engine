#version 460

layout (location = 0) in vec2 fragTexCoord;
layout (location = 0) out vec4 outColor;
//this is getting forced to include "spvSparseResidency"
//i dont quite know what it means or why its doing that, so im going to write this in GLSL

layout(set = 0, binding = 0) uniform sampler2D tex;

layout(push_constant) uniform Push {
    vec4 scaleOffset;
	vec3 color;
} push;

void main() {
    //ivec2 textureSizeAtLOD0 = textureSize(tex, 0);
	ivec2 texelCoords;
	texelCoords.x = fragTexCoord.x;
	texelCoords.y = fragTexCoord.y;

	vec4 texColor = texture(tex, texelCoords).rgba;
    
    outColor = vec4(texColor.rgb, texColor.a);
    
}