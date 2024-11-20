#version 460

layout (location = 0) in vec2 fragTexCoord;
layout (location = 0) out vec4 outColor;
//this is getting forced to include "spvSparseResidency"
//i dont quite know what it means or why its doing that, so im going to write this in GLSL

layout(set = 0, binding = 0) uniform sampler2DArray uiTextures;

layout(push_constant) uniform Push {
    vec4 scaleOffset;
    vec4 color; //the 4th float is for alignment only
    int textureID;
} push;

void main() {
    const vec4 texColor = texture(uiTextures, vec3(fragTexCoord, push.textureID));
    
    outColor = vec4(push.color.rgb * texColor.rgb, texColor.a);
    
}