#include "EWEngine/Graphics/DescriptorHandler.h"

#include <stdexcept>

namespace EWE {



	std::unordered_map<LDSL_Enum, EWEDescriptorSetLayout*> DescriptorHandler::descriptorSetLayouts;

	std::unordered_map<DescSet_Enum, std::vector<VkDescriptorSet>> DescriptorHandler::descriptorSets;
	//std::unordered_map<PipeDescSetLayouts_Enum, std::vector<VkDescriptorSetLayout>> DescriptorHandler::pipeDescSetLayouts;

    //std::vector<VkDescriptorSetLayout> DescriptorHandler::dynamicMaterialPipeDescSetLayouts[DYNAMIC_PIPE_LAYOUT_COUNT];

    void DescriptorHandler::cleanup() {
        printf("before descriptor handler cleanup \n");
        for (auto& dsl : descriptorSetLayouts) {
            dsl.second->~EWEDescriptorSetLayout();
            ewe_free(dsl.second);
        }
        descriptorSetLayouts.clear();
        printf("after desc set layouts \n");
        for (auto& descriptorSet : descriptorSets) {
            EWEDescriptorPool::freeDescriptors(DescriptorPool_Global, descriptorSet.second);
            //globalPool->freeDescriptors(descriptorSet.second);
        }
        printf("After freeing  descritpors \n");
        descriptorSets.clear();
        printf("after desc sets \n");
        /*
        printf("before cleaning pipeDescSetLayouts, size : %d \n", pipeDescSetLayouts.size());
        for (auto iter = pipeDescSetLayouts.begin(); iter != pipeDescSetLayouts.end(); iter++) {
            printf("\t iterfirst(%d) size : %d \n", iter->first, iter->second.size());
        }
        for (auto iter = pipeDescSetLayouts.begin(); iter != pipeDescSetLayouts.end(); iter++) {
            for (int i = 0; i < iter->second.size(); i++) {
                if (iter->second[i] != VK_NULL_HANDLE) {
                    printf("destroying pipeDescSetLayouts, iter(iter first: i) - (%d:%d) \n", iter->first, i);
                    vkDestroyDescriptorSetLayout(EWEDevice::GetVkDevice(), iter->second[i], nullptr);
                }
                else {
                    printf("why is pipe desc set layout[%d] nullhandle, but exists? \n", i);
                }
            }
        }

        printf("after pipedesc set layouts \n");
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < dynamicMaterialPipeDescSetLayouts[i].size(); j++) {
                if (dynamicMaterialPipeDescSetLayouts[i][j] != VK_NULL_HANDLE) {
                    vkDestroyDescriptorSetLayout(EWEDevice::GetVkDevice(), dynamicMaterialPipeDescSetLayouts[i][j], nullptr);
                }
                else {
                    printf("why is dynamicMaterialPipeDescSetLayouts[%d][%d] nullhandle, but exists? \n", i, j);
                }
            }
        }
        */
        //pipeDescSetLayouts.clear();
        printf("after descriptor handler cleanup \n");
    }

    EWEDescriptorSetLayout* DescriptorHandler::getLDSL(LDSL_Enum whichLDSL) {
        if (whichLDSL == LDSL_pointLight && (!descriptorSetLayouts.contains(LDSL_pointLight))) {
            printf("returning global instead of point LDSL \n");
            return descriptorSetLayouts.at(LDSL_global);
        }
#if _DEBUG
        else if (!descriptorSetLayouts.contains(whichLDSL)) {
            printf("failed to find LDSL : %d \n", whichLDSL);
        }
#endif
        return descriptorSetLayouts.at(whichLDSL);
    }

