C:\Programming\slang-2025.6.3\bin\slangc.exe InitialFrequencySpectrum.comp.slang -profile glsl_460 -target spirv -o ../InitialFrequencySpectrum.comp.spv -emit-spirv-directly -O3 -entry computeMain
REM C:\Programming\slang-2024.14\bin\slangc.exe InitialFrequencySpectrum.comp.slang -profile glsl_460 -target glsl -o debug/d_InitialFrequencySpectrum.comp.spv -entry computeMain

C:\Programming\slang-2025.6.3\bin\slangc.exe TimeDependentSpectrum.comp.slang -profile glsl_460 -target spirv -o ../TimeDependentSpectrum.comp.spv -O3 -emit-spirv-directly -entry computeMain
REM C:\Programming\slang-2024.14\bin\slangc.exe TimeDependentSpectrum.comp.slang -profile glsl_460 -target spirv -o debug/d_TimeDependentSpectrum.comp.spv -O3 -emit-spirv-directly -g2 -entry computeMain
REM C:\Programming\slang-2024.14\bin\slangc.exe TimeDependentSpectrum.comp.slang -profile glsl_460 -target glsl -o debug/dg_TimeDependentSpectrum.comp.spv -entry computeMain


C:\Programming\slang-2025.6.3\bin\slangc.exe OceanFFT.comp.slang -profile glsl_460 -target spirv -o ../OceanFFT.comp.spv -O3 -emit-spirv-directly -entry computeMain
REM C:\Programming\slang-2024.14\bin\slangc.exe OceanFFT.comp.slang -profile glsl_460 -target spirv -o debug/d_OceanFFT.comp.spv -O3 -emit-spirv-directly -g2 -entry computeMain

C:\Programming\slang-2025.6.3\bin\slangc.exe computeSpecularMap.comp.slang -profile glsl_460 -target spirv -o ../computeSpecularMap.comp.spv -O3 -emit-spirv-directly -entry computeMain

C:\Programming\slang-2025.6.3\bin\slangc.exe InitializeFoam.comp.slang -profile glsl_460 -target spirv -o ../InitializeFoam.comp.spv -O3 -emit-spirv-directly -entry computeMain

C:\Programming\slang-2025.6.3\bin\slangc.exe ocean.vert.slang -profile glsl_460 -target spirv -o ../ocean.vert.spv -O3 -emit-spirv-directly -entry vertexMain

C:\Programming\slang-2025.6.3\bin\slangc.exe ocean.frag.slang -profile glsl_460 -target spirv -o ../ocean.frag.spv -O3 -emit-spirv-directly -entry fragmentMain