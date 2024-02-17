#version 450

layout(location = 0) in vec2 tileUV;

layout (location = 0) out vec4 outColor;

struct PointLight{
	vec4 position; //ignore w
	vec4 color; //w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo{
	mat4 projection;
	mat4 view;
	vec4 cameraPos;
	//vec4 ambientLightColor;
} ubo;

layout(set = 2, binding = 0) uniform sampler2D texSampler;

void main(){
	ivec2 texelCoords;
	texelCoords.x = int(tileUV.x);
	texelCoords.y = int(tileUV.y);
	vec4 texColor = texelFetch(texSampler, texelCoords, 0).rgba;
	//vec4 texColor = texture(texSampler, tileUV, 0);

	//outColor = vec4(0);
	//outColor.b = tileUV.x;
	//outColor.g = tileUV.y;
	//outColor.a = 1.0;
	//return;
	outColor = texColor;
	//outColor = vec4(fragColor, 1.0);
}