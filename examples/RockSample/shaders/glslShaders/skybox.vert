#version 450 core
layout (location = 0) in vec3 pos;

layout(location = 0) out vec3 TexCoords;

layout(set = 0, binding = 0) uniform GlobalUbo{
	mat4 projView;
	vec4 cameraPos;
} ubo;

void main() {
    TexCoords = pos / 10000.f; //need the negative because the skyboxes are being rendered upside down
    gl_Position = ubo.projView * vec4(pos, 1.0);
}