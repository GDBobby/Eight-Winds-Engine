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
	int octaves;
	float worldPosNoiseScaling;
    float sandHeight;
    float grassHeight;
} tbo;

vec2 SimplexHash(vec2 p) {// replace this by something better
	p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

//https://www.shadertoy.com/view/Msf3WH
float SimplexNoise(const vec2 p ) {
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;

	vec2  i = floor( p + (p.x+p.y)*K1 );
    vec2  a = p - i + (i.x+i.y)*K2;
    float m = step(a.y,a.x); 
    vec2  o = vec2(m,1.0-m);
    vec2  b = a - o + K2;
	vec2  c = a - 1.0 + 2.0*K2;
    vec3  h = max( 0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );
	vec3  n = h*h*h*h*vec3( dot(a,SimplexHash(i+0.0)), dot(b,SimplexHash(i+o)), dot(c,SimplexHash(i+1.0)));
    return dot( n, vec3(70.0) );
}

float NoiseWithOctaves(vec2 uv, int octaves){
	float freq = 0.25;
    mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );
	float f  = 0.5 * SimplexNoise( uv ); 
	uv = m * uv;
	for(int i = 1; i < octaves; i++){
		freq /= 2.0;
		f += freq * SimplexNoise(uv);
		uv = m * uv;
	}

	return f;
}

layout(vertices = 4) out;

layout(location = 0) in vec3 inNormal[];
layout(location = 1) in vec2 inUV[];
layout(location = 2) in vec3 inPos[];

layout(location = 0) out vec3 outNormal[4];
layout(location = 1) out vec2 outUV[4];
layout(location = 2) out vec3 outPos[4];

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
	//pos.y -= textureLod(samplerHeight, inUV[0], 0.0).r * tbo.displacementFactor;
	pos.y -= NoiseWithOctaves(inPos[0].xz, tbo.octaves) * tbo.displacementFactor;

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
	outPos[gl_InvocationID] = inPos[gl_InvocationID];
} 