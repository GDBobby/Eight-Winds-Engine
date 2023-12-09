#pragma once

#include <string>
#include <vector>

std::string version = { "#version 450 \n" };
/*
std::vector<std::string> fragTangentEntry = {
	{"layout (location = 0) in vec2 fragTexCoord;"},
	{"layout(location = 1) in vec3 TangentViewPos; "},
	{"layout(location = 2) in vec3 TangentFragPos; "},
	{"layout(location = 3) in mat3 TBN; "},
	{"layout (location = 0) out vec4 outColor;"},
};
*/
std::vector<std::string> fragNNEntry = {
	"layout (location = 0) in vec3 fragPosWorld;",
	"layout (location = 1) in vec3 fragNormalWorld;",
	"layout (location = 2) in vec2 fragTexCoord;",
};
std::vector<std::string> fragBumpEntry = {
	"layout(location = 0) in vec3 tangentFragPos;",
	"layout(location = 1) in vec3 tangentSunDir;",
	"layout(location = 2) in vec2 fragTexCoord;",
	"layout(location = 3) in vec3 tangentViewPos;"
};

std::string fragExit = "layout (location = 0) out vec4 outColor;";


std::vector<std::string> dataBindings = {
	{"struct PointLight{vec4 position;vec4 color;};"},

	{"layout(set = 0, binding = 0) uniform GlobalUbo {"},
	{"mat4 projection;mat4 view;vec4 cameraPos;"},
	{"}ubo;"},

	{"layout(set = 0, binding = 1) uniform LightBufferObject {"},
	{"vec4 ambientColor;vec4 sunlightDirection;vec4 sunlightColor;PointLight pointLights[10];int numLights;"},
	{"}lbo;"},

	{"const float PI = 3.14159265359;"},
};
std::vector<std::string> functionBlock = {
	{"float DistributionGGX (vec3 Normal, vec3 HalfAngle, float roughness) {"},
	{"float a2 = roughness * roughness;"},
	{"float NdotH = max(dot(Normal, HalfAngle), 0.0);"},
	{"float denom = ((NdotH * a2 - NdotH) * NdotH + 1.0);"},
	//{"float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);"},
	{"return a2 / (PI * denom * denom);}"},
	/*
	{"float GeometrySchlickGGX (float NdotV, float roughness) {"},
	{"float r = (roughness + 1.0);"},
	{"float k = (r * r) / 8.0;"},
	{"return NdotV / NdotV * (1.0 - k) + k;}"},

	{"float GeometrySmith (vec3 Normal, vec3 ViewDir, vec3 LightDir, float roughness) {"},
	{"return GeometrySchlickGGX (max (dot (Normal, ViewDir), 0.0), roughness) *"},
	{"GeometrySchlickGGX (max (dot (Normal, LightDir), 0.0), roughness); }"},
	*/

	{"float Shlick(float NdotV, float NdotL, float roughness){"},
	{"float k = roughness * roughness / 2;"},
	{"float g_v = NdotV / (NdotV * (1 - k) + k);"},
	{"float g_l = NdotL / (NdotL * (1 - k) + k);"},
	{"return g_v * g_l;}"},

	{"vec3 FresnelSchlick (float cosTheta, vec3 F0) {"},
	{"return F0 + (vec3(1.0) - F0) * pow (1.0 - cosTheta, 5.0); }"}
};


//first index is no bones, second index is with bones
std::string albedoBinding[2] = {
	{"layout(set = 1, binding = 0) uniform sampler2D albedoSampler;"},
	{"layout(set = 2, binding = 0) uniform sampler2D albedoSampler;"}
};

std::string normalBinding[2] = {
	{"layout(set = 1, binding = 1) uniform sampler2D normalSampler;"},
	{"layout(set = 2, binding = 1) uniform sampler2D normalSampler;"}
};

std::string firstHalfBinding[2] = {
	{"layout (set = 1, binding = "},
	{"layout (set = 2, binding = "},
};
std::string secondHalfBinding = { ") uniform sampler2D " };

std::vector<std::string> calcNormalFunction = {
	"vec3 calculateNormal() {",
	"vec3 tangentNormal = texture(normalSampler, fragTexCoord).rgb * 2.0 - 1.0;",
	"vec3 N = normalize(fragNormalWorld);vec3 T = normalize(fragTangentWorld.xyz);vec3 B = normalize(cross(N, T));mat3 TBN = mat3(T, B, N);",
	"return normalize(TBN * tangentNormal);}"
};
std::vector<std::string> parallaxMapping = {
	"vec2 parallaxMapping(vec2 uv, vec3 viewDir) {",
	"float height = 1.0 - textureLod(bumpSampler, uv, 0.0).a;",
	"vec2 p = viewDir.xy * ((height * 0.005) - .01f) / viewDir.z;", //height * ubo.heightScale * .5
	"return uv - p;}",
};


