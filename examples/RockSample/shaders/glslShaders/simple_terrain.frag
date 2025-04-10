#version 450

struct PointLight{
	vec4 position; //ignore w
	vec4 color; //w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projView;
	vec4 cameraPos;
} ubo;

layout(set = 0, binding = 1) uniform LightBufferObject {
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
	PointLight pointLights[10]; //max lights in frameinfo header
	int numLights;
} lbo;


layout(set = 0, binding = 2) uniform TescBO{
    mat4 projection;
    mat4 view;
    vec4 frustumPlanes[6];
    vec2 viewportDim;
    float displacementFactor;
    float tessFactor;
    float tessEdgeSize;
	int octaves;
	float worldPosNoiseScaling;
    float sandHeight;
    float grassHeight;
} tbo;


layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inPos;
layout(location = 0) out vec4 outColor;

void main(){
    const float height = -inPos.y; //getting it from a tese pass now
	/*
	for (int i = 0; i < 6; i++) {
		const float range = layers[i].y - layers[i].x;
		float weight = (range - abs(height - layers[i].y)) / range;
		weight = max(0.0, weight);
		color += weight * texture(samplerLayers, vec3(inUV * 16.0, i)).rgb;
	}
	*/

	if(height < 0.0){
		outColor = vec4(0.0, 0.0, 1.0, 1.0);
	}
	else if (height < tbo.sandHeight){
		outColor = vec4(0.75, 0.6, 0.5, 1.0);
	}
	else if (height < tbo.grassHeight){
		outColor = vec4(0.0, 1.0, 0.0, 1.0);
	}
    else{
	    outColor = vec4(1.0, 1.0, 1.0, 1.0);
    }

}