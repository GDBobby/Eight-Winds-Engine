layout (location = 0) in vec3 fragPosWorld;
layout (location = 1) in vec3 fragNormalWorld;
layout (location = 2) in vec2 fragTexCoord;
layout (location = 3) in vec3 fragTangentWorld;
layout (location = 4) in float instanceIndex;
layout (location = 0) out vec4 outColor;
struct PointLight{
	vec4 position;
	vec4 color;
	}

;
layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projView;
	vec4 cameraPos;
	}

ubo;
layout(set = 0, binding = 1) uniform LightBufferObject {
	vec4 ambientColor;
	vec4 sunDirection;
	vec4 sunColor;
	PointLight pointLights[10];
	int numLights;
	}

lbo;
const float PI = 3.14159265359;
float DistributionGGX(const float NdotH, const float rough) {
	const float a2 = rough * rough;
	float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
	return a2 / (PI * denom * denom);
	}

vec3 FresnelShlick(const float VdotH, const float metallic, const vec3 albedo){
	const vec3 f0 = mix(vec3(0.04), vec3(albedo), metallic);
	return f0 + (1.0 - f0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
	}

float CombinedGeometrySmith(const float NdotL, const float NdotV, const float roughness){
	const float rp1 = roughness + 1.0;
	const float k = rp1 * rp1 / 8.0;
	const float g_v = NdotV / (NdotV * (1.0 - k) + k);
	const float g_l = NdotL / (NdotL * (1.0 - k) + k);
	return g_v * g_l;
	}

vec3 Pointlight_BRDF(const vec3 albedo, const float metal, const float rough, const vec3 normal, const vec3 view, const float NdotV) {
	vec3 Lo = vec3(0.0);
	for(int i = 0;
	 i < lbo.numLights;
	 i++){
	Ìvec3 lightDirUnNormalized = lbo.pointLights[i].position.xyz - fragPosWorld;
	Ìconst float invAttenuation = dot(lightDirUnNormalized, lightDirUnNormalized);
	Ìconst vec3 radiance = lbo.pointLights[i].color.rgb * lbo.pointLights[i].color.w / invAttenuation;
	Ìconst vec3 lightDir = lightDirUnNormalized / sqrt(invAttenuation);
	Ìconst vec3 halfAngle = normalize(view + lightDir);
	Ìconst float NdotH = max(dot(normal, halfAngle), 0.0);
	Ìconst float VdotH = max(dot(view, halfAngle), 0.0);
	Ìconst float NdotL = max(dot(normal, lightDir), 0.0);
	Ìconst vec3 fresnel = FresnelShlick(VdotH, metal, albedo);
	Ìconst float NDF = DistributionGGX(NdotH, rough);
	Ìconst float geo = CombinedGeometrySmith(NdotL, NdotV, rough);
	Ìconst float denom = 4.0 * NdotV * NdotL + 0.0001;
	Ìconst vec3 specular = NDF * geo * fresnel / denom;
	Ìconst vec3 kD = vec3(1.0) - fresnel;
	ÌLo += (kD * albedo / PI + specular) * radiance * NdotL;
	Ì}

	return Lo;
	}

vec3 Sun_BRDF(const vec3 albedo, const float metal, const float rough, const vec3 normal, const vec3 view, const float NdotV){
	const vec3 halfAngle = normalize(view + lbo.sunDirection.xyz);
	const float NdotL = max(dot(normal, lbo.sunDirection.xyz), 0.0);
	const float NdotH = max(dot(normal, halfAngle), 0.0);
	const float VdotH = max(dot(view, halfAngle), 0.0);
	const float NDF = DistributionGGX(NdotH, rough);
	const float geo = CombinedGeometrySmith(NdotL, NdotV, rough);
	const vec3 fresnel = FresnelShlick(VdotH, metal, albedo);
	const float denom = 4.0 * NdotV * NdotL + 0.0001;
	const vec3 specular = NDF * geo * fresnel / denom;
	const vec3 kD = vec3(1.0) - fresnel;
	return (kD * albedo / PI + specular) * lbo.sunColor.rgb * lbo.sunColor.w * NdotL;
	}

struct MaterialBuffer{
	vec3 albedo;
	float rough;
	float metal;
	}

;
layout(std430, set = 0, binding = 3) readonly buffer MaterialBufferObject{
	MaterialBuffer mbo[];
	}

;
layout (set = 0, binding = 4) uniform sampler2DArray materialTextures;
const int albedoIndex = 0;
const int normalIndex = 1;
vec3 calculateNormal() {
	vec3 tangentNormal = texture(materialTextures, vec3(fragTexCoord, normalIndex)).rgb * 2.0 - 1.0;
	const vec3 N = normalize(fragNormalWorld);
	const vec3 T = normalize(fragTangentWorld.xyz);
	const vec3 B = normalize(cross(N, T));
	const mat3 TBN = mat3(T, B, N);
	return normalize(TBN * tangentNormal);
	}

void main(){
	vec3 albedo = texture(materialTextures, vec3(fragTexCoord, albedoIndex)).rgb;
	const vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPosWorld);
	vec3 normal = calculateNormal();
	int instanceIndexInt = int(instanceIndex);
	float rough = mbo[instanceIndexInt].rough;
	float metal = mbo[instanceIndexInt].metal;
	const float NdotV = max(dot(normal, viewDir), 0.0);
	vec3 Lo = Pointlight_BRDF(albedo, metal, rough, normal, viewDir, NdotV);
	Lo += Sun_BRDF(albedo, metal, rough, normal, viewDir, NdotV);
	vec3 ambient = lbo.ambientColor.rgb * albedo;
	vec3 color = ambient + Lo;
	color /= (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));
	outColor = vec4(color, 1.0);
	}