std::vector<std::string> mainEntryBlock[2] = {
	{
		"void main(){",
		"vec3 albedo = texture(albedoSampler, fragTexCoord).rgb;",
	},
	{
		"void main(){",
		"vec3 viewDirection = normalize(tangentViewPos - tangentFragPos);"
		//"vec2 parallaxUV = parallaxMapping(fragTexCoord, viewDirection);",
		//"if (parallaxUV.x < 0.0 || parallaxUV.x > 1.0 || parallaxUV.y < 0.0 || parallaxUV.y > 1.0) { discard; }",
		//"vec3 normal = nromalize(textureLod(normalSampler, parallaxUV, 0.0).rgb * 2.0 - 1.0);
		"vec3 normal = normalize(textureLod(normalSampler, fragTexCoord, 0.0).rgb * 2.0 - 1.0);"
		"vec3 albedo = texture(albedoSampler, fragTexCoord).rgb;",
	}
};

std::vector<std::string> mainSecondBlockNN = {
	//"vec3 cameraPosWorld = ubo.inverseView[3].xyz;",
	"vec3 viewDirection = normalize(ubo.cameraPos.xyz - fragPosWorld);",
};

std::vector<std::string> mainThirdBlock = {
	"float NdotV = max(dot(normal, viewDirection), 0.0);",
	"vec3 F0 = vec3(0.04);",
	"F0 = mix(F0, albedo, metal);",
	"vec3 Lo = vec3(0.0);",
};
std::vector<std::string> pointLightLoop = {
	"for(int i = 0; i < lbo.numLights; i++){",
	"PointLight light = lbo.pointLights[i];",
	"vec3 directionToLight = light.position.xyz - fragPosWorld;",
	"float lightDistTan = dot(directionToLight, directionToLight);",
	"float attenuation = 1.0 / (1.0 + 0.09 * lightDistTan + .032 * (lightDistTan * lightDistTan));",
	"directionToLight = normalize(directionToLight);",
	"vec3 radiance = light.color.rgb * attenuation * light.color.w;",
	"vec3 HalfAngle = normalize(viewDirection + directionToLight);",
	"float NDF = DistributionGGX(normal, HalfAngle, roughness);",

	"float NdotL = max(dot(normal, directionToLight), 0.0);",

	"float Geo = Shlick(NdotV, NdotL, roughness);",
	"vec3 Fres = FresnelSchlick(max(dot(HalfAngle, viewDirection), 0.0), F0);",
	"vec3 kD = vec3(1.0) - Fres;",
	"kD *= 1.0 - metal;",
	"vec3 specular = (NDF * Geo * Fres)  / (4.0 * NdotV * NdotL + .0001);",
	"Lo += (kD * albedo / PI + specular) * radiance * NdotL;}",
};


std::vector<std::string> sunCalculation = {
	"vec3 sunDir = lbo.sunlightDirection.xyz;",
	"vec3 sunHalfAngle = normalize(viewDirection + sunDir);",

	"float sunNDF = DistributionGGX(normal, sunHalfAngle, roughness);",
	"float sunNdotL = max(dot(normal, sunDir), 0.0);",
	"float sunGeo = Shlick(NdotV, sunNdotL, roughness);",

	"vec3 sunFres = FresnelSchlick(max(dot(sunHalfAngle, viewDirection), 0.0), F0);",

	"vec3 sunKd = (vec3(1.0) - sunFres) * 1.0 - metal;",
	"vec3 sunSpecular = (sunNDF * sunGeo * sunFres) / (4.0 * NdotV * sunNdotL + .0001);",
	"Lo += (sunKd * albedo / PI + sunSpecular) * lbo.sunlightColor.rgb * lbo.sunlightColor.w * sunNdotL;",
};
std::vector<std::string> bumpSunCalculation = {
	"vec3 sunHalfAngle = normalize(viewDirection + tangentSunDir);"
	"float sunNDF = DistributionGGX(normal, sunHalfAngle, roughness);",
	"float sunNdotL = max(dot(normal, tangentSunDir), 0.0);",
	"float sunGeo = Shlick(NdotV, sunNdotL, roughness);",


	"vec3 sunFres = FresnelSchlick(max(dot(sunHalfAngle, viewDirection), 0.0), F0);",
	"vec3 sunKd = (vec3(1.0) - sunFres) * 1.0 - metal;",
	"vec3 sunSpecular = (sunNDF * sunGeo * sunFres) / (4.0 * NdotV * sunNdotL + .0001);",
	"Lo += (sunKd * albedo / PI + sunSpecular) * lbo.sunlightColor.rgb * lbo.sunlightColor.w * sunNdotL;",
};





// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ VERTEX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

std::vector<std::string> vertexTangentInput = {
	"layout(location = 0) in vec3 position;",
	"layout(location = 1) in vec3 normal;",
	"layout(location = 2) in vec2 uv;",
	"layout(location = 3) in vec3 tangent;",
	"layout(location = 4) in ivec4 boneIds;",
	"layout(location = 5) in vec4 weights;",
};
std::vector<std::string> vertexNNInput = {
	"layout(location = 0) in vec3 position;",
	"layout(location = 1) in vec3 normal;",
	"layout(location = 2) in vec2 uv;",
	"layout(location = 3) in ivec4 boneIds;",
	"layout(location = 4) in vec4 weights;",
};

