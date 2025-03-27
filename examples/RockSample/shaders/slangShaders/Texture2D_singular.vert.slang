#version 460

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 fragTexCoord;

layout(push_constant) uniform Push {
    vec4 scaleOffset;
    vec3 color;
} push;

[shader("vertex")]
void vertexMain() {

    vec2 temp = push.scaleOffset.zw + (position * push.scaleOffset.xy);

  gl_Position = vec4(temp.x, temp.y, 0.0, 1.0);
  
  fragTexCoord = uv;
}