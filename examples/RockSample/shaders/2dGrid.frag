#version 450

layout (location = 0) in vec2 gridCoord;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform Push {
  vec4 scaleOffset;
  vec2 gridScale;
  vec3 color;
} push;

void main() {
	outColor = vec4(push.color, 1.0);
	
	vec2 fractaled = fract(gridCoord);
	fractaled.x -= 0.5;
	fractaled.y -= 0.5;
	fractaled *= 2.0;
	
	float xS = fractaled.x * fractaled.x * fractaled.x * fractaled.x * fractaled.x * fractaled.x;
	float yS = fractaled.y * fractaled.y * fractaled.y * fractaled.y * fractaled.y * fractaled.y;
	float curve = 1 / (xS + yS);
	
	float fractaledLength = sqrt(fractaled.x * fractaled.x + fractaled.y * fractaled.y);
	
	outColor *= float(fractaledLength > curve);

}