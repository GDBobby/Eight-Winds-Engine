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
} tbo;
layout(set = 0, binding = 3) uniform PerlinBO{
	int numbers[256];
} pbo;

float fade(const float t){
	return t * t * t * (t * (t * 6 - 15) + 10);
}
float grad(const int hash, const float x, const float y){
	const int h = hash & 15;
	const float u = (float(h < 8) * x) + (float(h >= 8) * y);
	const float v = (float(h < 4) * y) + (float(h >= 4) * float(h == 12 || h == 14) * x);

	const float signageU = float((h&1) == 0);
	const float signageV = float((h&2) == 0);

	return (u * (-1.0 + 2.0 * signageU)) + (v * (-1.0 + 2.0 * signageV));
}

float PerlinAt(const float ix, const float iy){
	const int X = int(floor(ix)) & 255;
	const int Y = int(floor(iy)) & 255;
	const float x = fract(ix);
	const float y = fract(iy);

	const float u = fade(x);
	const float v = fade(y);

	const int AA = pbo.numbers[(pbo.numbers[X] + Y) % 256];
	const int AB = pbo.numbers[(pbo.numbers[X] + Y + 1) % 256];
	const int BA = pbo.numbers[(pbo.numbers[X + 1] + Y) % 256];
	const int BB = pbo.numbers[(pbo.numbers[X + 1] + Y + 1) % 256];

	const float gradAA = grad(pbo.numbers[AA], x, y);
	const float gradBA = grad(pbo.numbers[BA], x - 1.0, y);
	const float gradAB = grad(pbo.numbers[AB], x, y - 1.0);
	const float gradBB = grad(pbo.numbers[BB], x - 1.0, y - 1.0);

	const float lerped0 = mix(gradAA, gradBA, u);
	const float lerped1 = mix(gradAB, gradBB, u);

	return mix(lerped0, lerped1, v);
}

layout (set = 0, binding = 3) uniform sampler2D samplerHeight;

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
	outHeight = textureLod(samplerHeight, outUV, 0.0).r;
	pos.y -= outHeight * tbo.displacementFactor;
	// Perspective projection
	gl_Position = ubo.projView * pos;

	//outWorldPos = pos.xyz;
}