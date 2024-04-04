#include "oceanography.h.slang"
RWTexture2DArray<float4> oceanTextures;

[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 id: SV_DispatchThreadID) {
    oceanTextures[uint3(id.xy, id.z + CASCADE_COUNT * 2)] = float4(-5.0, -5.0, -5.0, -5.0);
}