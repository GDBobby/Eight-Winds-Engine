#version 450

//https://github.com/SaschaWillems/Vulkan/blob/master/shaders/glsl/terraintessellation/terrain.tesc


layout(set = 0, binding = 2) uniform TescBO{
    mat4 projection;
    mat4 view;
    vec4 frustumPlanes[6];
    vec2 viewportDim;
    float displacementFactor;
    float tessFactor;
    float tessEdgeSize;
} tbo;

layout(set = 0, binding = 3) uniform sampler2D samplerHeight;

layout(vertices = 4) out;

layout(location = 0) in vec3 inNormal[];
layout(location = 1) in vec2 inUV[];

layout(location = 0) out vec3 outNormal[4];
layout(location = 1) out vec2 outUV[4];

// Calculate the tessellation factor based on screen space
// dimensions of the edge
float screenSpaceTessFactor(const vec4 p0, const vec4 p1) {
	// Calculate edge mid point
	const vec4 midPoint = 0.5 * (p0 + p1);
	// Sphere radius as distance between the control points
	const float radius = distance(p0, p1) / 2.0;

	// View space
	const vec4 v0 = tbo.view * midPoint;

	// Project into clip space
	vec4 clip0 = (tbo.projection * (v0 - vec4(radius, vec3(0.0))));
	vec4 clip1 = (tbo.projection * (v0 + vec4(radius, vec3(0.0))));

	// Get normalized device coordinates
	clip0 /= clip0.w;
	clip1 /= clip1.w;

	// Convert to viewport coordinates
	clip0.xy *= tbo.viewportDim;
	clip1.xy *= tbo.viewportDim;
	
	// Return the tessellation factor based on the screen size 
	// given by the distance of the two edge control points in screen space
	// and a reference (min.) tessellation size for the edge set by the application
	return clamp(distance(clip0, clip1) / tbo.tessEdgeSize * tbo.tessFactor, 1.0, 64.0);
}

// Checks the current's patch visibility against the frustum using a sphere check
// Sphere radius is given by the patch size
bool frustumCheck() {
	// Fixed radius (increase if patch size is increased in example)
	const float radius = 8.0f;
	vec4 pos = gl_in[gl_InvocationID].gl_Position;
	pos.y -= textureLod(samplerHeight, inUV[0], 0.0).r * tbo.displacementFactor;

	// Check sphere against frustum planes
	for (int i = 0; i < 6; i++) {
		if ((dot(pos, tbo.frustumPlanes[i]) + radius) < 0.0) {
			return false;
		}
	}
	return true;
}


void main() {
	if (gl_InvocationID == 0) {
		if (!frustumCheck()) {
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelInner[1] = 0.0;

			gl_TessLevelOuter[0] = 0.0;
			
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			gl_TessLevelOuter[3] = 0.0;
		}
		else {
			if (tbo.tessFactor > 0.0) {
				gl_TessLevelOuter[0] = screenSpaceTessFactor(gl_in[3].gl_Position, gl_in[0].gl_Position);
				gl_TessLevelOuter[1] = screenSpaceTessFactor(gl_in[0].gl_Position, gl_in[1].gl_Position);
				gl_TessLevelOuter[2] = screenSpaceTessFactor(gl_in[1].gl_Position, gl_in[2].gl_Position);
				gl_TessLevelOuter[3] = screenSpaceTessFactor(gl_in[2].gl_Position, gl_in[3].gl_Position);

				gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5);
				gl_TessLevelInner[1] = mix(gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5);
			}
			else {
				// Tessellation factor can be set to zero by example
				// to demonstrate a simple passthrough
				gl_TessLevelInner[0] = 1.0;
				gl_TessLevelInner[1] = 1.0;

				gl_TessLevelOuter[0] = 1.0;
				gl_TessLevelOuter[1] = 1.0;
				gl_TessLevelOuter[2] = 1.0;
				gl_TessLevelOuter[3] = 1.0;
			}
		}

	}

	gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
	outNormal[gl_InvocationID] = inNormal[gl_InvocationID];
	outUV[gl_InvocationID] = inUV[gl_InvocationID];
} 