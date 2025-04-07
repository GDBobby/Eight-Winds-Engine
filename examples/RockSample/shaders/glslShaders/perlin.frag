#version 450

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec2 fragTexCoord;

layout (location = 0) out vec4 outFragColor;

struct PointLight{
	vec4 position;
	vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projView;
	vec4 cameraPos;
} ubo;

layout(set = 0, binding = 1) uniform LightBufferObject {
	vec4 ambientColor;
	vec4 sunDirection;
	vec4 sunColor;
	PointLight pointLights[10];
	int numLights;
} lbo;



layout(set = 0, binding = 2) uniform PerlinBO{
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

void main(){
    const float perlin = PerlinAt(fragTexCoord.x, fragTexCoord.y);
    outFragColor = vec4(perlin, perlin, perlin, 1.0);
}