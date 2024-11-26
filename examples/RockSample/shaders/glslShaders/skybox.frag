#version 450 core
layout (location = 0) out vec4 outColor;

layout(location = 0) in vec3 TexCoords;

layout (set = 0, binding = 1) uniform samplerCube skybox;

void main() {    
    outColor = texture(skybox, TexCoords);
}