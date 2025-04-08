#version 460
#extension GL_NV_mesh_shader : require

layout(location = 0) in PerVertexData{
    vec3 worldSpacePosition;
    vec3 worldSpaceNormal;
    float rootHeight;
    float height;
} v_in;

layout(location = 0) out vec4 outColor;

vec2 SimplexHash(vec2 p ) { // replace this by something better {
	p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

//https://www.shadertoy.com/view/Msf3WH
float SimplexNoise(const vec2 p) {
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;

	vec2  i = floor( p + (p.x+p.y)*K1 );
    vec2  a = p - i + (i.x+i.y)*K2;
    float m = step(a.y,a.x); 
    vec2  o = vec2(m,1.0-m);
    vec2  b = a - o + K2;
	vec2  c = a - 1.0 + 2.0*K2;
    vec3  h = max( 0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );
	vec3  n = h*h*h*h*vec3( dot(a,SimplexHash(i+0.0)), dot(b,SimplexHash(i+o)), dot(c,SimplexHash(i+1.0)));
    return dot( n, vec3(70.0) );
}


void main(){
    	
    float selfshadow = clamp(pow((v_in.worldSpacePosition.y - v_in.rootHeight) / v_in.height, 1.5), 0, 1);

    vec3 outRGB = vec3(0.41, 0.44, 0.29);
    outRGB.r = pow(outRGB.r, 2.2) * selfshadow;
    outRGB.g = pow(outRGB.g, 2.2) * selfshadow;
    outRGB.b = pow(outRGB.b, 2.2) * selfshadow;
    outRGB *= 0.75 + 0.25 * SimplexNoise(0.25 * v_in.worldSpacePosition.xy);

    outColor = vec4(outRGB, 1.0);
    
    //vec3 normal = normalize(v_in.worldSpaceNormal);

    //if (!gl_FrontFacing) {
   //     normal = -normal;
    //}

    //output.normal.xyz = normalize(lerp(float3(0, 0, 1), normal, 0.25));

    //return output;
}
