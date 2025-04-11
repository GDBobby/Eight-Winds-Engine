
static const float M_2_SQRT_PI = 3.5449077018110320545963349666823;

#include "Ocean_Render_Inputs.h.slang"
#include "oceanography.h.slang"
#include "fragment_constants.h.slang"
#include "Foam_Functions.slang"


//https://github.com/ebruneton/precomputed_atmospheric_scattering
float Bruneton_MeanFresnel(const float3 view, const float3 normal, const float2 sigmaSq) {
	const float2 v = view.xz;
	const float2 t = v * v / (1.0 - view.y * view.y);
	const float sigmaV = sqrt(dot(t, sigmaSq));
	const float cosThetaV = dot(view, normal);
    return pow(abs(1.0 - cosThetaV), 5.0 * exp(-2.69 * sigmaV)) 
		/ (1.0 + 22.7 * pow(abs(sigmaV), 1.5));
}

float Bruneton_Fresnel(const float3 viewDir, const float3 normal, const float2 slopeVarianceSqrd) {
    const float R = 0.5;
    const float fresnel = R + (1 - R)
		* Bruneton_MeanFresnel(viewDir, normal, slopeVarianceSqrd);
    return saturate(fresnel);
}

float Bruneton_erfc(float x) {
    return 2.0 * exp(-x * x) / (2.319 * x + sqrt(4.0 + 1.52 * x * x));
}

float Bruneton_Lambda(float cosTheta, float sigmaSq) {
    float v = cosTheta / sqrt((1.0 - cosTheta * cosTheta) * 2.0 * sigmaSq);
    float vSqrd = v * v;
    float denom = 1.0 / (2.319 * v + sqrt(4.0 + 1.52 * vSqrd));
    return max(0.0, ((1.0 / (v * M_2_SQRT_PI)) - denom) * exp(-vSqrd));

    // return max(0.0, (exp(-v * v) - v * M_SQRT_PI * Bruneton_erfc(v)) / (2.0 * v * M_SQRT_PI));
}

float Bruneton_Lambda2(const float3 direction, const float zenith, const float3 tangentX, const float3 tangentY, const float2 sigmaSq)
{
    const float tanDirection = atan2(dot(direction, tangentY), dot(direction, tangentX));
    const float cosTanDir = 1.0 / (1.0 + tanDirection * tanDirection);
    const float sigma = sigmaSq.x * cosTanDir + sigmaSq.y * (1.0 - cosTanDir);

    const float v = zenith / sqrt((1.0 - zenith * zenith) * 2.0 * sigma);
    const float vSqrd = v * v;
    const float denom = 1.0 / (2.319 * v + sqrt(4.0 + 1.52 * vSqrd));
    return max(0.0, ((1.0 / (v * M_2_SQRT_PI)) - denom) * exp(-vSqrd));
}

float Bruneton_reflectedSunRadiance(const WorldSpaceData ws, const float2 slopeVarianceSqrd, const float3 tangentX, const float3 tangentY) {
    float2 sigmaSq;
    sigmaSq.x = max(1e-4, slopeVarianceSqrd.x + SPECULAR_MIN_ROUGHNESS * 0.2);
    sigmaSq.y = max(1e-4, slopeVarianceSqrd.y + SPECULAR_MIN_ROUGHNESS * 0.2);

    float3 halfAngle = normalize(sunDirection.xyz + ws.viewDir);
    const float NDotH = max(0.001, dot(halfAngle, ws.normal));

    float tangentZenithX = dot(halfAngle, tangentX) / NDotH;
    float tangentZenithY = dot(halfAngle, tangentY) / NDotH;
    float halfZenith = dot(halfAngle, ws.normal);

    float p = exp(-0.5 * (tangentZenithX * tangentZenithX / sigmaSq.x 
		+ tangentZenithY * tangentZenithY / sigmaSq.y))
		/ (M_2_PI * sqrt(sigmaSq.x * sigmaSq.y));

    float VdotN = max(dot(ws.viewDir, ws.normal), 0.01);
    float viewBrunetonLambda = Bruneton_Lambda2(ws.viewDir, VdotN, tangentX, tangentY, sigmaSq);
    // light zenith is only calculated outside of the function because i need view zenith outside of the function
    float LdotN = max(dot(sunDirection.rgb, ws.normal), 0.01);
    float lightBrunetonLambda = Bruneton_Lambda2(sunDirection.rgb, LdotN, tangentX, tangentY, sigmaSq);

    float fresnel = 0.02 + 0.98 * pow(1.0 - dot(ws.viewDir, halfAngle), 5.0);

    return fresnel * p / (
	(1.0 + lightBrunetonLambda + viewBrunetonLambda) 
	* VdotN * halfZenith * halfZenith * halfZenith * halfZenith * 4.0);
}

float3 Bruneton_Reflection(const WorldSpaceData ws) {
    // return Brune_SkyRadiance(transpose(oceanData.worldToWindSpace), brune);

    float3 reflectedDir = reflect(-ws.viewDir, ws.normal);

    // im not entirely sure why, but this prevents the reflection from showing the bottom of the skymap
    // might be some blending that im missing
    // still doesnt look great with this
    reflectedDir.y = reflectedDir.y * 0.5 + 0.5;

    return skyTexture.SampleLevel(reflectedDir, 0.0).rgb;
}

