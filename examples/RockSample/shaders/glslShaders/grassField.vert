#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
//layout(location = 1) in float uv;

//instancing
layout(location = 2) in mat4 instanceTransform;

//layout(location = 0) out float fragUV;
layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo{
	mat4 projView;
	vec4 cameraPos;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform Push{
	vec2 uvScroll;
} push;

void main(){
	//position += texture(texSampler, instanceTexCoord).rgb * position.y;
	vec4 positionWorld = instanceTransform * (vec4(position, 1.0));
	positionWorld = vec4(positionWorld.xyz - (texture(texSampler, (positionWorld.xz / 10.0) - push.uvScroll).rgb * position.y * 25.0), positionWorld.w);
	gl_Position = ubo.projection * ubo.view * positionWorld;
	
	//fragUV = uv;
	fragColor = vec4(color, 1.0);
}