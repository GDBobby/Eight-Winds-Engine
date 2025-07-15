

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

//https://www.shadertoy.com/view/XsX3zB

/* discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 */
vec3 random3(vec3 c) {
	float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0*j);
	j *= .125;
	r.x = fract(512.0*j);
	j *= .125;
	r.y = fract(512.0*j);
	return r-0.5;
}

//skew constants
const float F3 =  0.3333333;
const float G3 =  0.1666667;

float SimplexNoise3(vec3 p) {
	 /* 1. find current tetrahedron T and it's four vertices */
	 /* s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices */
	 /* x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices*/
	 
	 /* calculate s and x */
	 vec3 s = floor(p + dot(p, vec3(F3)));
	 vec3 x = p - s + dot(s, vec3(G3));
	 
	 /* calculate i1 and i2 */
	 vec3 e = step(vec3(0.0), x - x.yzx);
	 vec3 i1 = e*(1.0 - e.zxy);
	 vec3 i2 = 1.0 - e.zxy*(1.0 - e);
	 	
	 /* x1, x2, x3 */
	 vec3 x1 = x - i1 + G3;
	 vec3 x2 = x - i2 + 2.0*G3;
	 vec3 x3 = x - 1.0 + 3.0*G3;
	 
	 /* 2. find four surflets and store them in d */
	 vec4 w, d;
	 
	 /* calculate surflet weights */
	 w.x = dot(x, x);
	 w.y = dot(x1, x1);
	 w.z = dot(x2, x2);
	 w.w = dot(x3, x3);
	 
	 /* w fades from 0.6 at the center of the surflet to 0.0 at the margin */
	 w = max(0.6 - w, 0.0);
	 
	 /* calculate surflet components */
	 d.x = dot(random3(s), x);
	 d.y = dot(random3(s + i1), x1);
	 d.z = dot(random3(s + i2), x2);
	 d.w = dot(random3(s + 1.0), x3);
	 
	 /* multiply d by w^4 */
	 w *= w;
	 w *= w;
	 d *= w;
	 
	 /* 3. return the sum of the four surflets */
	 return dot(d, vec4(52.0));
}


float hybrid_multi_fractal_processing(vec3 point, const float step, const uint octave, const float lacunarity, const float hmf_H, const float hmf_offset) {
	vec3 temp_point = point;

	float hmf_exponent_array[16];
	float frequency = 1.0;
	float result, signal, weight;
	
	for (uint i = 0; i < octave; i++) {
		// compute weight for each frequency
		hmf_exponent_array[i] = pow(frequency, -hmf_H);
		frequency *= lacunarity;
	}

	// get first octave of function
	result = (SimplexNoise3(temp_point * step) + hmf_offset) * hmf_exponent_array[0];
	weight = result;
	// increase frequency
	temp_point = temp_point * lacunarity;
	// spectral construction inner loop, where the fractal is built
	for (uint i = 1; i < octave; i++) {
		// prevent divergence
		if (weight > 1.0) weight = 1.0;
		// get next frequency
		signal = (SimplexNoise3(temp_point * step) + hmf_offset) * hmf_exponent_array[i];
		result += weight * signal;
		// this is why hmf_H must specify a high fractal dimension
		weight *= signal;
		temp_point = temp_point * lacunarity;
	}

	return result;
}

float NoiseWithOctaves(vec2 uv, const int octaves){
	float freq = 0.5;
    const mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );
	float f  = 0.5 * SimplexNoise( uv ); 
	for(int i = 1; i < octaves; i++){
		freq /= 2.0;
		uv = m * uv;
		f += freq * SimplexNoise(uv);
	}

	return f;
}