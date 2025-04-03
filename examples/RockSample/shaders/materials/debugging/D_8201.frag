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
vec4 sunlightDirection;
vec4 sunlightColor;
PointLight pointLights[10];
int numLights;
}
lbo;
const float PI = 3.14159265359;
float DistributionGGX (vec3 Normal, vec3 HalfAngle, float roughness) {
float a2 = roughness * roughness;
float NdotH = max(dot(Normal, HalfAngle), 0.0);
float denom = ((NdotH * a2 - NdotH) * NdotH + 1.0);
return a2 / (PI * denom * denom);
}
float Shlick(float NdotV, float NdotL, float roughness){
float k = roughness * roughness / 2;
float g_v = NdotV / (NdotV * (1 - k) + k);
float g_l = NdotL / (NdotL * (1 - k) + k);
return g_v * g_l;
}
vec3 FresnelSchlick (float cosTheta, vec3 F0) {
return F0 + (vec3(1.0) - F0) * pow (1.0 - cosTheta, 5.0);
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
vec3 viewDirection = normalize(ubo.cameraPos.xyz - fragPosWorld);
vec3 normal = calculateNormal();
int instanceIndexInt = int(instanceIndex);
float roughness = mbo[instanceIndexInt].rough;
float metal = mbo[instanceIndexInt].metal;
float NdotV = max(dot(normal, viewDirection), 0.0);
vec3 F0 = vec3(0.04);
F0 = mix(F0, albedo, metal);
vec3 Lo = vec3(0.0);
for(int i = 0;
 i < lbo.numLights;
 i++){
PointLight light = lbo.pointLights[i];
vec3 directionToLight = light.position.xyz - fragPosWorld;
float lightDistTan = dot(directionToLight, directionToLight);
float attenuation = 1.0 / (1.0 + 0.09 * lightDistTan + .032 * (lightDistTan * lightDistTan));
directionToLight = normalize(directionToLight);
vec3 radiance = light.color.rgb * attenuation * light.color.w;
vec3 HalfAngle = normalize(viewDirection + directionToLight);
float NDF = DistributionGGX(normal, HalfAngle, roughness);
float NdotL = max(dot(normal, directionToLight), 0.0);
float Geo = Shlick(NdotV, NdotL, roughness);
vec3 Fres = FresnelSchlick(max(dot(HalfAngle, viewDirection), 0.0), F0);
vec3 kD = vec3(1.0) - Fres;
kD *= 1.0 - metal;
vec3 specular = (NDF * Geo * Fres)  / (4.0 * NdotV * NdotL + .0001);
Lo += (kD * albedo / PI + specular) * radiance * NdotL;
}
vec3 sunDir = lbo.sunlightDirection.xyz;
vec3 sunHalfAngle = normalize(viewDirection + sunDir);
float sunNDF = DistributionGGX(normal, sunHalfAngle, roughness);
float sunNdotL = max(dot(normal, sunDir), 0.0);
float sunGeo = Shlick(NdotV, sunNdotL, roughness);
vec3 sunFres = FresnelSchlick(max(dot(sunHalfAngle, viewDirection), 0.0), F0);
vec3 sunKd = (vec3(1.0) - sunFres) * 1.0 - metal;
vec3 sunSpecular = (sunNDF * sunGeo * sunFres) / (4.0 * NdotV * sunNdotL + .0001);
Lo += (sunKd * albedo / PI + sunSpecular) * lbo.sunlightColor.rgb * lbo.sunlightColor.w * sunNdotL;
vec3 ambient = lbo.ambientColor.rgb * albedo;
vec3 color = ambient + Lo;
color /= (color + vec3(1.0));
color = pow(color, vec3(1.0/2.2));
outColor = vec4(color, 1.0);
}
