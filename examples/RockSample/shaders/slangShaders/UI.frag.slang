#version 450

layout (location = 0) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform Sampler2DArray<float4> texSampler;

layout(push_constant) uniform Push {
  vec4 scaleOffset;
  vec3 color;
  int textureIndex;
} push;

[shader("fragment")]
void fragmentMain() {
    uint status;
    float4 color = texSampler.Sample(float3(fragTexCoord, float(push.textureIndex)), int2(0, 0), 0.f, status);
    outColor = vec4(push.color * color.rgb, color.a);
}