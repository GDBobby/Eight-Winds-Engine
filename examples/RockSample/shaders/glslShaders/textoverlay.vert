#version 450 core

layout (location = 0) out vec2 outUV;

layout(set = 0, binding = 0) uniform inData{
	vec4 posUV[16384];
};


void main(void) {
	gl_Position = vec4(posUV[gl_VertexIndex + gl_InstanceIndex * 4].xy, 0.0, 1.0);
	outUV = posUV[gl_VertexIndex + gl_InstanceIndex * 4].zw;
}

//based on some old version of this https://github.com/SaschaWillems/Vulkan/blob/master/shaders/glsl/base/textoverlay.vert