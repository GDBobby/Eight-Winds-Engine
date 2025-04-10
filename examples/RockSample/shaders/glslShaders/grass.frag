#version 450
#extension GL_GOOGLE_include_directive : enable

#include "SimplexNoise.glsl"

layout(location = 0) in PerVertexData{
    vec3 worldPos;
    vec3 worldNormal;
    float distanceFromCenter;
    float heightPercent;
} v_in;

layout(location = 0) out vec4 outColor;

void main(){
    	
    const float heightShadow = clamp(pow(v_in.heightPercent, 1.5), 0, 1);
    const float midShadow = clamp(0.3 + 0.7 * pow(abs(v_in.distanceFromCenter), 1.5), 0.0, 1.0);
    const float selfshadow = min(heightShadow, midShadow);
    //debugPrintfEXT("self shadow - (%f)", selfshadow);
    
    

    vec3 outRGB = vec3(0.41, 0.44, 0.29);
    outRGB.r = pow(outRGB.r, 2.2) * selfshadow;
    outRGB.g = pow(outRGB.g, 2.2) * selfshadow;
    outRGB.b = pow(outRGB.b, 2.2) * selfshadow;
    outRGB *= 0.6 + 0.25 * SimplexNoise(0.25 * v_in.worldPos.xy);

    outColor = vec4(outRGB, 1.0);
    
    //vec3 normal = normalize(v_in.worldSpaceNormal);

    //if (!gl_FrontFacing) {
   //     normal = -normal;
    //}

    //output.normal.xyz = normalize(lerp(float3(0, 0, 1), normal, 0.25));

    //return output;
}
