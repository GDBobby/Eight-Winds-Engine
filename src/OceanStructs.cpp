#include "EWEngine/Systems/Ocean/OceanStructs.h"

#include "EWEngine/Graphics/DescriptorHandler.h"

namespace EWE {
    namespace Ocean {
#define CASCADE_COUNT_IN_ZED_GROUP_COUNT 0 //0 or 1, false or true


        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  IFS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        IntialFrequencySpectrumPushData::IntialFrequencySpectrumPushData() {
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
            mDepth = 100.f;
        }


        InitialFrequencySpectrumGPUData::InitialFrequencySpectrumGPUData() {
            CreatePipeLayout();
            CreatePipeline();
            CreateBuffers();
        }
        InitialFrequencySpectrumGPUData::~InitialFrequencySpectrumGPUData() {
            EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, &descriptorSet[0]);
            EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, &descriptorSet[1]);

			vkDestroyPipeline(EWEDevice::GetVkDevice(), pipeline, nullptr);
			vkDestroyPipelineLayout(EWEDevice::GetVkDevice(), pipeLayout, nullptr);
			vkDestroyShaderModule(EWEDevice::GetVkDevice(), shaderModule, nullptr);

            jonswapBuffer->~EWEBuffer();
			ewe_free(jonswapBuffer);

