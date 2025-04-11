#pragma once

#include <string>
#include <vector>
#include <array>

namespace FragmentShaderText {
	const std::string version = { "#version 450 \n" };

	const std::vector<std::string> fragNNEntry = {
		"layout (location = 0) in vec3 fragPosWorld;",
		"layout (location = 1) in vec3 fragNormalWorld;",
		"layout (location = 2) in vec2 fragTexCoord;",
	};
	const std::vector<std::string> fragBumpEntry = {
		"layout(location = 0) in vec3 tangentFragPos;",
		"layout(location = 1) in vec3 tangentSunDir;",
		"layout(location = 2) in vec2 fragTexCoord;",
		"layout(location = 3) in vec3 tangentViewPos;"
	};

	const std::string fragExit = "layout (location = 0) out vec4 outColor;";

	const std::vector<std::string> dataBindings = {
		{"struct PointLight{vec4 position;vec4 color;};"},

		{"layout(set = 0, binding = 0) uniform GlobalUbo {"},
		{"mat4 projView;vec4 cameraPos;"},
		{"}ubo;"},

		{"layout(set = 0, binding = 1) uniform LightBufferObject {"},
		{"vec4 ambientColor;vec4 sunDirection;vec4 sunColor;PointLight pointLights[10];int numLights;"},
		{"}lbo;"},

		{"const float PI = 3.14159265359;"},
	};

	const std::vector<std::string> DistributionGGXFuncBlock{
		"float DistributionGGX(const float NdotH, const float rough) {",
		"const float a2 = rough * rough;",
		"const float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;",
		"return a2 / (PI * denom * denom);}"
	};
	const std::vector<std::string> FresnelShlickFuncBlock{
		"vec3 FresnelShlick(const float VdotH, const float metallic, const vec3 albedo){",
		"const vec3 baseReflectivity = mix(vec3(0.04), vec3(albedo), metallic);",
		"return baseReflectivity + (1.0 - baseReflectivity) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);}",
	};
	const std::vector<std::string> CombinedGeoFuncBlock{
		"float CombinedGeometrySmith(const float NdotL, const float NdotV, const float roughness){",
		"const float rp1 = roughness + 1.0;",
		"const float k = rp1 * rp1 / 8.0;",
		"const float g_v = NdotV / (NdotV * (1.0 - k) + k);",
		"const float g_l = NdotL / (NdotL * (1.0 - k) + k);",
		"return g_v * g_l;}",
	};

	const std::vector<std::string> PointLight_BRDF_FuncBlock{
		"vec3 Pointlight_BRDF(const vec3 albedo, const float metal, const float rough, const vec3 normal, const vec3 view, const float NdotV) {",
		"vec3 Lo = vec3(0.0);",
		"for(int i = 0; i < lbo.numLights; i++){",
    	"vec3 lightDirUnNormalized = lbo.pointLights[i].position.xyz - fragPosWorld;",
		"const float invAttenuation = dot(lightDirUnNormalized, lightDirUnNormalized);",
		//float attenuation = 1.0 / (1.0 + 0.09 * lightDistTan + .032 * (lightDistTan * lightDistTan));
		//vec3 radiance = light.color.rgb * attenuation * light.color.w
		"const vec3 radiance = lbo.pointLights[i].color.rgb * lbo.pointLights[i].color.w / invAttenuation;",
		"const vec3 lightDir = lightDirUnNormalized / sqrt(invAttenuation);",
		"const vec3 halfAngle = normalize(view + lightDir);",

		"const float NdotH = max(dot(normal, halfAngle), 0.0);",
		"const float VdotH = max(dot(view, halfAngle), 0.0);",
		"const float NdotL = max(dot(normal, lightDir), 0.0);",

		"const vec3 fresnel = FresnelShlick(VdotH, metal, albedo);",
		"const float NDF = DistributionGGX(NdotH, rough);",
		"const float geo = CombinedGeometrySmith(NdotL, NdotV, rough);",

		//its worth noting, NdotV and NdotL are both in this denominator, and in the numerator via combined_geometry_smith
		//they combine with the 0.0001 in the denominator, but removing it shouldn't have a large impact. worth testing
		"const float denom = 4.0 * NdotV * NdotL + 0.0001;",
		"const vec3 specular = NDF * geo * fresnel / denom;",
		"const vec3 kD = vec3(1.0) - fresnel;",
		"Lo += (kD * albedo / PI + specular) * radiance * NdotL;",
		"}return Lo;}",
	};

	const std::vector<std::string> Spotlight_BRDF_FuncBlock{

	};

