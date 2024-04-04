#version 460
#include "oceanography.h.slang"

static const float gravity_sqrd = 96.2361;

RWTexture2DArray<float4> frequencySpectrum;

struct SpectrumParameters {
    float scale;
    float spreadBlend;
    float swell;
    float windSpeed;
    float peakEnhancement;
    float shortWavesFade;
    float fetch;
    float windDirection;
};

//layout(std430, binding = 0) readonly buffer SpectrumData {
//    SpectrumParameters spectrumData;
//};
ConstantBuffer<SpectrumParameters> spectrumData;

layout(push_constant) uniform Push {
    float4 lengthScale;
    float4 cutoffLow;
    float4 cutoffHigh;
    float depth;
} push;

float NormalizationFactor(float s)
{
    float s2 = s * s;
    float s3 = s2 * s;
    float s4 = s2 * s2;

    uint boolSelect = s < 5.0;
    const float coefficients[2][5] = {
        { -0.000564, 0.00776, -0.044, 0.192, 0.163 },
        { -4.80e-08, 1.07e-05, -9.53e-04, 5.90e-02, 3.93e-01}
    };


    return coefficients[boolSelect][0] * s4 + coefficients[boolSelect][1] * s3 + coefficients[boolSelect][2] * s2 + coefficients[boolSelect][3] * s + coefficients[boolSelect][4];
}

float Cosine2s(float theta, float s) {
    return NormalizationFactor(s) * pow(abs(cos(0.5 * theta)), 2.0 * s);
}

float SpreadPowerHasselman(float dispersion, float peakDispersion, float windSpeed)
{
    if (dispersion > peakDispersion)
    {
        return 9.77 * pow(abs(dispersion / peakDispersion), -2.33 - 1.45 * (windSpeed * peakDispersion / gravity - 1.17));
    }
    else
    {
        return 6.97 * pow(abs(dispersion / peakDispersion), 4.06);
    }
}

float DirectionSpectrum(float waveAngle, float dispersion, float peakDispersion, uint parIndex)
{
    const float s = SpreadPowerHasselman(dispersion, peakDispersion, spectrumData.windSpeed) 
	+ lerp(16.0 * tanh(min(dispersion / peakDispersion / 10.0, 20.0)), 25.0, spectrumData.swell) * spectrumData.swell * spectrumData.swell;

    const float spread = Cosine2s(waveAngle, s);

    return lerp(0.5 / M_PI, spread, spectrumData.spreadBlend);
}

float DirectionSpectrum(float waveAngle, float dispersion, float peakDispersion)
{
    const float s = SpreadPowerHasselman(dispersion, peakDispersion, spectrumData.windSpeed) 
	+ lerp(16.0 * tanh(min(dispersion / peakDispersion / 10.0, 20.0)), 25.0, spectrumData.swell) * spectrumData.swell * spectrumData.swell;

    float spread = Cosine2s(waveAngle, s);

    return lerp(0.5 / M_PI, spread, spectrumData.spreadBlend);
}

float TMACorrection(const float dispersion, const float depth)
{
    // approximate <4% error
    const float dispersionH = dispersion * sqrt(depth / gravity);
    if (dispersionH <= 1.0)
    {
        return 0.5 * dispersionH * dispersionH;
    }
    else if (dispersionH < 2.0)
    {
        return 1.0 - 0.5 * (2.0 - dispersionH) * (2.0 - dispersionH);
    }
    else
    {
        return 1;
    }
}

// i need to figure out what alpha represents and rename it
// horvath doesnt refer to it by a name other than alpha
float JonswapAlpha(const float chi) {
    return 0.076 * pow(chi, -0.22);
}
float JonswapPeakDispersion(const float chi, const float windSpeed)
{
    return M_2_PI * 3.5 * pow(chi, -0.33) * gravity / windSpeed;
}

float JONSWAP(const float dispersion, const float peakDispersion, const float chi, const float peakEnhancement)
{
    //const float sigma = float(dispersion > peakDispersion) * 0.02 + 0.07;
    const float sigma = select(dispersion > peakDispersion, 0.09, 0.07);

    const float r = exp(
        -(dispersion - peakDispersion) * (dispersion - peakDispersion)
				/ (2.0 * sigma * sigma * peakDispersion * peakDispersion)
			  );

    const float peakDispersionOverDispersion = peakDispersion / dispersion;
    return JonswapAlpha(chi) * gravity_sqrd
		/ (dispersion * dispersion * dispersion * dispersion * dispersion)
		* exp(-1.25 * peakDispersionOverDispersion * peakDispersionOverDispersion * peakDispersionOverDispersion * peakDispersionOverDispersion)
		* pow(abs(peakEnhancement), r) * 3.3 / peakEnhancement;
}

float FrequencyDerivative(const float waveVecMag, const float depth, const float freq)
{
    const float theta = min(waveVecMag * depth, 10.0);
    const float th = tanh(theta);
    const float ch = cosh(theta);

    return (depth * (sigmaOverRho * waveVecMag * waveVecMag * waveVecMag + gravity * waveVecMag) / (ch * ch)
		+ (gravity + 3.0 * sigmaOverRho * waveVecMag * waveVecMag) * th) * 0.5 / freq;
}

float ShortWavesFade(const float waveVecMag, const float shortWavesFade)
{
    return exp(-shortWavesFade * shortWavesFade * waveVecMag * waveVecMag);
}

