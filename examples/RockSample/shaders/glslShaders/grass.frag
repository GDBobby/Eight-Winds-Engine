#version 450
#extension GL_GOOGLE_include_directive : enable

#include "SimplexNoise.glsl"


layout(set = 0, binding = 2) uniform GrassBufferObject{
    float spacing;
    float height;
    float time;
    float windDir;
    vec4 endDistance; //LOD = 6 - sqrt(x) / endDistance; blade vertex count = LOD * 2
    float windStrength;
    int displayLOD;
} gbo;

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
    
    if(gbo.displayLOD == 1){
        uint uintLOD = uint(v_in.worldNormal.x);
        switch(uintLOD){
            case 0:{
                outColor = vec4(0.0, 0.0, 0.0, 1.0);
                break;
            }
            case 1:{
                outColor = vec4(0.0, 0.0, 1.0, 1.0);
                break;
            }
            case 2:{
                outColor = vec4(0.0, 0.5, 1.0, 1.0);
                break;
            }
            case 3:{
                outColor = vec4(0.0, 1.0, 1.0, 1.0);
                break;
            }
            case 4:{
                outColor = vec4(0.0, 1.0, 0.5, 1.0);
                break;
            }
            case 5:{
                outColor = vec4(0.0, 1.0, 0.0, 1.0);
                break;
            }
            case 6:{
                outColor = vec4(0.5, 1.0, 0.0, 1.0);
                break;
            }
            case 7:{
                outColor = vec4(1.0, 1.0, 0.0, 1.0);
                break;
            }
            case 8:{
                outColor = vec4(1.0, 0.5, 0.0, 1.0);
                break;
            }
            default:{
                outColor = vec4(1.0);
            }
        }
    }
    else{

        vec3 outRGB = vec3(0.41, 0.44, 0.29);
        outRGB.r = pow(outRGB.r, 2.2) * selfshadow;
        outRGB.g = pow(outRGB.g, 2.2) * selfshadow;
        outRGB.b = pow(outRGB.b, 2.2) * selfshadow;
        outRGB *= 0.6 + 0.25 * SimplexNoise(0.25 * v_in.worldPos.xy);

        outColor = vec4(outRGB, 1.0);
    }
    
    //vec3 normal = normalize(v_in.worldSpaceNormal);

    //if (!gl_FrontFacing) {
   //     normal = -normal;
    //}

    //output.normal.xyz = normalize(lerp(float3(0, 0, 1), normal, 0.25));

    //return output;
}
