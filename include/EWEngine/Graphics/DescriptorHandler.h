#pragma once
#define DRAWING_POINTS false

#include "EWEngine/Graphics/Descriptors.h"
#include "EWEngine/Graphics/Device_Buffer.h"

#include <unordered_map>


namespace EWE {
    enum DescSet_Enum : uint16_t {
        DS_global,
        DS_pointLight,

        DS_MAX_COUNT,
    };

    /*
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
    */
	enum LDSL_Enum : uint16_t {
		LDSL_global,
		LDSL_pointLight,
		LDSL_boned,
        LDSL_smallInstance,
        LDSL_largeInstance,
	};

	enum Buffer_Enum : uint16_t {
		Buff_ubo,
		Buff_gpu,
	};

    class DescriptorHandler {
    private:
        DescriptorHandler() {}

        static std::unordered_map<LDSL_Enum, EWEDescriptorSetLayout*> descriptorSetLayouts;
        static std::unordered_map<DescSet_Enum, std::vector<VkDescriptorSet>> descriptorSets;
        //static std::unordered_map<PipeDescSetLayouts_Enum, std::vector<VkDescriptorSetLayout>> pipeDescSetLayouts;

    public:
        static void cleanup();
        static EWEDescriptorSetLayout* getLDSL(LDSL_Enum whichLDSL);
        static void initGlobalDescriptors(std::unordered_map<Buffer_Enum, std::vector<EWEBuffer*>>& bufferMap);
        
        static void initDescriptors(std::unordered_map<Buffer_Enum, std::vector<EWEBuffer*>>& bufferMap);
        static VkDescriptorSetLayout getDescSetLayout(LDSL_Enum whichDescSet);
        static VkDescriptorSet* getDescSet(DescSet_Enum whichDescSet, int8_t whichFrameIndex);
    };
}