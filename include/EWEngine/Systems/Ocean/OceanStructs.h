#pragma once

#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Device_Buffer.h"
#include "EWEngine/Graphics/Pipeline.h"
#include "EWEngine/Graphics/Model/Model.h"

#include <glm/glm.hpp>
#include <array>

namespace EWE {
    namespace Ocean {
        constexpr uint32_t OCEAN_WAVE_COUNT = 256;
        constexpr uint32_t LOCAL_WORK_GROUP_SIZE = 32;
        constexpr float smallestWaveMultiplier = 4.f;
        constexpr float minWavesInCascade = 6.f;
        constexpr float O_PI = 3.14159265358979323846264338327950288f;

        struct IntialFrequencySpectrumPushData {

            glm::vec4 mLengthScale;
            glm::vec4 mCutoffLow;
            glm::vec4 mCutoffHigh;
            float  mDepth;

            IntialFrequencySpectrumPushData();
        };
        struct TimeDependentFrequencySpectrumPushData {
            glm::vec4 mLengthScale;
            glm::vec4 mCutoffLow;
            glm::vec4 mCutoffHigh;
            float  mDepth;
            float  mTime;
            TimeDependentFrequencySpectrumPushData() : mTime{ 0.f } {}
            void CopyFromIFS(IntialFrequencySpectrumPushData const& ifsData) {
                memcpy(this, &ifsData, sizeof(IntialFrequencySpectrumPushData));
            }
        };
        struct JONSWAP_Parameters {
            float mScale{ 1.f };
            float mSpreadBlend{ 1.f };
            float mSwell{ 0.f };
            float mWindSpeed{ 40.f };
            float mPeakEnhancement{ 3.3f };
            float mShortWavesFade{ 0.1f };
            float mFetch{ 100.f };
            float mWindDirection{ -29.81 / 180.f * O_PI };
        };


        struct InitialFrequencySpectrumGPUData {
            VkPipeline pipeline = VK_NULL_HANDLE;
            VkPipelineLayout pipeLayout = VK_NULL_HANDLE;
            VkDescriptorSet descriptorSet[2] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
            JONSWAP_Parameters jonswapParams{};
            VkShaderModule shaderModule = VK_NULL_HANDLE;
            IntialFrequencySpectrumPushData pushData{};
            EWEDescriptorSetLayout* eweDSL{ nullptr };

            EWEBuffer* jonswapBuffer = nullptr;
            InitialFrequencySpectrumGPUData();
            ~InitialFrequencySpectrumGPUData();

            void Compute(FrameInfo const& frameInfo);
            void CreateDescriptorSet(VkDescriptorImageInfo* descImageInfo);
        private:
            void CreatePipeLayout();
            void CreatePipeline();
            void CreateBuffers();
        };
        struct TimeDependentFrequencySpectrumGPUData {
            VkPipeline pipeline = VK_NULL_HANDLE;
            VkPipelineLayout pipeLayout = VK_NULL_HANDLE;
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            VkShaderModule shaderModule = VK_NULL_HANDLE;
            TimeDependentFrequencySpectrumPushData pushData{};
            EWEDescriptorSetLayout* eweDSL{ nullptr };

            TimeDependentFrequencySpectrumGPUData();
            ~TimeDependentFrequencySpectrumGPUData();

            void CreateDescriptorSet(VkDescriptorImageInfo* frequencyImage, VkDescriptorImageInfo* outputImage);
            void Compute(FrameInfo const& frameInfo, float dt);
        private:
            void CreatePipeLayout();
            void CreatePipeline();
        };
        struct FFTPushData {
            float deltaTime;
            float foamBias;
            int secondPass;
            FFTPushData() : deltaTime{ 0.f }, foamBias{ 1.f } {}
        };
        struct FFTGPUData {
            VkPipeline pipeline = VK_NULL_HANDLE;
            VkPipelineLayout pipeLayout = VK_NULL_HANDLE;
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            VkShaderModule shaderModule = VK_NULL_HANDLE;
            FFTPushData pushData{};
            EWEDescriptorSetLayout* eweDSL{ nullptr };
            VkImage fftImage;

            FFTGPUData();
            ~FFTGPUData();
            void Compute(FrameInfo const& frameInfo, float dt);

            void CreateDescriptorSet(VkDescriptorImageInfo* outputImage);
            void SetVkImage(VkImage image) { fftImage = image; }
        private:
            void CreatePipeLayout();
            void CreatePipeline();
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
            OceanFragmentData() {}
            OceanFragmentData(glm::vec4 const& lengthScale) : mLengthScales{ lengthScale } {
            }
        };
        struct OceanRenderParameters {
            OceanFragmentData oceanFragmentData{};
            float m__Padding;
            FoamRenderData foamDataBuffer{};
            SSSDataBuffer sssDataBuffer{};
            OceanRenderParameters() {}
        };

        struct OceanGraphicsGPUData {
            std::unique_ptr<EWEPipeline> pipe;
            VkPipelineLayout pipeLayout = VK_NULL_HANDLE;
            VkDescriptorSet descriptorSet[2] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
            std::unique_ptr<EWEModel> oceanModel{ nullptr };
            EWEBuffer* renderData[2] = { nullptr, nullptr };
            EWEDescriptorSetLayout* eweDSL{ nullptr };
            OceanRenderParameters oceanRenderParameters{};
            OceanGraphicsGPUData();
            ~OceanGraphicsGPUData();

            void Render(FrameInfo const& frameInfo);
            void CreateDescriptorSet(VkDescriptorImageInfo* outputImages, VkDescriptorImageInfo* skyboxImage);
        private:
            void CreatePipeLayout();
            void CreatePipeline();
            void CreateModel();
        };
    } //namespace ocean
} //namespace EWE