#pragma once

#include <glm/glm.hpp>
#include <array>

namespace EWE {
    namespace Ocean {
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


#define PI 3.1415926535897932384626433832795f

        struct Spectrum_Settings {
            float scale{};
            float angle{};
            float spreadBlend{};
            float swell{};
            float alpha{};
            float peakOmega{};
            float gamma{};
            float shortWavesFade{};
        };

        struct Ocean_Draw_Push_Constant {
            glm::mat4 modelMatrix{};
            float time{};
        };
        struct Cascade_Parameters {
            int size{ 1024 };
            float lengthScale{};
            float cutoffHigh{};
            float cutoffLow{};
            float gravityAcceleration{9.81f};
            float depth{500.f};
        };
        struct Derivative_Data {
            glm::vec2 data[5]{};
        };

        struct Ocean_Ubo {
            float LengthScale0{};
            float LengthScale1{};
            float LengthScale2{};
            float LOD_scale{1};
            float SSSBase{};
            float SSSScale{};
        };

        struct Ocean_Material {
            glm::vec4 color{0.03457636f, 0.12297464f, 0.1981132f, 1.f };
            glm::vec4 foamColor{ 1.f };
            glm::vec4 SSSColor{ 0.1541919f, 0.8857628f, 0.990566f, 1.f };
            float SSSStrength{ 0.133f };
            float roughness{ 0.311f };
            float roughnessScale{ 0.0044f };
            float maxGloss{ 0.91f };
            float FoamBiasLOD0{ 0.84f };
            float FoamBiasLOD1{ 1.83f };
            float FoamBiasLOD2{ 2.72f };
            float FoamScale{ 2.4f };
            float ContactFoam{ 1.f };
        };

        struct Ocean_Time_Struct {
            float lambda{ 0.f };
            float time{ 0.f };
            int size{ 0 };
            float dt{ 0.f };
        };

        struct Display_Spectrum_Settings {
            //[Range(0, 1)]
            float scale{};
            float windSpeed{};
            float windDirection{};
            float fetch{};
            //[Range(0, 1)]
            float spreadBlend{};
            //[Range(0, 1)]
            float swell{};
            float peakEnhancement{};
            float shortWavesFade{};
        };

        struct Wave_Settings {
            float gravity{ 9.81f };
            float depth{ 500.f };
            //[Range(0, 1)]
            float lambda{ 1.f };
            Display_Spectrum_Settings local_display{};
            Display_Spectrum_Settings swell_display{};
            std::array<Spectrum_Settings, 2> spectrums{};

            Wave_Settings() {
                local_display.scale = 1.f;
                local_display.windSpeed = 0.5f;
                local_display.windDirection = -29.81f;
                local_display.fetch = 100000.f;
                local_display.spreadBlend = 0.198f;
                local_display.swell = 0.198f;
                local_display.peakEnhancement = 3.3f;
                local_display.shortWavesFade = 0.01f;

                swell_display.scale = 0.f;
                swell_display.windSpeed = 1.f;
                swell_display.windDirection = 0.f;
                swell_display.fetch = 300000.f;
                swell_display.spreadBlend = 1.f;
                swell_display.swell = 1.f;
                swell_display.peakEnhancement = 3.3f;
                swell_display.shortWavesFade = 0.01f;

                FillSettingsStruct();
            }


            /*
             void SetParametersToShader(ComputeShader shader, int kernelIndex, ComputeBuffer paramsBuffer) {
                shader.SetFloat(G_PROP, g);
                shader.SetFloat(DEPTH_PROP, depth);

                FillSettingsStruct(local, ref spectrums[0]);
                FillSettingsStruct(swell, ref spectrums[1]);

                paramsBuffer.SetData(spectrums);
                shader.SetBuffer(kernelIndex, SPECTRUMS_PROP, paramsBuffer);
            }
            */

            void FillSettingsStruct() {
                spectrums[0].scale = local_display.scale;
                spectrums[0].angle = local_display.windDirection / 180.f * PI;
                spectrums[0].spreadBlend = local_display.spreadBlend;
                spectrums[0].swell = glm::clamp(local_display.swell, 0.01f, 1.0f);
                //settings.swell = Mathf.Clamp(display.swell, 0.01f, 1);
                spectrums[0].alpha = JonswapAlpha(gravity, local_display.fetch, local_display.windSpeed);
                spectrums[0].peakOmega = JonswapPeakFrequency(gravity, local_display.fetch, local_display.windSpeed);
                spectrums[0].gamma = local_display.peakEnhancement;
                spectrums[0].shortWavesFade = local_display.shortWavesFade;

                spectrums[1].scale = swell_display.scale;
                spectrums[1].angle = swell_display.windDirection / 180.f * PI;
                spectrums[1].spreadBlend = swell_display.spreadBlend;
                spectrums[1].swell = glm::clamp(swell_display.swell, 0.01f, 1.0f);
                //settings.swell = Mathf.Clamp(display.swell, 0.01f, 1);
                spectrums[1].alpha = JonswapAlpha(gravity, swell_display.fetch, swell_display.windSpeed);
                spectrums[1].peakOmega = JonswapPeakFrequency(gravity, swell_display.fetch, swell_display.windSpeed);
                spectrums[1].gamma = swell_display.peakEnhancement;
                spectrums[1].shortWavesFade = swell_display.shortWavesFade;
            }

            float JonswapAlpha(float g, float fetch, float windSpeed) {
                return 0.076f * glm::pow(g * fetch / windSpeed / windSpeed, -0.22f);
            }

            float JonswapPeakFrequency(float g, float fetch, float windSpeed) {
                return 22.f * glm::pow(windSpeed * fetch / g / g, -0.33f);
            }
        };
    }
}