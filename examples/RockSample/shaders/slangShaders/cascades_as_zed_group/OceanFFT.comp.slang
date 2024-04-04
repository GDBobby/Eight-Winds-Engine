#version 460
#include "oceanography.h.slang"

/*
	most implementations do horizontal then vertical
	if i do vertical then horizontal, i can calculate jacobian foam without a separate compute dispatch
	foam calculation is bundled in butterfly_hori.comp.fsl

*/

// depthdownsample.comp.fsl

RWTexture2DArray<float4> fftTex;

layout(push_constant) uniform Push {
    float deltaTime;
    float foamBias;
} push;

static const uint texture_count = 2; //displacement, derivatives

groupshared float4 pingpong[2][WAVE_COUNT];
#define FOAM_DECAY_RATE 0.2

[shader("compute")]
[numthreads(WAVE_COUNT, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID) {

    const uint2 position = id.yx;
    const uint index = id.x;
    const uint total_texture_count = texture_count;// * CASCADE_COUNT; id.z being used instead of loops for cascades

    float2 twiddle[LOG_2_VALUE];
    uint2 inputIndices[LOG_2_VALUE];


    //calculating twiddle and butterfly indices (cooley-tukey)
    [unroll] for (uint stage = 0; stage < (LOG_2_VALUE - 1); stage++)
    {
        const uint b = WAVE_COUNT >> (stage + 1);
        const uint w = b * (index / b);
        // float mult = -M_2_PI / WAVE_COUNT * ((position / b) * b);
        const float intermediateSinCos = -M_2_PI / WAVE_COUNT * w;

        twiddle[stage] = float2(cos(intermediateSinCos), -sin(intermediateSinCos));
        const uint intermediateIndex = (w + index) % WAVE_COUNT;
        inputIndices[stage] = uint2(intermediateIndex, intermediateIndex + b);
    }

    // vertical
    for (uint texture_id = 0; texture_id < total_texture_count; texture_id++) {

        pingpong[0][index] = fftTex[uint3(position, id.z + texture_id * CASCADE_COUNT)];
        GroupMemoryBarrierWithGroupSync();

        bool flag = false; // this is used to flip/flop read/write

        for (uint stage = 0; stage < (LOG_2_VALUE - 1); stage++) {

            const float4 bfBuffer = pingpong[uint(flag)][inputIndices[stage].y];
            pingpong[uint(!flag)][index] = pingpong[uint(flag)][inputIndices[stage].x]
					+ float4(ComplexMult(twiddle[stage], bfBuffer.xy), ComplexMult(twiddle[stage], bfBuffer.zw));

            flag = !flag;
            GroupMemoryBarrierWithGroupSync();
        }
        //final stage pass, writes to buffer
        const float4 bfBufferOuter = pingpong[uint(flag)][inputIndices[LOG_2_VALUE - 1].y];

        fftTex[uint3(position, id.z + texture_id * CASCADE_COUNT)] = pingpong[uint(flag)][inputIndices[LOG_2_VALUE - 1].x]
+ float4(ComplexMult(twiddle[LOG_2_VALUE - 1], bfBufferOuter.xy), ComplexMult(twiddle[LOG_2_VALUE - 1], bfBufferOuter.zw));

        // this if statement shouldnt be calculated on multiple branches simultaneously
        if (texture_id < (total_texture_count - 1)) {
            GroupMemoryBarrierWithGroupSync();
        }
    }

    const uint2 positionFlipped = position.yx;

    float4 final_values[total_texture_count];
    // horizontal
    [unroll] for (uint texture_id = 0; texture_id < (total_texture_count); texture_id++) {

        pingpong[0][index] = fftTex[uint3(positionFlipped, id.z + texture_id * CASCADE_COUNT)];
        GroupMemoryBarrierWithGroupSync();

        bool flag = false;
        [unroll] for (uint stage = 0; stage < (LOG_2_VALUE - 1); stage++)
        {

            const float4 bfBuffer1 = pingpong[uint(flag)][inputIndices[stage].y];
            pingpong[uint(!flag)][index] = pingpong[uint(flag)][inputIndices[stage].x]
					+ float4(ComplexMult(twiddle[stage], bfBuffer1.xy), ComplexMult(twiddle[stage], bfBuffer1.zw));

            flag = !flag;
            GroupMemoryBarrierWithGroupSync();
        }
        //final stage pass, write to buffer
        const float4 bfBufferOuter1 = pingpong[uint(flag)][inputIndices[LOG_2_VALUE - 1].y];

        final_values[texture_id] = pingpong[uint(flag)][inputIndices[LOG_2_VALUE - 1].x]
					+ float4(ComplexMult(twiddle[LOG_2_VALUE - 1], bfBufferOuter1.xy), ComplexMult(twiddle[LOG_2_VALUE - 1], bfBufferOuter1.zw));
        fftTex[uint3(positionFlipped, id.z + texture_id * CASCADE_COUNT)] = final_values[texture_id];

        // this if statement shouldnt be calculated on multiple branches simultaneously
        if (texture_id < (total_texture_count - 1)) {
            GroupMemoryBarrierWithGroupSync();
        }
    }
    const uint cascade = id.z;
    const float jacob_xx = 1.0 + final_values[0].z;
    const float tempHold = final_values[1].w;
    const float jacob_zz = 1.0 + tempHold;
    const float jacob_xz_sqrd = tempHold * tempHold;

    const float jacobian = jacob_xx * jacob_zz - jacob_xz_sqrd;
    const float jacob_minus =
        (jacob_xx + jacob_zz -
            sqrt((jacob_xx - jacob_zz) * (jacob_xx - jacob_zz) + 4.0 * jacob_xz_sqrd))
            * 0.5;
    const float2 current_foam = float2(push.foamBias - jacob_minus, push.foamBias - jacobian);
    float2 persistent_foam = fftTex[uint3(positionFlipped, cascade + (CASCADE_COUNT * 2))].zw;
    persistent_foam -= FOAM_DECAY_RATE * push.deltaTime;
    persistent_foam = max(current_foam, persistent_foam);
    fftTex[uint3(positionFlipped, cascade + (CASCADE_COUNT * 2))] = float4(current_foam, persistent_foam);
    
}

