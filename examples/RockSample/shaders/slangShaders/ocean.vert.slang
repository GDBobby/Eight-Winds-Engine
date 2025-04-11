#include "oceanography.h.slang"
#include "Ocean_Render_Inputs.h.slang"

float2 SimplexHash(float2 p) { // replace this by something better {
    p = float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)));
    return -1.0 + 2.0 * frac(sin(p) * 43758.5453123);
}

// https://www.shadertoy.com/view/Msf3WH
float SimplexNoise(const float2 p) {
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;

    float2 i = floor(p + (p.x + p.y) * K1);
    float2 a = p - i + (i.x + i.y) * K2;
    float m = step(a.y, a.x);
    float2 o = float2(m, 1.0 - m);
    float2 b = a - o + K2;
    float2 c = a - 1.0 + 2.0 * K2;
    float3 h = max(0.5 - float3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
    float3 n = h * h * h * h * float3(dot(a, SimplexHash(i + 0.0)), dot(b, SimplexHash(i + o)), dot(c, SimplexHash(i + 1.0)));
    return dot(n, float3(70.0));
}

float NoiseWithOctaves(float2 uv, int octaves) {
    float freq = 0.5;
    const float2x2 m = float2x2(1.6, 1.2, -1.2, 1.6);
    float f = 0.5 * SimplexNoise(uv);
    for (int i = 1; i < octaves; i++) {
        freq /= 2.0;
        uv = mul(m, uv);
        f += freq * SimplexNoise(uv);
    }

    return f;
}

float3 SampleDisplacement(float viewDist, float2 worldUV, float cascadeFadeDistance) {
    const float lodThreshold = 1.0 / cascadeFadeDistance;
    const float4 weights = GetLodWeights(viewDist, oceanData.lengthScales, cascadeFadeDistance);

    const float height = NoiseWithOctaves(worldUV / 1000.0, 8); // this should scale from something like -sqrt(2) to sqrt(2)

    float3 displace = float3(0.0); // oceanTextures.SampleLevel(float3(worldUV / oceanData.lengthScales[0], 0.0), 0.0).xyz * weights[0];

    const float depthTesting = saturate(height * oceanData.beachScale.z + oceanData.beachScale.x) * oceanData.beachScale.y;
    if (depthTesting > 0.0) {
      displace += oceanTextures.SampleLevel(float3(worldUV / oceanData.lengthScales[0], 0.0), 0.0).xyz * depthTesting;
    }


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