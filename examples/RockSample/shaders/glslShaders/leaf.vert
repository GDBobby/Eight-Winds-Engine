#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
//layout(location = 1) in float uv;

//instancing
//layout(location = 3) in mat4 instanceTransform;

//layout(location = 0) out float fragUV;
layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec2 fragTexCoord;

layout(set = 0, binding = 0) readonly buffer LeafBO {
	mat4 projView;
	vec4 cameraPos;
	mat4 leafMatrices[1024];
} lbo;

void main(){
	//position += texture(texSampler, instanceTexCoord).rgb * position.y;
	//in int gl_InstanceID;
	vec4 positionWorld = lbo.leafMatrices[gl_InstanceIndex] * (vec4(position, 1.0));
	gl_Position = lbo.projView * positionWorld;
	
	fragPosWorld = positionWorld.xyz;
	//fragUV = uv;
	fragNormalWorld = normal;
	fragTexCoord = uv;
}