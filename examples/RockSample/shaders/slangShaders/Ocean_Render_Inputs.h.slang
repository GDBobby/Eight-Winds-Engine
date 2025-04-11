#ifndef OCEAN_RENDER_INPUTS_H
#define OCEAN_RENDER_INPUTS_H

struct AssembledVertex {
    float3 position;
};
struct VertexStageOutput {
    float4 sv_Position : SV_Position;
    float3 worldPos : POSITION;
    float2 uv : UV;
};
/*
cbuffer Uniform_Block {
    float4x4 mvp;
    float3 cameraPos;
    float3 sunDirection;
    float3 sunColor;
};
*/
layout(set = 0, binding = 0) cbuffer GlobalUbo {
    float4x4 projView;
    float4 cameraPos;
};

struct PointLight {
    float4 position; // ignore w
    float4 color;    // w is intensity
};
layout(set = 0, binding = 1) cbuffer GPUSceneData {
    float4 ambientColor;
    float4 sunDirection; // w for sun power
    float4 sunColor;
    PointLight pointLights[10]; // max lights in frameinfo header
    int numLights;
};

struct OceanData {
    float4 beachScale;
    float4 lengthScales;
    float windSpeed;
    float waveScale;
    float waveAlignment;
    float referenceWaveHeight;
    float2 windDirection;
    float horizon_fog;

};
struct FoamDataBuffer {
    float underwaterFoamParallax;
    float normalDetail;
    float density;
    float underwaterTexScale;
    float sharpness;
    float persistence;
    float coverage;
    float underwater;
    float bias;
    float __padding0;
    float __padding1;
    float __padding2;
    float3 tint;
    float3 albedo;
    float4 cascadeWeights;
    float4 normalWeights;
};
struct SSSDataBuffer {
    float normalStrength;
    float sunStrength;
    float environmentStrength;
    float fadeDistance;
    float heightBias;
    float spread;
    float __padding0;
    float __padding1;
    float3 color;
    float3 colorMulti;
};

layout(set = 0, binding = 2) cbuffer OceanUniform {
    OceanData oceanData;
    FoamDataBuffer foamBuffer;
    SSSDataBuffer sssBuffer;
};

layout(set = 0, binding = 3) Sampler2DArray<float4> oceanTextures;
layout(set = 0, binding = 4) SamplerCube<float4> skyTexture;


#endif