#pragma once

#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Device_Buffer.h"

#include <glm/glm.hpp>
#include <array>

namespace EWE {
    namespace Ocean {
        constexpr uint32_t OCEAN_WAVE_COUNT = 256;
        constexpr float smallestWaveMultiplier = 4.f;
        constexpr float minWavesInCascade = 6.f;
        constexpr float O_PI = 3.14159265358979323846264338327950288f;

        enum Pipe_Enum : uint16_t {
            Pipe_precompute_twiddle = 0,

            Pipe_permute,
            Pipe_scale,

            Pipe_horizontal_fft,
            Pipe_horizontal_inverse_fft,
            Pipe_vertical_fft,
            Pipe_vertical_inverse_fft,

            Pipe_initial_spectrum,
            Pipe_conjugate_spectrum,
            Pipe_time_dependent_spectrum,
            Pipe_textures_merger,

            Pipe_Enum_size,
        };

        struct SSSDataBuffer
        {
            float mNormalStrength = 1.f;
            float mSunStrength = 0.f;
            float mEnvironmentStrength = 0.f;
            float mFadeDistance = 3.f;
            float mHeightBias = 0.f;
            float mSpread = 0.2f;
            float m__Padding0;
            float m__Padding1;
            glm::vec3 mColor{ 0.13333334f, 0.9411765f, 0.6039216f };
            float  m__Padding2;
            glm::vec3 mColorMulti{ 0.0f, 0.025490196f, 0.02745098f };
        };
        struct FoamRenderData
        {
            float mUnderwaterFoamParallax{ 1.2f };
            float mNormalDetail{ 0.5f };
            float mDensity{ 8.4f };
            float mUnderwaterTexScale{ 0.2f };
            float mSharpness{ 0.5f };
            float  mPersistence{ 0.86f };
            float  mCoverage{ 0.709f };
            float  mUnderwater{ 0.36f };
            float  mBias{ 0.f }; //if there is a foam detail map, remove this
            //should probably pack bias into the albedo's alpha channel, right now its only reading rgb
            float m__Padding4[3];

            glm::vec3 mTint{ 0.66087574f, 0.7406194f, 0.7924528f };
            float  m__Padding0;
            glm::vec3 mAlbedo{ 0.49302f, 0.72549f, 1.0f }; //if there is an albedo texture, remove this
            float m__Padding1; //not sure if this is necessary
            glm::vec4 mCascadeWeights{ 0.0f, 1.0f, 0.5f, 0.3f };
            glm::vec4 mNormalWeights{ 1.0f, 0.66f, 0.33f, 0.0f };
        };
        struct OceanFragmentData
        {
            glm::vec4  mLengthScales;
            float mWindSpeed;
            float mWaveScale{ 1.f };
            float mWaveAlignment{ 1.f };
            float mReferenceWaveHeight{ 1.f };
            glm::vec2 mWindDirection{ 1.f, 0.f };

            // stuffing these in here for simplicity, remove later
            float mHorizon_fog{ 0.f };

            OceanFragmentData(const glm::vec4 lengthScale) : mLengthScales{ lengthScale } {
            }
        };
        struct IntialFrequencySpectrumPushData {

            glm::vec4 mLengthScale;
            glm::vec4 mCutoffLow;
            glm::vec4 mCutoffHigh;
            float  mDepth;

            IntialFrequencySpectrumPushData() {
                const float lengthScaleMultiplier = smallestWaveMultiplier * minWavesInCascade / static_cast<float>(OCEAN_WAVE_COUNT);
                mLengthScale[0] = 400.f;
                mLengthScale[1] = mLengthScale[0] * lengthScaleMultiplier;
                mLengthScale[2] = mLengthScale[1] * lengthScaleMultiplier;
                mLengthScale[3] = mLengthScale[2] * lengthScaleMultiplier;

                const float highMulti = 2.f * O_PI * static_cast<float>(OCEAN_WAVE_COUNT) / smallestWaveMultiplier;
                mCutoffHigh[0] = highMulti / mLengthScale[0];
                mCutoffHigh[1] = highMulti / mLengthScale[1];
                mCutoffHigh[2] = highMulti / mLengthScale[2];
                mCutoffHigh[3] = highMulti / mLengthScale[3];

                const float lowMulti = O_PI * 2 * minWavesInCascade;
#if 0 // ALLOW_OVERLAP
                mCutoffLow[0] = lowMulti / mLengthScale[0];
                mCutoffLow[1] = lowMulti / mLengthScale[1];
                mCutoffLow[2] = lowMulti / mLengthScale[2];
                mCutoffLow[3] = lowMulti / mLengthScale[3];
#else
                mCutoffLow[0] = 0.f;
                mCutoffLow[1] = glm::max(lowMulti / mLengthScale[1], mCutoffHigh[0]);
                mCutoffLow[2] = glm::max(lowMulti / mLengthScale[2], mCutoffHigh[1]);
                mCutoffLow[3] = glm::max(lowMulti / mLengthScale[3], mCutoffHigh[2]);
#endif
            }
        };
        struct TimeDependentFrequencySpectrumPushData {
            glm::vec4 mLengthScale;
            glm::vec4 mCutoffLow;
            glm::vec4 mCutoffHigh;
            float  mDepth;
            float  mTime;
            TimeDependentFrequencySpectrumPushData(IntialFrequencySpectrumPushData const& ifsData) : mTime{0.f} {
                memcpy(this, &ifsData, sizeof(glm::vec3) * 3 + sizeof(float));

            }
        };
        struct JONSWAP_Parameters
        {
            float mScale{ 1.f };
            float mSpreadBlend{ 1.f };
            float mSwell{ 0.f };
            float mWindSpeed{ 5.f };
            float mPeakEnhancement{ 3.3f };
            float mShortWavesFade{ 0.1f };
            float mFetch{ 100.f };
            float mWindDirection{ -29.81 / 180.f * O_PI };
        };


        struct InitialFrequencySpectrumGPUData {
            VkPipeline pipeline = VK_NULL_HANDLE;
            VkPipelineLayout pipeLayout = VK_NULL_HANDLE;
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            JONSWAP_Parameters jonswapParams{};
            IntialFrequencySpectrumPushData pushData{};

            EWEBuffer* jonswapBuffer[2];
            InitialFrequencySpectrumGPUData() {

            }
        };
    } //namespace ocean
} //namespace EWE