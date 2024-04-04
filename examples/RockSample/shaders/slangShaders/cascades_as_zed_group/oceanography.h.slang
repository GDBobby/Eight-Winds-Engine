
#ifndef OCEANOGRAPHY_H
#define OCEANOGRAPHY_H
static const float M_2_PI = 6.283185307179586476925286766559;
static const float M_PI = 3.1415926535897932384626433832795;
static const float gravity = 9.81;
static const float sigmaOverRho = 7.4e-5; // this is surface tension divided by density

static const uint WAVE_COUNT = 256;
static const uint LOG_2_VALUE = 8; // this must be equal to log2(WAVE_COUNT)

#define CASCADE_COUNT 4 // needs to match the cpu value

typedef float2 Complex

Complex ComplexMult(const Complex a, const Complex b)
{
    return float2(a.r * b.r - a.g * b.g, a.r * b.g + a.g * b.r);
}
Complex ComplexExp(const Complex a)
{
    return Complex(cos(a.y), sin(a.y)) * exp(a.x);
}

float Frequency(const float waveVecMag, const float depth)
{
    // https://en.wikipedia.org/wiki/Dispersion_(water_waves)
    // i think that with enough of a render budget, a switch statement would be used to define deep water, shallow water, and capillary waves
    return sqrt(gravity * waveVecMag * tanh(waveVecMag * depth));
}

float EaseInOutClamped(const float x)
{
    const float tempX = saturate(x);
    return 3.0 * tempX * tempX - 2.0 * tempX * tempX * tempX;
}
float4 GetLodWeights(const float viewDist, const float4 lengthScales, const float lodScale)
{
    float4 weights;

    [ForceUnroll] for (uint cascade = 0; cascade < CASCADE_COUNT; cascade++)
    {
        weights[cascade] = 1.0 - EaseInOutClamped(viewDist / (lengthScales[cascade] * lodScale) - 1.0);
    }
    return weights;
}

#endif