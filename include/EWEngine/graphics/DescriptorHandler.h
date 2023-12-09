#pragma once
#include "EWE_texture.h"
//#include "EWE_descriptors.h" //included in EWEtexture
#define DRAWING_POINTS false

#define DYNAMIC_PIPE_LAYOUT_COUNT 24

namespace EWE {
    enum DescSet_Enum {
        DS_global,
        DS_pointLight,
        //DS_spotLight,
        //DS_boned,
        //DS_bonePIndex,
        //DS_skybox,
        //DS_boneWeapon,
        DS_loading,

        DS_MAX_COUNT,
    };
    enum PipeDescSetLayouts_Enum {
#if DRAWING_POINTS
        PDSL_pointLight,
#endif
        PDSL_global,
        PDSL_textured,
        PDSL_boned,
        PDSL_2d,
        //PDSL_skybox,
        //PDSL_sprite,
        //PDSL_boneWeapon,
        //PDSL_bonedPlayerIndex,
        PDSL_visualEffect,
        PDSL_grass,
        PDSL_loading,
        PDSL_orbOverlay,

        PSL_MAX_COUNT,
    };
	enum LDSL_Enum {
		LDSL_global,
		LDSL_pointLight,
		LDSL_boned,
        LDSL_smallInstance,
        LDSL_largeInstance,
		//LDSL_boneWeapon,
		//LDSL_bonePIndex,
	};
	enum Buffer_Enum {
		Buff_ubo,
		Buff_gpu,
        Buff_loading,
	};

    class DescriptorHandler {
    private:
        DescriptorHandler() {}

        static std::map<LDSL_Enum, std::unique_ptr<EWEDescriptorSetLayout>> descriptorSetLayouts;
        static std::map<DescSet_Enum, std::vector<VkDescriptorSet>> descriptorSets;
        static std::map<PipeDescSetLayouts_Enum, std::vector<VkDescriptorSetLayout>> pipeDescSetLayouts;
        static std::vector<VkDescriptorSetLayout> dynamicMaterialPipeDescSetLayouts[DYNAMIC_PIPE_LAYOUT_COUNT];
    public:
        static void cleanup(EWEDevice& device, std::shared_ptr<EWEDescriptorPool> globalPool) {
            printf("before descriptor handler cleanup \n");
            descriptorSetLayouts.clear();
            printf("after desc set layouts \n");
            for (auto& descriptorSet : descriptorSets) {
                globalPool->freeDescriptors(descriptorSet.second);
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
                        vkDestroyDescriptorSetLayout(device.device(), iter->second[i], nullptr);
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
                        vkDestroyDescriptorSetLayout(device.device(), dynamicMaterialPipeDescSetLayouts[i][j], nullptr);
                    }
                    else {
                        printf("why is dynamicMaterialPipeDescSetLayouts[%d][%d] nullhandle, but exists? \n", i, j);
                    }
                }
            }
            */
            pipeDescSetLayouts.clear();
            for (int i = 0; i < 10; i++) {
                dynamicMaterialPipeDescSetLayouts[i].clear();
            }

            printf("after descriptor handler cleanup \n");
        }
        static EWEDescriptorSetLayout& getLDSL(LDSL_Enum whichLDSL) {
            if (whichLDSL == LDSL_pointLight && descriptorSetLayouts.find(LDSL_pointLight) == descriptorSetLayouts.end()) {
                printf("returning global instead of point LDSL \n");
                return *(descriptorSetLayouts[LDSL_global]);
            }
#if _DEBUG
            else if (descriptorSetLayouts.find(whichLDSL) == descriptorSetLayouts.end()) {
                printf("failed to find LDSL : %d \n", whichLDSL);
            }
#endif
            return *(descriptorSetLayouts[whichLDSL]);
        }
        static void initGlobalDescriptors(std::shared_ptr<EWEDescriptorPool> globalPool, std::map<Buffer_Enum, std::vector<std::unique_ptr<EWEBuffer>>>& bufferMap, EWEDevice& device);
        
        static void initDescriptors(std::shared_ptr<EWEDescriptorPool> globalPool, std::map<Buffer_Enum, std::vector<std::unique_ptr<EWEBuffer>>>& bufferMap);
        static VkDescriptorSetLayout getDescSetLayout(LDSL_Enum whichDescSet, EWEDevice& device);
        static std::vector<VkDescriptorSetLayout>* getPipeDescSetLayout(PipeDescSetLayouts_Enum PDLe, EWEDevice& device);
        static std::vector<VkDescriptorSetLayout>* getDynamicPipeDescSetLayout(uint8_t textureCount, bool hasBones, bool instanced, EWEDevice& device);
        static VkDescriptorSet* getDescSet(DescSet_Enum whichDescSet, int8_t whichFrameIndex) {
#if _DEBUG
            if (descriptorSets.find(whichDescSet) == descriptorSets.end()) {
                printf("failed to find DescSet in getDescSet : %d \n", whichDescSet);
            }
#endif
            return &descriptorSets[whichDescSet][whichFrameIndex];
        }
        //std::vector<std::unique_ptr<EWEBuffer>>* bufferVector, int maxFIF, std::shared_ptr<EWEDescriptorPool> globalPool
        /*
        static void setActorBoneDescriptor(int maxFIF, std::shared_ptr<EWEDescriptorPool> globalPool, std::vector<std::unique_ptr<EWEBuffer>>* bufferVector, EWEDevice& device) {
            printf("before actor bone descriptor \n");
            //bufferVector->clear();
            descriptorSets[DS_ActorBone].clear();
            vkDeviceWaitIdle(device.device());

            
            for (int i = 0; i < maxFIF; i++) {
                descriptorSets[DS_ActorBone].push_back(VkDescriptorSet{});
                if (!
                    EWEDescriptorWriter(DescriptorHandler::getLDSL(LDSL_boned), *globalPool)
                    .writeBuffer(0, &bufferVector->at(i)->descriptorInfo())
                    .build(descriptorSets[DS_ActorBone].back())
                    ) {
                    printf("bone set failure \n");
                }
            }
            printf("after actor bone descriptor \n");
        }
        */



        //std::vector<std::unique_ptr<EWEPipeline>> dynamicMaterialPipeline;
    };
}