float3 Bruneton_Specular(const WorldSpaceData ws, const float2 slopeVarianceSqrd, const float3 tangentX, const float3 tangentY) {
    // float radiance = mainLight.shadowAttenuation * Bruneton_reflectSunRadiance(ws);
    // return radiance * SPECULAR_STRENGTH * sunColor.rgb;
    return Bruneton_reflectedSunRadiance(ws, slopeVarianceSqrd, tangentX, tangentY) * SPECULAR_STRENGTH * sunColor.rgb;
}

float2 SubsurfaceScatteringFactor(const WorldSpaceData ws) {
    float normalFactor = saturate(dot(normalize(lerp(ws.viewDir, ws.normal, sssBuffer.normalStrength)), ws.viewDir));
    float heightFactor = saturate((ws.position.y + oceanData.referenceWaveHeight * (1 + sssBuffer.heightBias)) * 0.5 / max(0.5, oceanData.referenceWaveHeight));
    heightFactor = pow(abs(heightFactor), max(1.0, oceanData.referenceWaveHeight * 0.4));

    float2 factor; // x is sun, y is environment
    factor.x = sssBuffer.sunStrength * normalFactor * heightFactor * pow(saturate(dot(sunDirection.xyz, -ws.viewDir)), min(50.0, 1 / sssBuffer.spread));
    factor.y = sssBuffer.environmentStrength * normalFactor * heightFactor * saturate(1.0 - ws.viewDir.y);
    factor *= sssBuffer.fadeDistance / (sssBuffer.fadeDistance + ws.viewDist);
    return factor;
}

float3 Bruneton_Refraction(const WorldSpaceData ws, const float2 sss, const FoamData foamData, const float3 foamColor) {
    float3 color = float3(0.0, 0.0127, 0.040196); // this is ocean diffuse color
    color += sssBuffer.color * saturate(sss.x + sss.y);
    const float NdotL = saturate(dot(ws.normal, sunDirection.xyz));

    color += (NdotL * 0.8 + 0.2) * sunColor.rgb * sssBuffer.colorMulti;

    const float underwaterFoamVisibility = 20.0 / (20.0 + ws.viewDist);
    const float3 tint = AbsorptionTint(0.8);
    const float3 underwaterFoamColor = foamColor * tint * tint;
    return lerp(color, underwaterFoamColor, foamData.coverage.y * underwaterFoamVisibility);
}

float4 HorizonBlend(const WorldSpaceData ws) {
    const float3 dir = -float3(ws.viewDir.x, 0.0, ws.viewDir.z);

    const float distanceScale = 100.0 + 7.0 * abs(cameraPos.y);
    const float t = exp(-5.0 / oceanData.horizon_fog * (abs(ws.viewDir.y) + distanceScale / (ws.viewDist + distanceScale)));
    return float4(HORIZON_COLOR, t);
}

float2 SlopeVarianceSqrd(float windSpeed, float viewDist, float alignment, float scale) {
    float upwind = 0.01 * sqrt(windSpeed) * viewDist / (viewDist + scale);
    return float2(upwind, upwind * (1 - 0.3 * alignment));
}

float3 CalculateOceanColor(const WorldSpaceData ws, const FoamData foamData) {
    float3 tangentY = float3(0.0, ws.normal.z, -ws.normal.y);
    tangentY /= max(0.001, length(tangentY));

    float3 tangentX = cross(tangentY, ws.normal);

    float2 slopeVarianceSqrd = ROUGHNESS_SCALE * (1.0 + WATER_ROUGHNESS * 0.3) * SlopeVarianceSqrd(oceanData.windSpeed * oceanData.waveScale, ws.viewDist, oceanData.waveAlignment, ROUGHNESS_DISTANCE);

    const float2 sss = SubsurfaceScatteringFactor(ws);

    const float fresnel = Bruneton_Fresnel(ws.viewDir, ws.normal, slopeVarianceSqrd);

    const float foamCoverageY = 1.0 - foamData.coverage.y;

    const float3 specular = Bruneton_Specular(ws, slopeVarianceSqrd, tangentX, tangentY) * foamCoverageY * foamCoverageY * foamCoverageY * foamCoverageY * foamCoverageY;
    const float3 reflected = float3(0.0);//Bruneton_Reflection(ws);
    const float3 foamLitColor = LitFoamColor(ws, foamData, reflected);
    const float3 refracted = Bruneton_Refraction(ws, sss, foamData, foamLitColor);
    float3 color = specular + lerp(refracted, reflected, fresnel);

    color = lerp(color, foamLitColor, foamData.coverage.x);
    // debug to see effect
    // color = lerp(color, lerp(color, foamLitColor, foamData.coverage.x), float(ws.position.x > 0));

    // DEBUGGING
    // return lerp(-reflect(-ws.viewDir, ws.normal), reflected, float(ws.position.z > 0));
    return color;

    // const float4 horizon = HorizonBlend(ws);
    // return lerp(color, horizon.rgb, horizon.a);
}