    VkDescriptorSetLayout DescriptorHandler::getDescSetLayout(LDSL_Enum whichDescSet) {
        {
            auto foundDSL = descriptorSetLayouts.find(whichDescSet);
            if (foundDSL != descriptorSetLayouts.end()) {
                return foundDSL->second->GetDescriptorSetLayout();
            }
        }
        //printf("constructing LDSL : %d \n", whichDescSet);
        EWEDescriptorSetLayout* dsl;
        switch (whichDescSet) {
        case LDSL_global: {
            dsl = EWEDescriptorSetLayout::Builder()
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();
            break;
        }
        case LDSL_boned: {
            //printf("CREATING LDSL_boned \n");
            dsl = EWEDescriptorSetLayout::Builder()
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .build();
            break;
        }
        case LDSL_smallInstance: { //supports bone+instancing
            dsl = EWEDescriptorSetLayout::Builder()
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .build();
            break;
        }
        case LDSL_largeInstance: { //supports bone+instancing
            dsl = EWEDescriptorSetLayout::Builder()
                .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .build();
            break;
        }
        default: {
            printf("TRYING TO CREATE A LAYOUT THAT DOES NOT HAVE SUPPORT : %d \n", whichDescSet);
            //throw std::
        }
        }
        //printf("returning LDSL : %d \n", whichDescSet);
        return descriptorSetLayouts.emplace(whichDescSet, dsl).first->second->GetDescriptorSetLayout();
    }

    void DescriptorHandler::initGlobalDescriptors(std::unordered_map<Buffer_Enum, std::vector<EWEBuffer*>>& bufferMap) {
        printf("init global descriptors \n");
        DescriptorHandler::getDescSetLayout(LDSL_global);
        DescriptorHandler::getDescSetLayout(LDSL_boned);
        descriptorSets.emplace(DS_global, std::vector<VkDescriptorSet>{});
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            //printf("init ars descriptors, loop : %d \n", i);
            descriptorSets.at(DS_global).emplace_back(EWEDescriptorWriter(DescriptorHandler::getLDSL(LDSL_global), DescriptorPool_Global)
                .writeBuffer(0, bufferMap.at(Buff_ubo)[i]->DescriptorInfo())
                .writeBuffer(1, bufferMap.at(Buff_gpu)[i]->DescriptorInfo())
                .build());
        }
#if DEBUG_NAMING
        DebugNaming::SetObjectName(EWEDevice::GetVkDevice(), descriptorSets.at(DS_global)[0], VK_OBJECT_TYPE_DESCRIPTOR_SET, "global DS[0]");
        DebugNaming::SetObjectName(EWEDevice::GetVkDevice(), descriptorSets.at(DS_global)[1], VK_OBJECT_TYPE_DESCRIPTOR_SET, "global DS[1]");
#endif
    }
    void DescriptorHandler::initDescriptors(std::unordered_map<Buffer_Enum, std::vector<EWEBuffer*>>& bufferMap) {

        //printf("initializing VkDescriptorSets \n");
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            //printf("init ars descriptors, loop : %d \n", i);
            //printf("after global set \n");
#if DRAWING_POINTS
            advancedRS.pointLightDescriptorSet.push_back(VkDescriptorSet{});
            if (!
                EWEDescriptorWriter(DescriptorHandler::getLDSL(LDSL_pointLight), *advancedRS.globalPool)
                .writeBuffer(0, &uboBuffers[i]->descriptorInfo())
                .build(advancedRS.pointLightDescriptorSet.back())
                ) {
                std::cout << "PointLight SET FAILURE" << std::endl;
            }
            printf("after pointlight set \n");
#endif
            //printf("after bone weapon set 2 \n");
            /*
            advancedRS.spotlightDescriptorSet.push_back(VkDescriptorSet{});
            if (!
                EWEDescriptorWriter(*advancedRS.globalSetLayout, *advancedRS.globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeBuffer(1, &sboBufferInfo)
                .build(advancedRS.spotlightDescriptorSet.back())
                ) {
                std::cout << "spotLIGHT SET FAILURE" << std::endl;
            }
            */
        }
        //printf("returning from init VkDescriptorSets \n");
    }
    VkDescriptorSet* DescriptorHandler::getDescSet(DescSet_Enum whichDescSet, int8_t whichFrameIndex) {
#if _DEBUG
        if (!descriptorSets.contains(whichDescSet)) {
            printf("failed to find DescSet in getDescSet : %d \n", whichDescSet);
        }
#endif
        return &descriptorSets[whichDescSet][whichFrameIndex];
    }
}