std::vector<std::string> vertexTangentOutput = {
	"layout(location = 0) out vec3 fragPosWorld;",
	"layout(location = 1) out vec3 fragNormalWorld;",
	"layout(location = 2) out vec2 fragTexCoord;",
	"layout(location = 3) out vec3 fragTangentWorld;",
};
std::vector<std::string> vertexNNOutput = {
	"layout(location = 0) out vec3 fragPosWorld;",
	"layout(location = 1) out vec3 fragNormalWorld;",
	"layout(location = 2) out vec2 fragTexCoord;",
	"layout(location = 3) out vec3 fragTangentWorld;",
};

std::vector<std::string> vertexEntry = {
	"const int MAX_BONE_INFLUENCE = 4;",

	"struct PointLight{vec4 position;vec4 color;};",

	"layout(set = 0, binding = 0) uniform GlobalUbo {",
	"mat4 projection;mat4 view;vec4 cameraPos;} ubo;",

	"layout(set = 0, binding = 1) uniform GPUSceneData {",
	"vec4 ambientColor;vec4 sunlightDirection;vec4 sunlightColor;",
	"PointLight pointLights[10];int numLights;",
	"} gpuScene;",
};
//when ready to swap out pipelines for smaller actor counts
std::vector<std::string> vertexSmallInstanceBuffersFirstHalf = {
	"layout(set = 1, binding = 0) uniform ModelMatrices { mat4 modelMatrix[]; }; ",
	"layout (set = 1, binding = 1) readonly buffer JointMatrices {mat4 finalBonesMatrices[];};"
	"void main(){int boneIndex = gl_InstanceIndex *",
};

std::vector<std::string> vertexInstanceBuffersFirstHalf = {
	"layout(set = 1, binding = 0) readonly buffer ModelMatrices { mat4 modelMatrix[]; }; ",
	"layout (set = 1, binding = 1) readonly buffer JointMatrices {mat4 finalBonesMatrices[];};"
	"void main(){int boneIndex = gl_InstanceIndex *",
};
std::vector<std::string> vertexInstanceBuffersSecondHalf = {
	"mat4 skinMat = finalBonesMatrices[boneIds[0] + boneIndex] * weights[0] + finalBonesMatrices[boneIds[1] + boneIndex] * weights[1]",
	"+ finalBonesMatrices[boneIds[2] + boneIndex] * weights[2] + finalBonesMatrices[boneIds[3] + boneIndex] * weights[3]; ",
	"vec4 positionWorld = modelMatrix[gl_InstanceIndex] * skinMat * vec4(position, 1.0f);",
	"gl_Position = ubo.projection * ubo.view * positionWorld;"
};

std::vector<std::string> vertexNoInstanceBuffers = {
	"layout (set = 1, binding = 0) uniform JointMatrices {mat4 finalBonesMatrices[];};"
	"layout(push_constant) uniform Push {mat4 modelMatrix;int index_boneCount;} push; ",
	"void main(){"
	"mat4 skinMat = finalBonesMatrices[boneIds[0] + push.index_boneCount] * weights[0] + finalBonesMatrices[boneIds[1] + push.index_boneCount] * weights[1]",
	"+ finalBonesMatrices[boneIds[2] + push.index_boneCount] * weights[2] + finalBonesMatrices[boneIds[3] + push.index_boneCount] * weights[3];",
	"vec4 positionWorld = push.modelMatrix * skinMat * vec4(position, 1.0f);",
	"gl_Position = ubo.projection * ubo.view * positionWorld;",
};

std::vector<std::string> vertexTangentInstancingMainExit = {
	"fragNormalWorld = normalize(transpose(inverse(mat3(ubo.view * modelMatrix[gl_InstanceIndex] * skinMat))) * normal);",
	"fragTangentWorld = normalize(transpose(inverse(mat3(ubo.view * modelMatrix[gl_InstanceIndex] * skinMat))) * tangent);",

	"fragPosWorld = positionWorld.xyz;fragTexCoord = uv;}"
};

std::vector<std::string> vertexTangentNoInstancingMainExit = {
	"fragNormalWorld = normalize(transpose(inverse(mat3(ubo.view * push.modelMatrix * skinMat))) * normal);",
	"fragTangentWorld = normalize(transpose(inverse(mat3(ubo.view * push.modelMatrix * skinMat))) * tangent);",

	"fragPosWorld = positionWorld.xyz;fragTexCoord = uv;}",
};

std::vector<std::string> vertexNNInstancingMainExit = {
	"fragNormalWorld = normalize(transpose(inverse(mat3(ubo.view * modelMatrix[gl_InstanceIndex] * skinMat))) * normal);",
	"fragPosWorld = positionWorld.xyz; fragTexCoord = uv;}",
};


std::vector<std::string> vertexNNNoInstancingMainExit = {
	"fragNormalWorld = normalize(transpose(inverse(mat3(ubo.view * push.modelMatrix * skinMat))) * normal);",
	"fragPosWorld = positionWorld.xyz; fragTexCoord = uv;}",
};