            eweDSL->~EWEDescriptorSetLayout();
            ewe_free(eweDSL);
		}

        void InitialFrequencySpectrumGPUData::CreatePipeLayout() {
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(IntialFrequencySpectrumPushData);

            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
            pipelineLayoutInfo.pushConstantRangeCount = 1;

            EWEDescriptorSetLayout::Builder dslBuilder{};
            dslBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1);
            dslBuilder.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
            eweDSL = dslBuilder.build();

            VkDescriptorSetLayout dsLayout = eweDSL->GetDescriptorSetLayout();

            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &dsLayout;

            EWE_VK_ASSERT(vkCreatePipelineLayout(EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipeLayout));
        }
        void InitialFrequencySpectrumGPUData::CreatePipeline() {
            VkComputePipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipelineInfo.layout = pipeLayout;
            Pipeline_Helper_Functions::createShaderModule("InitialFrequencySpectrum.comp.spv", &shaderModule);
            VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
            computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            computeShaderStageInfo.module = shaderModule;
            computeShaderStageInfo.pName = "main";
            pipelineInfo.stage = computeShaderStageInfo;
            vkCreateComputePipelines(EWEDevice::GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
        }
        void InitialFrequencySpectrumGPUData::CreateBuffers() {
            jonswapBuffer = EWEBuffer::CreateAndInitBuffer(&jonswapParams, sizeof(JONSWAP_Parameters), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        }
        void InitialFrequencySpectrumGPUData::CreateDescriptorSet(VkDescriptorImageInfo* descImageInfo) {

            EWEDescriptorWriter descWriter{eweDSL, DescriptorPool_Global};
            descWriter.WriteImage(0, descImageInfo);
            descWriter.WriteBuffer(1, jonswapBuffer->DescriptorInfo());
            descriptorSet[0] = descWriter.Build();
            descriptorSet[1] = descWriter.Build();
        }
        void InitialFrequencySpectrumGPUData::Compute(FrameInfo const& frameInfo) {
            vkCmdBindPipeline(frameInfo.cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

            vkCmdBindDescriptorSets(frameInfo.cmdBuf,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                pipeLayout,
                0, 1,
                &descriptorSet[frameInfo.index],
                0, nullptr
            );

            vkCmdPushConstants(frameInfo.cmdBuf, pipeLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushData), &pushData);

#if CASCADE_COUNT_IN_ZED_GROUP_COUNT
            vkCmdDispatch(frameInfo.cmdBuf, OCEAN_WAVE_COUNT / LOCAL_WORK_GROUP_SIZE, OCEAN_WAVE_COUNT / LOCAL_WORK_GROUP_SIZE, cascade_count);
#else
            vkCmdDispatch(frameInfo.cmdBuf, OCEAN_WAVE_COUNT / LOCAL_WORK_GROUP_SIZE, OCEAN_WAVE_COUNT / LOCAL_WORK_GROUP_SIZE, 1);
#endif
        }

        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TDFS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        TimeDependentFrequencySpectrumGPUData::TimeDependentFrequencySpectrumGPUData() {
            CreatePipeLayout();
            CreatePipeline();
        }
        TimeDependentFrequencySpectrumGPUData::~TimeDependentFrequencySpectrumGPUData() {
            EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, &descriptorSet);

            vkDestroyPipeline(EWEDevice::GetVkDevice(), pipeline, nullptr);
            vkDestroyPipelineLayout(EWEDevice::GetVkDevice(), pipeLayout, nullptr);
            vkDestroyShaderModule(EWEDevice::GetVkDevice(), shaderModule, nullptr);

            eweDSL->~EWEDescriptorSetLayout();
            ewe_free(eweDSL);
        }


        void TimeDependentFrequencySpectrumGPUData::CreatePipeLayout() {
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(TimeDependentFrequencySpectrumPushData);

            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
            pipelineLayoutInfo.pushConstantRangeCount = 1;

            EWEDescriptorSetLayout::Builder dslBuilder{};
            dslBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1);
            dslBuilder.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1);
            eweDSL = dslBuilder.build();

            VkDescriptorSetLayout dsLayout = eweDSL->GetDescriptorSetLayout();

            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &dsLayout;

            EWE_VK_ASSERT(vkCreatePipelineLayout(EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipeLayout));
        }
        void TimeDependentFrequencySpectrumGPUData::CreatePipeline() {

            VkComputePipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipelineInfo.layout = pipeLayout;
            Pipeline_Helper_Functions::createShaderModule("TimeDependentSpectrum.comp.spv", &shaderModule);
            VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
            computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            computeShaderStageInfo.module = shaderModule;
            computeShaderStageInfo.pName = "main";
            pipelineInfo.stage = computeShaderStageInfo;
            vkCreateComputePipelines(EWEDevice::GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
        }
        void TimeDependentFrequencySpectrumGPUData::CreateDescriptorSet(VkDescriptorImageInfo* frequencyImage, VkDescriptorImageInfo* outputImage) {

            EWEDescriptorWriter descWriter{ eweDSL, DescriptorPool_Global };
            descWriter.WriteImage(0, frequencyImage);
            descWriter.WriteImage(1, outputImage);
            descriptorSet = descWriter.Build();
        }
        void TimeDependentFrequencySpectrumGPUData::Compute(FrameInfo const& frameInfo, float dt) {
            vkCmdBindPipeline(frameInfo.cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

            vkCmdBindDescriptorSets(frameInfo.cmdBuf,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                pipeLayout,
                0, 1,
                &descriptorSet,
                0, nullptr
            );
            pushData.mTime += dt;
            vkCmdPushConstants(frameInfo.cmdBuf, pipeLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushData), &pushData);

#if CASCADE_COUNT_IN_ZED_GROUP_COUNT
            vkCmdDispatch(frameInfo.cmdBuf, OCEAN_WAVE_COUNT / LOCAL_WORK_GROUP_SIZE, OCEAN_WAVE_COUNT / LOCAL_WORK_GROUP_SIZE, cascade_count);
#else
            vkCmdDispatch(frameInfo.cmdBuf, OCEAN_WAVE_COUNT / LOCAL_WORK_GROUP_SIZE, OCEAN_WAVE_COUNT / LOCAL_WORK_GROUP_SIZE, 1);
#endif
        }


        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ FFT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


        FFTGPUData::FFTGPUData() {
            CreatePipeLayout();
            CreatePipeline();
        }
        FFTGPUData::~FFTGPUData() {
            eweDSL->~EWEDescriptorSetLayout();
            ewe_free(eweDSL);
        }

        void FFTGPUData::CreateDescriptorSet(VkDescriptorImageInfo* outputImage) {
            EWEDescriptorWriter descWriter{ eweDSL, DescriptorPool_Global };
            descWriter.WriteImage(0, outputImage);
            descriptorSet = descWriter.Build();
        }

        void FFTGPUData::CreatePipeLayout() {
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(FFTPushData);

            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
            pipelineLayoutInfo.pushConstantRangeCount = 1;

            EWEDescriptorSetLayout::Builder dslBuilder{};
            dslBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1);
            eweDSL = dslBuilder.build();

            VkDescriptorSetLayout dsLayout = eweDSL->GetDescriptorSetLayout();

            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &dsLayout;

            EWE_VK_ASSERT(vkCreatePipelineLayout(EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipeLayout));
        }
        void FFTGPUData::CreatePipeline() {

            VkComputePipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipelineInfo.layout = pipeLayout;
            Pipeline_Helper_Functions::createShaderModule("OceanFFT.comp.spv", &shaderModule);
            VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
            computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            computeShaderStageInfo.module = shaderModule;
            computeShaderStageInfo.pName = "main";
            pipelineInfo.stage = computeShaderStageInfo;
            vkCreateComputePipelines(EWEDevice::GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
        }
        void FFTGPUData::Compute(FrameInfo const& frameInfo, float dt) {
            vkCmdBindPipeline(frameInfo.cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

            vkCmdBindDescriptorSets(frameInfo.cmdBuf,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                pipeLayout,
                0, 1,
                &descriptorSet,
                0, nullptr
            );
            pushData.deltaTime = dt;
            pushData.secondPass = 0;
            vkCmdPushConstants(frameInfo.cmdBuf, pipeLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushData), &pushData);

#if CASCADE_COUNT_IN_ZED_GROUP_COUNT
            vkCmdDispatch(frameInfo.cmdBuf, 1, OCEAN_WAVE_COUNT, cascade_count);
#else
            vkCmdDispatch(frameInfo.cmdBuf, 1, OCEAN_WAVE_COUNT, 1);
            EWEDevice::GetEWEDevice()->TransferImageStage(frameInfo.cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, fftImage);
            pushData.secondPass = 1;
            vkCmdPushConstants(frameInfo.cmdBuf, pipeLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushData), &pushData);
            vkCmdDispatch(frameInfo.cmdBuf, 1, OCEAN_WAVE_COUNT, 1);
#endif
        }

        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GRAPHICS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        OceanGraphicsGPUData::OceanGraphicsGPUData() {
            CreatePipeLayout();
            CreatePipeline();
            CreateModel();
        }
        OceanGraphicsGPUData::~OceanGraphicsGPUData() {
            renderData[0]->~EWEBuffer();
            ewe_free(renderData[0]);
            renderData[1]->~EWEBuffer();
            ewe_free(renderData[1]);
            delete oceanModel;
        }

        void OceanGraphicsGPUData::CreateDescriptorSet(VkDescriptorImageInfo* outputImage, VkDescriptorImageInfo* skyboxImage) {
            //creating the buffer here as well
            //OceanRenderParameters
            renderData[0] = ConstructSingular<EWEBuffer>(ewe_call_trace, sizeof(OceanRenderParameters), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            renderData[0]->Map();
            renderData[0]->WriteToBuffer(&oceanRenderParameters, sizeof(OceanRenderParameters));
            renderData[0]->Flush();

            renderData[1] = ConstructSingular<EWEBuffer>(ewe_call_trace, sizeof(OceanRenderParameters), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            renderData[1]->Map();
            renderData[1]->WriteToBuffer(&oceanRenderParameters, sizeof(OceanRenderParameters));
            renderData[1]->Flush();

            EWEDescriptorWriter descWriter{ eweDSL, DescriptorPool_Global };
            descWriter.WriteBuffer(0, renderData[0]->DescriptorInfo());
            descWriter.WriteImage(1, outputImage);
            descWriter.WriteImage(2, skyboxImage);
            descriptorSet[0] = descWriter.Build();

            descWriter.WriteBuffer(0, renderData[1]->DescriptorInfo());
            descriptorSet[1] = descWriter.Build();

        }
        void OceanGraphicsGPUData::CreatePipeLayout() {

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;

            std::vector<VkDescriptorSetLayout> tempDSL;
            tempDSL.push_back(DescriptorHandler::getDescSetLayout(LDSL_global));

            EWEDescriptorSetLayout::Builder dslBuilder{};
            dslBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, 1);
            dslBuilder.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS, 1);
            dslBuilder.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS, 1);
            eweDSL = dslBuilder.build();

            tempDSL.push_back(eweDSL->GetDescriptorSetLayout());	
            
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL.size());
            pipelineLayoutInfo.pSetLayouts = tempDSL.data();
            EWE_VK_ASSERT(vkCreatePipelineLayout(EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipeLayout));
        }
        void OceanGraphicsGPUData::CreatePipeline(){
            EWEPipeline::PipelineConfigInfo pipelineConfig{};
            EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);

            pipelineConfig.pipelineLayout = pipeLayout;
            pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<SimpleVertex>();
            pipelineConfig.attributeDescriptions = SimpleVertex::GetAttributeDescriptions();

            std::string vertString = "ocean.vert.spv";
            std::string fragString = "ocean.frag.spv";

            //EWEPipeline* tempPtr = new EWEPipeline(vertString, fragString, pipelineConfig);

            pipe = std::make_unique<EWEPipeline>(vertString, fragString, pipelineConfig);

        }
        void OceanGraphicsGPUData::CreateModel() {
            const uint32_t GRID_SIZE = 1024;
            constexpr uint32_t vertex_width = (GRID_SIZE + 1);

            const int halfGridSize = GRID_SIZE / 2;
            const float gridScale = 1.f;


            glm::vec3 baseTranslation = {
                -float(GRID_SIZE) / 2.f * gridScale,
                0.f,
                -float(GRID_SIZE) / 2.f * gridScale
            };
            glm::vec3 vertexTranslation = baseTranslation;

            std::vector<SimpleVertex> gridVertices{};
            gridVertices.resize(vertex_width * vertex_width);
            std::vector<uint32_t> gridIndices;
            gridIndices.resize(GRID_SIZE * GRID_SIZE * 6);

            uint32_t currentVertexPos = 0;
            for (uint32_t y = 0; y < vertex_width; y++) {
                vertexTranslation[0] = baseTranslation[0];
                for (uint32_t x = 0; x < vertex_width; x++) {
                    gridVertices[currentVertexPos++].position = vertexTranslation;
                    //memcpy(gOceanRenderData.pGridVertices + currentVertexPos++, vertexTranslation, sizeof(float) * 3);

                    vertexTranslation[0] += gridScale;
                }
                vertexTranslation[2] += gridScale;
            }

            uint32_t currentIndexPos = 0;
            // clockwise starting from top left
            for (uint32_t y = 0; y < GRID_SIZE; y++) {
                for (uint32_t x = 0; x < GRID_SIZE; x++) {
                    //triangle 1
                    gridIndices[currentIndexPos++] = x + y * vertex_width;
                    gridIndices[currentIndexPos++] = x + y * vertex_width + 1;
                    gridIndices[currentIndexPos++] = x + (y + 1) * vertex_width;
                    //triangle 2
                    gridIndices[currentIndexPos++] = gridIndices[currentIndexPos - 2];
                    gridIndices[currentIndexPos++] = x + 1 + (y + 1) * vertex_width;
                    gridIndices[currentIndexPos++] = x + (y + 1) * vertex_width;
                }
            }

            oceanModel = EWEModel::CreateMesh(gridVertices.data(), gridVertices.size(), sizeof(gridVertices[0]), gridIndices, Queue::graphics);
        }

        void OceanGraphicsGPUData::Render(FrameInfo const& frameInfo) {
            pipe->bind(frameInfo.cmdBuf);

            vkCmdBindDescriptorSets(frameInfo.cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeLayout,
                0, 1,
                DescriptorHandler::getDescSet(DS_global, frameInfo.index),
                0, nullptr
            );
            vkCmdBindDescriptorSets(frameInfo.cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeLayout,
                1, 1,
                &descriptorSet[frameInfo.index],
                0, nullptr
            );

            oceanModel->BindAndDraw(frameInfo.cmdBuf);
        }

    } //namespace Ocean
} //namespace EWE