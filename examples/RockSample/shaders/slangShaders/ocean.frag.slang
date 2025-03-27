#include "Bruneton_Functions.slang" //includes multiple other files, i should expand them into here, but im a bit lazy
#include "Ocean_Render_Inputs.h.slang"

struct NormalData {
    float3 waterNormal;
    float3 foamNormal;
    float4 lodWeights;
};

// void SampleNormal(float viewDist, float2 worldUV, out float3 waterNormal, out float3 foamNormal, out float4 lodWeights)
NormalData SampleNormal(float viewDist, float2 worldUV) {
    NormalData ret;
    ret.lodWeights = GetLodWeights(viewDist, oceanData.lengthScales, 200.0);
    const float lodThreshold = 1.0 / 200.0;

    const float4 derivatives0 = oceanTextures.Sample(float3(worldUV / oceanData.lengthScales[CASCADE_COUNT], CASCADE_COUNT));
    const float4 derivatives1 = oceanTextures.Sample(float3(worldUV / oceanData.lengthScales[CASCADE_COUNT + 1], CASCADE_COUNT + 1.0));
#if CASCADE_COUNT >= 3
    const float4 derivatives2 = oceanTextures.Sample(float3(worldUV / oceanData.lengthScales[CASCADE_COUNT + 2], CASCADE_COUNT + 2.0));
#endif
#if CASCADE_COUNT == 4
    const float4 derivatives3 = oceanTextures.Sample(float3(worldUV / oceanData.lengthScales[CASCADE_COUNT + 3], CASCADE_COUNT + 3.0));
#endif

    const float4 waterDerivatives =
        derivatives0 * ret.lodWeights[0] * ret.lodWeights[0]
			+ derivatives1 * ret.lodWeights[1] * ret.lodWeights[1]
#if CASCADE_COUNT >= 3
            + derivatives2 * ret.lodWeights[2] * ret.lodWeights[2]
#endif
#if CASCADE_COUNT == 4
            + derivatives3 * ret.lodWeights[3] * ret.lodWeights[3]
#endif
        ;

    const float4 foamWeights = saturate(foamBuffer.normalWeights + foamBuffer.normalDetail) * CASCADE_COUNT;
    const float4 foamDerivatives =
        derivatives0 * ret.lodWeights[0] * foamWeights[0]
			+ derivatives1 * ret.lodWeights[1] * foamWeights[1]
#if CASCADE_COUNT >= 3
            + derivatives2 * ret.lodWeights[2] * foamWeights[2]
#endif
#if CASCADE_COUNT == 4
            + derivatives3 * ret.lodWeights[3] * foamWeights[3]
#endif
        ;

    ret.waterNormal = normalize(float3(
        -waterDerivatives.x / max(0.001, 1.0 + waterDerivatives.z),
        1.0,
        -waterDerivatives.y / max(0.001, 1.0 + waterDerivatives.w)
		   )
    );
    ret.foamNormal = normalize(float3(
        -foamDerivatives.x / max(0.001, 1.0 + foamDerivatives.z),
        1.0,
        -foamDerivatives.y / max(0.001, 1.0 + foamDerivatives.w)
		   )
    );
    return ret;
}

[shader("fragment")]
float4 fragmentMain(VertexStageOutput input) {
    WorldSpaceData ws;
    ws.viewDir = cameraPos.xyz - input.worldPos;
    ws.viewDist = length(ws.viewDir);
    const NormalData normalData = SampleNormal(ws.viewDist, input.uv);
    ws.normal = normalData.waterNormal;

    FoamData foamData = CalculateFoamData(input.uv, ws.viewDist, ws.viewDir, ws.normal, normalData.lodWeights);
    foamData.normal = normalData.foamNormal;

    ws.viewDir /= ws.viewDist;
    ws.roughVal = 0.0;
    ws.position = input.worldPos;

    if (dot(ws.normal, ws.viewDir) < 0.0) {
        // backface
        ws.normal = reflect(ws.normal, ws.viewDir);
    }
    const float3 outColor = CalculateOceanColor(ws, foamData);
    return float4(outColor, 1.0);
    //return float4(ws.normal, 1.0);
}