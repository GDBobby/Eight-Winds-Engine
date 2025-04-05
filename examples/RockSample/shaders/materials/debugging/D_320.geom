layout(triangles) in;
layout(location = 0) out vec4 fragColor;
layout (location = 0) in vec3 fragPosWorld[];
layout (location = 1) in vec3 fragNormalWorld[];
layout (location = 2) in vec2 fragTexCoord[];
layout(location = 3)in float instanceIndex[];
layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projView;
	vec4 cameraPos;
}

ubo;
layout(line_strip, max_vertices = 6) out;
void main(){
	gl_Position = gl_in[0].gl_Position;
	fragColor = vec4(0.0, 1.0, 0.0, 1.0);
	EmitVertex();
	gl_Position = gl_Position + ubo.projView * vec4(fragNormalWorld[0], 0.0);
	fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	EmitVertex();
	EndPrimitive();
}