// waveAngle to windDirection difference
float CalculateSpectrum(const float dispersion, const float waveWindAngle, const uint parsIndex)
{
    const float chi = min(1e4, abs(gravity * spectrumData.fetch * 1000.0 / (spectrumData.windSpeed * spectrumData.windSpeed)));

    const float peakDispersion = JonswapPeakDispersion(chi, spectrumData.windSpeed);

    return 
		JONSWAP(dispersion, peakDispersion, chi, spectrumData.peakEnhancement) 
		* TMACorrection(dispersion, push.depth)
		* DirectionSpectrum(waveWindAngle, dispersion, peakDispersion, parsIndex);
}

// random value
float2 box_muller_22(uint2 inVals) //, float mean, float std_deviation)
// mean is 0, std_deviation is 1, might as well just hard code it
{

    inVals = inVals * 1664525u + 1013904223u;

    inVals.x += inVals.y * 1664525u;
    inVals.y += inVals.x * 1664525u;

    inVals = inVals ^ (inVals >> 16u);

    inVals.x += inVals.y * 1664525u;
    inVals.y += inVals.x * 1664525u;

    float2 uniform_distr_vals = float2(inVals ^ (inVals >> 16u));
    uniform_distr_vals /= float(0xffffffffu);

    const float epsilon = 0.00001;

    float u1 = -2.0 * log(clamp(uniform_distr_vals.x, epsilon, 1.0));
    float u2 = uniform_distr_vals.y;
    // float mag = std_deviation * sqrt(u1);
    // return mag * float2(cos(M_2_PI * u2) + mean, sin(M_2_PI * u2) + mean);
    return sqrt(u1) * float2(cos(M_2_PI * u2), sin(M_2_PI * u2));
}

float GetCascadeContribution(const float waveLength, const uint cascade, const float4 cutoffLow, const float4 cutoffHigh)
{
    float cpCount = 0.0;
    float participation = float(waveLength <= cutoffHigh[cascade] && waveLength > cutoffLow[cascade]);

    [unroll] for (uint i = 0; i < CASCADE_COUNT; i++)
    {
        cpCount += float(waveLength <= cutoffHigh[i] && waveLength > cutoffLow[i]);
    }
    return participation / cpCount;
}

[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    float2 pos = float2(id.xy) - float(WAVE_COUNT) / 2.0;
    float2 boxValues = box_muller_22(id.xy); //, 0.0, 1.0);

    const uint cascade = id.z;

    const float waveAngFreq = M_2_PI / push.lengthScale[cascade];
    float2 waveVec = pos * waveAngFreq;
    float waveVecMag = length(waveVec);

    float contribution = GetCascadeContribution(waveVecMag, cascade, push.cutoffLow, push.cutoffHigh);
    if (contribution > 0.0)
    // if(waveVecMag > Get(cutoffLow) && waveVecMag <= Get(cutoffHigh))
    {

        float waveAngle = atan2(waveVec.y, waveVec.x);
        float dispersion = Frequency(waveVecMag, push.depth);
        float dispersionDerivative = FrequencyDerivative(waveVecMag, push.depth, dispersion);

        float spectrum = CalculateSpectrum(dispersion, waveAngle - spectrumData.windDirection, 0) 
                            * ShortWavesFade(waveVecMag, spectrumData.shortWavesFade)
                            * spectrumData.scale;
        /*
        if(spectrums[1].scale > 0.001)
        {
            spectrum += CalculateSpectrum(dispersion, waveAngle - spectrums[1].windDirection, 1)
                            * ShortWavesFade(waveVecMag, spectrums[1].shortWavesFade)
                            * spectrums[1].scale;
        }
        */

        spectrum = sqrt(2.0 * spectrum * abs(dispersionDerivative / waveVecMag) * waveAngFreq * waveAngFreq);

        float4 initialFrequency;
        initialFrequency.xy = boxValues * spectrum;

        // currently im double calculating
        // alternative is to write once, then read the WAVE_COUNT - id values, then write it back
        // im not sure if i can do that with a barrier and shared memory
        waveVec.x = (WAVE_COUNT - id.x) - float(WAVE_COUNT) / 2.0;
        waveVec.y = (WAVE_COUNT - id.y) - float(WAVE_COUNT) / 2.0;
        waveVec = waveVec * waveAngFreq;
        waveVecMag = length(waveVec);
        contribution = GetCascadeContribution(waveVecMag, cascade, push.cutoffLow, push.cutoffHigh);

        initialFrequency.z = 0.0; // covering if the branch is false
        initialFrequency.w = 0.0;
        if (contribution > 0.0) { //test branch removal, by writing a float2 instead of a float4
            waveAngle = atan2(waveVec.y, waveVec.x);
            dispersion = Frequency(waveVecMag, push.depth);
            dispersionDerivative = FrequencyDerivative(waveVecMag, push.depth, dispersion);

            spectrum = CalculateSpectrum(dispersion, waveAngle - spectrumData.windDirection, 0) 
                        * ShortWavesFade(waveVecMag, spectrumData.shortWavesFade)
                        * spectrumData.scale;
            spectrum = sqrt(2.0 * spectrum * abs(dispersionDerivative) / waveVecMag * waveAngFreq * waveAngFreq);

            initialFrequency.z = boxValues.x * spectrum;
            initialFrequency.w = boxValues.y * -spectrum;
        }
        frequencySpectrum[id] = initialFrequency;
    }
    else {
        frequencySpectrum[id] = float4(0.0, 0.0, 0.0, 0.0);
    }
    
}