#version 450

layout(location = 0) out vec2 fragUV;


layout(set = 1, binding = 0) readonly buffer tileVertices {
	vec4 tileVertex[]; //its already in world space 
};

layout(set = 1, binding = 1) readonly buffer tileIndices {
	int tileIndex[];
};
layout(set = 1, binding = 2) readonly buffer tileUVs{
	vec2 tileUV[];
};

layout(push_constant) uniform Push{
	vec3 translation;
	vec3 scaling;
} push;

void main(){
	vec4 translatedPos = tileVertex[tileIndex[gl_InstanceIndex * 4 + gl_VertexIndex]];
	translatedPos.x = (translatedPos.x + push.translation.x) * push.scaling.x;
	translatedPos.y = (translatedPos.y + push.translation.y) * push.scaling.y;
	translatedPos.z = (translatedPos.z + push.translation.z) * push.scaling.z;
	
	gl_Position = translatedPos;
	
	fragUV = tileUV[gl_InstanceIndex * 4 + gl_VertexIndex];
}