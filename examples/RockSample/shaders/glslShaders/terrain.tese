#version 450

//https://github.com/SaschaWillems/Vulkan/blob/master/shaders/glsl/terraintessellation/terrain.tese

layout(set = 0, binding = 0) uniform GlobalUbo{
	mat4 projView;
	vec4 cameraPos;
} ubo;

layout(set = 0, binding = 2) uniform TescBO{
    mat4 projection;
    mat4 view;
    vec4 frustumPlanes[6];
    vec2 viewportDim;
    float displacementFactor;
    float tessFactor;
    float tessEdgeSize;
	int octaves;
} tbo;

vec2 SimplexHash(vec2 p ) { // replace this by something better {
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

layout(quads, equal_spacing, cw) in;

layout (location = 0) in vec3 inNormal[];
layout (location = 1) in vec2 inUV[];
 
layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out float outHeight;

void main() {
	// Interpolate UV coordinates
	vec2 uv1 = mix(inUV[0], inUV[1], gl_TessCoord.x);
	vec2 uv2 = mix(inUV[3], inUV[2], gl_TessCoord.x);
	outUV = mix(uv1, uv2, gl_TessCoord.y);

	vec3 n1 = mix(inNormal[0], inNormal[1], gl_TessCoord.x);
	vec3 n2 = mix(inNormal[3], inNormal[2], gl_TessCoord.x);
	outNormal = mix(n1, n2, gl_TessCoord.y);

	// Interpolate positions
	vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);
	// Displace
	//outHeight = textureLod(samplerHeight, outUV, 0.0).r;
	outHeight = NoiseWithOctaves(outUV, tbo.octaves);
	pos.y -= outHeight * tbo.displacementFactor;
	// Perspective projection
	gl_Position = ubo.projView * pos;

	//outWorldPos = pos.xyz;
}