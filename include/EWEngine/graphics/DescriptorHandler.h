#pragma once
#include "EWEngine/Graphics/Texture.h"
//#include "EWE_descriptors.h" //included in EWEtexture
#define DRAWING_POINTS false

#define DYNAMIC_PIPE_LAYOUT_COUNT 24

namespace EWE {
    enum DescSet_Enum {
        DS_global,
        DS_pointLight,
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
	};
	enum Buffer_Enum {
		Buff_ubo,
		Buff_gpu,
        Buff_loading,
	};

    class DescriptorHandler {
    private:
        DescriptorHandler() {}

        static std::unordered_map<LDSL_Enum, std::unique_ptr<EWEDescriptorSetLayout>> descriptorSetLayouts;
        static std::unordered_map<DescSet_Enum, std::vector<VkDescriptorSet>> descriptorSets;
        static std::unordered_map<PipeDescSetLayouts_Enum, std::vector<VkDescriptorSetLayout>> pipeDescSetLayouts;
        static std::vector<VkDescriptorSetLayout> dynamicMaterialPipeDescSetLayouts[DYNAMIC_PIPE_LAYOUT_COUNT];

    public:
        static void cleanup(EWEDevice& device);
        static EWEDescriptorSetLayout& getLDSL(LDSL_Enum whichLDSL);
        static void initGlobalDescriptors(std::map<Buffer_Enum, std::vector<std::unique_ptr<EWEBuffer>>>& bufferMap, EWEDevice& device);
        
        static void initDescriptors(std::map<Buffer_Enum, std::vector<std::unique_ptr<EWEBuffer>>>& bufferMap);
        static VkDescriptorSetLayout getDescSetLayout(LDSL_Enum whichDescSet, EWEDevice& device);
        static std::vector<VkDescriptorSetLayout>* getPipeDescSetLayout(PipeDescSetLayouts_Enum PDLe, EWEDevice& device);
        static std::vector<VkDescriptorSetLayout>* getDynamicPipeDescSetLayout(uint8_t textureCount, bool hasBones, bool instanced, EWEDevice& device);
        static VkDescriptorSet* getDescSet(DescSet_Enum whichDescSet, int8_t whichFrameIndex);
        //std::vector<std::unique_ptr<EWEBuffer>>* bufferVector, int maxFIF
        /*
        static void setActorBoneDescriptor(int maxFIF, std::vector<std::unique_ptr<EWEBuffer>>* bufferVector, EWEDevice& device) {
            printf("before actor bone descriptor \n");
            //bufferVector->clear();
            descriptorSets[DS_ActorBone].clear();
            vkDeviceWaitIdle(device.device());

            
            for (int i = 0; i < maxFIF; i++) {
                descriptorSets[DS_ActorBone].push_back(VkDescriptorSet{});
                if (!
                    EWEDescriptorWriter(DescriptorHandler::getLDSL(LDSL_boned), DescriptorPool_Global)
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