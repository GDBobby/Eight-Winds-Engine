
#ifndef FRAGMENT_CONSTANTS_H
#define FRAGMENT_CONSTANTS_H

static const float WATER_ROUGHNESS = 0.1;
static const float ROUGHNESS_SCALE = 1.0;
static const float SPECULAR_STRENGTH = 1.0;
static const float SPECULAR_MIN_ROUGHNESS = 0.1;
static const float REFLECTION_NORMAL_STRENGTH = 0.25;
static const float REFRACTION_STRENGTH = 0.25;
static const float ROUGHNESS_DISTANCE = 140;
static const float HORIZON_FOG = 0.25;
static const float3 HORIZON_COLOR = float3(1.0, 1.0, 1.0); //need to twiddle with this
// i need a horizon_color texture, which gives the color of the horizon based on the direction im looking, and the time of day

struct WorldSpaceData {
    float3 normal;
    float3 viewDir;
    float viewDist;
    float roughVal;  // 0.0
    float3 position; // world space
    float4 shore;
};

#endif