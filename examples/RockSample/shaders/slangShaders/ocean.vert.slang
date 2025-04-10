#include "oceanography.h.slang"
#include "Ocean_Render_Inputs.h.slang"

float3 SampleDisplacement(float viewDist, float2 worldUV, float cascadeFadeDistance) {
    const float lodThreshold = 1.0 / cascadeFadeDistance;
    const float4 weights = GetLodWeights(viewDist, oceanData.lengthScales, cascadeFadeDistance);

    float3 displace = oceanTextures.SampleLevel(float3(worldUV / oceanData.lengthScales[0], 0.0), 0.0).xyz * weights[0];
    

    [unroll] for (uint cascade = 1; cascade < CASCADE_COUNT; cascade++) {
        if (weights[cascade] > lodThreshold) {
            displace += oceanTextures.SampleLevel(float3(worldUV / oceanData.lengthScales[cascade], float(cascade)), 0.0).xyz * weights[cascade];
        }
    }
    return displace;
}

[shader("vertex")]
VertexStageOutput vertexMain(AssembledVertex assembledVertex) {
    VertexStageOutput output;
    output.worldPos = assembledVertex.position;
    const float viewDist = length(assembledVertex.position - cameraPos.xyz);

    output.uv = output.worldPos.xz;
    output.worldPos += SampleDisplacement(viewDist, output.uv, 200.0);

    output.sv_Position = mul(projView, float4(output.worldPos, 1.0));
    //output.sv_Position = mul(mvp, output.worldPos);
    return output;
}