	const std::vector<std::string> Sun_BRDF_FuncBlock{

		//"vec3 spotlight_brdf"

		//"vec3 directional_brdf"

		//i feel like a different name would be more appropriate. heavenly light, planetary light, idk. its an infinitely distance light. it could also be the moon
		"vec3 Sun_BRDF(const vec3 albedo, const float metal, const float rough, const vec3 normal, const vec3 view, const float NdotV){",
		"const vec3 halfAngle = normalize(view + lbo.sunDirection.xyz);", //sundir needs to be normalized
		"const float NdotL = max(dot(normal, lbo.sunDirection.xyz), 0.0);",
		"const float NdotH = max(dot(normal, halfAngle), 0.0);",
		"const float VdotH = max(dot(view, halfAngle), 0.0);",
		"const float NDF = DistributionGGX(NdotH, rough);",
		"const float geo = CombinedGeometrySmith(NdotL, NdotV, rough);",
		"const vec3 fresnel = FresnelShlick(VdotH, metal, albedo);",
		"const float denom = 4.0 * NdotV * NdotL + 0.0001;",
		"const vec3 specular = NDF * geo * fresnel / denom;",
		"const vec3 kD = vec3(1.0) - fresnel;",
		"return (kD * albedo / PI + specular) * lbo.sunColor.rgb * lbo.sunColor.w * NdotL;}"	

	};
	//theres a few other light types, but i wont get into that

	inline void AddLighting(std::string& retBuf){
		for(auto const& str : DistributionGGXFuncBlock){
			retBuf += str;
		}
		for(auto const& str : FresnelShlickFuncBlock){
			retBuf += str;
		}
		for(auto const& str : CombinedGeoFuncBlock){
			retBuf += str;
		}
		for(auto const& str : PointLight_BRDF_FuncBlock){
			retBuf += str;
		}
		for(auto const& str : Sun_BRDF_FuncBlock){
			retBuf += str;
		}
	}

	//first index is no bones, second index is with bones

	const std::string materialBufferInstancedPartOne = "struct MaterialBuffer{vec3 albedo;float rough;float metal;};";
	const std::string materialBufferInstancedPartTwo = "layout(std430, set = 0, binding = ";
	const std::string materialBufferInstancedPartThree = ") readonly buffer MaterialBufferObject{MaterialBuffer mbo[];};";

	const std::string firstHalfBinding =  "layout (set = 0, binding = ";
	const std::string secondHalfBinding = { ") uniform sampler2DArray materialTextures;" };
	std::vector<std::string> MBOSecondHalf = {
		{") uniform MaterialBufferObject{"},
		{"vec3 albedo;float rough;float metal;" },
		{"}mbo;"}
	};

	const std::vector<std::string> calcNormalFunction = {
		"vec3 calculateNormal() {",
		"vec3 tangentNormal = texture(materialTextures, vec3(fragTexCoord, normalIndex)).rgb * 2.0 - 1.0;",
		"const vec3 N = normalize(fragNormalWorld);const vec3 T = normalize(fragTangentWorld.xyz);const vec3 B = normalize(cross(N, T));const mat3 TBN = mat3(T, B, N);",
		"return normalize(TBN * tangentNormal);}"
	};
	const std::vector<std::string> parallaxMapping = {
		"vec2 parallaxMapping(vec2 uv, vec3 viewDir) {",
		"float height = 1.0 - textureLod(materialTextures, vec3(uv, bumpIndex), 0.0).a;",
		"vec2 p = viewDir.xy * ((height * 0.005) - .01f) / viewDir.z;", //height * ubo.heightScale * .5
		"return uv - p;}",
	};


	const std::vector<std::string> mainEntryBlock[2] = {
		{
			"void main(){",
		},
		{
			"void main(){",			
			"vec3 viewDir = normalize(tangentViewPos - tangentFragPos);"
			"vec3 normal = normalize(textureLod(materialTextures, vec3(fragTexCoord, normalIndex), 0.0).rgb * 2.0 - 1.0);"
			//"vec3 albedo = texture(materialTextures, vec3(fragTexCoord, albedoIndex)).rgb;",
		}
	};

	const std::vector<std::string> mainSecondBlockNN = {
		//"vec3 cameraPosWorld = ubo.inverseView[3].xyz;",
		"const vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPosWorld);",
	};

	const std::vector<std::string> mainThirdBlock = {
		"const float NdotV = max(dot(normal, viewDir), 0.0);",
		//"vec3 FresnelBase = mix(vec3(0.04), albedo, metal);", //need to add this in later
		"vec3 Lo = Pointlight_BRDF(albedo, metal, rough, normal, viewDir, NdotV);",
		"Lo += Sun_BRDF(albedo, metal, rough, normal, viewDir, NdotV);"
//"vec3 Pointlight_BRDF(const vec3 albedo, const float metal, const float rough, const vec3 normal, const vec3 view, const float NdotV) {",
		
		
	};
}