#version 460

#include "oceanography.h.slang"

// RWStructuredBuffer<float4> frequencySpectrum;
// RWTexture2DArray<float4> fftTex;

layout(set = 0, binding = 0) RWTexture2DArray<float4> frequencySpectrum;
layout(set = 0, binding = 1) RWTexture2DArray<float4> fftTex;

layout(push_constant) uniform Push {
    float4 lengthScale;
    float4 cutoffLow;
    float4 cutoffHigh;
    float depth;
    float time;
} push;

float GetBooleanContribution(const float waveLength, const float cutoffLow, const float cutoffHigh)
{
    return float(waveLength <= cutoffHigh && waveLength > cutoffLow);
}

[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 id: SV_DispatchThreadID) {
    const float2 pos = float2(id.xy) - float(WAVE_COUNT) / 2.0;

    [unroll] for (uint cascade = 0; cascade < CASCADE_COUNT; cascade++) {
        float2 waveVec = pos * M_2_PI / push.lengthScale[cascade];
        float waveVecMag = length(waveVec);

        const float boolContribution = GetBooleanContribution(waveVecMag, push.cutoffLow[cascade], push.cutoffHigh[cascade]);
        waveVec *= boolContribution;
        waveVecMag = max(1e-4, length(waveVec));

        const float phase = Frequency(waveVecMag, push.depth) * push.time * boolContribution;
        const Complex exponent = float2(cos(phase), sin(phase));

        const float4 initFreq = frequencySpectrum[uint3(id.xy, cascade)];
        const Complex h = ComplexMult(initFreq.xy, exponent);
        const Complex inverseH = float2(-h.y, h.x);

        const float2 displacementX = boolContribution * inverseH * waveVec.x / waveVecMag;
        const float2 displacementY = h;
        const float2 displacementZ = boolContribution * inverseH * waveVec.y / waveVecMag;

        const float2 displacementX_dx = boolContribution * -h * waveVec.x * waveVec.x / waveVecMag;
        const float2 displacementY_dx = inverseH * waveVec.x;
        const float2 displacementZ_dx = boolContribution * -h * waveVec.x * waveVec.y / waveVecMag;

        const float2 displacementY_dz = inverseH * waveVec.y;
        const float2 displacementZ_dz = boolContribution * -h * waveVec.y * waveVec.y / waveVecMag;

        fftTex[uint3(id.xy, cascade)] = float4(
            displacementX.x - displacementY.y,
            displacementX.y + displacementY.x,
            displacementZ.x - displacementZ_dx.y,
            displacementZ.y + displacementZ_dx.x
        );
        fftTex[uint3(id.xy, cascade + CASCADE_COUNT)] = float4(
            displacementY_dx.x - displacementY_dz.y,
            displacementY_dx.y + displacementY_dz.x,
            displacementX_dx.x - displacementZ_dz.y,
            displacementX_dx.y + displacementZ_dz.x
        );
    }
}