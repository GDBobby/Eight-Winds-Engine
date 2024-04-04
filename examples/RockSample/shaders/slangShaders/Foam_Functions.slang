#ifndef FOAM_FUNCTIONS_H
#define FOAM_FUNCTIONS_H

#include "oceanography.h.slang"
#include "Ocean_Render_Inputs.h.slang"
#include "fragment_constants.h.slang"

struct FoamData {
    float2 coverage;
    float3 normal;
    float3 albedo;
};

float4 SampleTurbulence(float2 worldUV, float4 lodWeights) {
    float4 turbulence = foamBuffer.cascadeWeights[0] * lodWeights[0]
                    * oceanTextures.Sample( 
                        float3(
                            worldUV / oceanData.lengthScales[0],
                            0.0 + CASCADE_COUNT * 2
                        )
                    );
    [unroll] for (uint cascade = 1; cascade < CASCADE_COUNT; cascade++) {
        if (lodWeights[cascade] > 200.0) {
            turbulence += foamBuffer.cascadeWeights[cascade] * lodWeights[cascade]
                * oceanTextures.Sample(
                    float3(
                        worldUV / oceanData.lengthScales[cascade],
                        float(cascade) + CASCADE_COUNT * 2
                    )
                );
        }
    }
    float denom = foamBuffer.cascadeWeights.x * lodWeights.x * CASCADE_COUNT 
                + foamBuffer.cascadeWeights.y * lodWeights.y * CASCADE_COUNT 
                + foamBuffer.cascadeWeights.z * lodWeights.z * CASCADE_COUNT 
                + foamBuffer.cascadeWeights.w * lodWeights.w * CASCADE_COUNT;

    return turbulence / denom;
}
float2 FoamCoverage(float4 lodWeights, float2 worldUV, float deepFoam, float bias) {
    float4 turbulence = SampleTurbulence(worldUV, lodWeights);
    float foamValueCurrent = lerp(turbulence.y, turbulence.x, foamBuffer.sharpness);
    float foamValuePersistent = (turbulence.z + turbulence.w) / 2.0;
    foamValueCurrent = lerp(foamValueCurrent, foamValuePersistent, foamBuffer.persistence);
    foamValueCurrent -= 1.0;

    float foamValue = foamValueCurrent + foamBuffer.coverage * (1.0 - bias);
    float surfaceFoam = saturate(foamValue * foamBuffer.density);

    float shallowUnderwaterFoam = saturate((foamValue + 0.1 * foamBuffer.underwater) * foamBuffer.density);
    float deepUnderwaterFoam = deepFoam * saturate((foamValue + foamBuffer.underwater * 0.25) * foamBuffer.density * 0.8);

    return float2(surfaceFoam, max(shallowUnderwaterFoam, deepUnderwaterFoam));
}

FoamData CalculateFoamData(const float2 worldUV, const float viewDist, const float3 viewDir, const float3 normal, const float4 lodWeights) //, float time)
{
    FoamData data;
    data.albedo = foamBuffer.albedo;

    const float deepFoam = 0.5;

    // float bias = Sampler2D(foamBias, worldUV * 0.0001).r * saturate(viewDist / oceanData.lengthScales[0] / 2.0);
    
    float bias = 0.0;

    data.coverage = FoamCoverage(lodWeights, worldUV, deepFoam, bias);
    data.coverage *= 1.0 - saturate((cameraPos.y + viewDist * 0.5 - 2000.0) * 0.0005);

    // this is disabled currently, my foam texture is awful
    // data.albedo = SampleTex2D(Get(foamAlbedo), Get(uSampler0), worldUV).rgb;
    // data.albedo = SampleLvlTex2D(Get(foamAlbedo), Get(samplerFoamAlbedo), worldUV).r;
    // im assuming albedo always returns a 1.0, may need to change that
    return data;
}
float3 LitFoamColor(const WorldSpaceData ws, const FoamData foamData, float3 reflectedColor) {
    const float NdotL = (0.2 * 0.8 * saturate(dot(foamData.normal, sunDirection.xyz)));

    return foamData.albedo * foamBuffer.tint * (NdotL * sunColor.rgb + float3(.812, 0.867, 0.851));
    // theres an additional thing here with spherical harmonics
    // would probably make the foam look much better, come back later
}

float3 AbsorptionTint(const float t) {
    const float4 colors[8] =
        {
            float4(0.0, 0.041025557, 0.094412796, 0.0),
            float4(0, 0.17351386, 0.43203577, 0.2),
            float4(0.16198544, 0.68352747, 0.79865986, 0.66608685),
            float4(1, 1, 1, 1),
            float4(1, 1, 1, 1),
            float4(1, 1, 1, 1),
            float4(1, 1, 1, 1),
            float4(1, 1, 1, 1)
        };
    // i should be able to calculate this value then bake it in as a constant
    // would save a decent amount of processing, in the fragment shader
    float3 color = colors[0].rgb;
    [unroll] for (uint i = 1; i < 8; i++)
    {
        float colorPos = saturate((t - colors[i - 1].w) / (colors[i].w - colors[i - 1].w)) * step(i, 3);
        color = lerp(color, colors[i].rgb, colorPos);
    }
    return color;
}

#endif