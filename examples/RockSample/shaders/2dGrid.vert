#version 450

layout (location = 0) in vec2 position;

const vec2 GRID[6] = vec2[](
	vec2(0.0, 0.0),
	vec2(0.0, 1.0),
	vec2(1.0, 0.0),
	vec2(1.0, 0.0),
	vec2(0.0, 1.0),
	vec2(1.0, 1.0)
);

layout(location = 0) out vec2 gridCoordinate;

layout(push_constant) uniform Push {
  vec4 scaleOffset;
  vec2 gridScale;
  vec3 color;
} push;

void main() {
	gl_Position = vec4((position.x + push.scaleOffset.z) * push.scaleOffset.x, (position.y + push.scaleOffset.w) * push.scaleOffset.y, 0.0, 1.0);
	//gl_Position = vec4(push.scaleOffset.xy * position + push.scaleOffset.zw, 0.0, 1.0);
  
	gridCoordinate = (GRID[gl_VertexIndex]) * push.gridScale;// * push.scaleOffset.xy;
}