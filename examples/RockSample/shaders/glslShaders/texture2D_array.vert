#version 460

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 fragTexCoord;

layout(push_constant) uniform Push {
  vec4 scaleOffset;
  vec3 color;
  int textureIndex;
} push;

void main() {
  gl_Position = vec4(push.scaleOffset.xy * position + push.scaleOffset.zw, 0.0, 1.0);
  
  fragTexCoord = uv;
}