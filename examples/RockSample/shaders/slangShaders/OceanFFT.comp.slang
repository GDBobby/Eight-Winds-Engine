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
    int secondPass; //true or false
} push;

static const uint texture_count = 2; //displacement, derivatives

groupshared float4 pingpong[2][WAVE_COUNT];
#define FOAM_DECAY_RATE 0.2

[shader("compute")]
[numthreads(WAVE_COUNT, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID) {

    const uint2 position = push.secondPass > 0 ? id.xy : id.yx;
    const uint index = id.x;
    const uint total_texture_count = texture_count * CASCADE_COUNT;

    float2 twiddle[LOG_2_VALUE];
    uint2 inputIndices[LOG_2_VALUE];

    float permute = 1.0 - (2.0 * push.secondPass * float((id.x + id.y) % 2));


    //calculating twiddle and butterfly indices (bailey's)
    [unroll] for (uint stage = 0; stage < LOG_2_VALUE; stage++)
    {
        const uint b = WAVE_COUNT >> (stage + 1);
        const uint w = b * (index / b);
        // float mult = -M_2_PI / WAVE_COUNT * ((position / b) * b);
        const float intermediateSinCos = -M_2_PI / WAVE_COUNT * w;

        twiddle[stage] = float2(cos(intermediateSinCos), -sin(intermediateSinCos));
        const uint intermediateIndex = (w + index) % WAVE_COUNT;
        inputIndices[stage] = uint2(intermediateIndex, intermediateIndex + b);
    }

    float4 final_values[total_texture_count];
    // vertical
    for (uint texture_id = 0; texture_id < total_texture_count; texture_id++) {

        pingpong[0][index] = fftTex[uint3(position, texture_id)];
        GroupMemoryBarrierWithGroupSync();

        bool flag = false; // this is used to flip/flop read/write

        [unroll] for (uint stage = 0; stage < (LOG_2_VALUE - 1); stage++) {

            const float4 bfBuffer = pingpong[uint(flag)][inputIndices[stage].y];
            pingpong[uint(!flag)][index] = pingpong[uint(flag)][inputIndices[stage].x]
					+ float4(ComplexMult(twiddle[stage], bfBuffer.xy), ComplexMult(twiddle[stage], bfBuffer.zw));

            flag = !flag;
            GroupMemoryBarrierWithGroupSync();
        }
        //final stage pass, writes to buffer
        const float4 bfBufferOuter = pingpong[uint(flag)][inputIndices[LOG_2_VALUE - 1].y];

        final_values[texture_id] = pingpong[uint(flag)][inputIndices[LOG_2_VALUE - 1].x] 
                    + float4(ComplexMult(twiddle[LOG_2_VALUE - 1], bfBufferOuter.xy),
                            ComplexMult(twiddle[LOG_2_VALUE - 1], bfBufferOuter.zw)
                    );
        final_values[texture_id] *= permute;
        fftTex[uint3(position, texture_id)] = final_values[texture_id];
    }

    if (push.secondPass > 0) {
        [unroll] for (uint cascade = 0; cascade < CASCADE_COUNT; cascade++) {
            const float jacob_xx = 1.0 + final_values[cascade].z;
            const float tempHold = final_values[cascade + CASCADE_COUNT].w;
            const float jacob_zz = 1.0 + tempHold;
            const float jacob_xz_sqrd = tempHold * tempHold;

            const float jacobian = jacob_xx * jacob_zz - jacob_xz_sqrd;
            const float jacob_minus =
                (jacob_xx + jacob_zz -
                 sqrt((jacob_xx - jacob_zz) * (jacob_xx - jacob_zz) + 4.0 * jacob_xz_sqrd))
				* 0.5;
            const float2 current_foam = float2(push.foamBias - jacob_minus, push.foamBias - jacobian);
            float2 persistent_foam = fftTex[uint3(position, cascade + (CASCADE_COUNT * 2))].zw;
            persistent_foam -= FOAM_DECAY_RATE * push.deltaTime;
            persistent_foam = max(current_foam, persistent_foam);
            fftTex[uint3(position, cascade + (CASCADE_COUNT * 2))] = float4(current_foam, persistent_foam);
        }
    }
}

