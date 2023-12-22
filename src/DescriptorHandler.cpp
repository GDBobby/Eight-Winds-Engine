#include "EWEngine/graphics/DescriptorHandler.h"


namespace EWE {



	std::unordered_map<LDSL_Enum, std::unique_ptr<EWEDescriptorSetLayout>> DescriptorHandler::descriptorSetLayouts;

	std::unordered_map<DescSet_Enum, std::vector<VkDescriptorSet>> DescriptorHandler::descriptorSets;
	std::unordered_map<PipeDescSetLayouts_Enum, std::vector<VkDescriptorSetLayout>> DescriptorHandler::pipeDescSetLayouts;

    std::vector<VkDescriptorSetLayout> DescriptorHandler::dynamicMaterialPipeDescSetLayouts[DYNAMIC_PIPE_LAYOUT_COUNT];

    VkDescriptorSetLayout DescriptorHandler::getDescSetLayout(LDSL_Enum whichDescSet, EWEDevice& device) {
        if (descriptorSetLayouts.find(whichDescSet) != descriptorSetLayouts.end()) {
            return descriptorSetLayouts[whichDescSet]->getDescriptorSetLayout();
        }
        //printf("constructing LDSL : %d \n", whichDescSet);
        switch (whichDescSet) {
        case LDSL_global: {
            descriptorSetLayouts[LDSL_global] = EWEDescriptorSetLayout::Builder(device)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build();
            break;
        }
        case LDSL_boned: {
            //printf("CREATING LDSL_boned \n");
            descriptorSetLayouts[LDSL_boned] = EWEDescriptorSetLayout::Builder(device)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .build();
            break;
        }
        case LDSL_smallInstance: { //supports bone+instancing
            descriptorSetLayouts[whichDescSet] = EWEDescriptorSetLayout::Builder(device)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .build();
            break;
        }
        case LDSL_largeInstance: { //supports bone+instancing
            descriptorSetLayouts[whichDescSet] = EWEDescriptorSetLayout::Builder(device)
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
        return descriptorSetLayouts[whichDescSet]->getDescriptorSetLayout();
    }

    std::vector<VkDescriptorSetLayout>* DescriptorHandler::getPipeDescSetLayout(PipeDescSetLayouts_Enum PDLe, EWEDevice& device) {
        if (pipeDescSetLayouts.find(PDLe) != pipeDescSetLayouts.end()) {
            return &pipeDescSetLayouts[PDLe];
        }
        //printf("PDSL construction : %d \n", PDLe);
        switch (PDLe) {
#if DRAWING_POINTS
        case PDSL_pointLight: {
            pipeDescSetLayouts[PDSL_pointLight].push_back(getDescSetLayout(DSL_pointLight, device));
            break;
        }
#endif
        case PDSL_global: {
            pipeDescSetLayouts[PDSL_global].push_back(getDescSetLayout(LDSL_global, device));
            break;
        }
        case PDSL_textured: {
            pipeDescSetLayouts[PDSL_textured].push_back(getDescSetLayout(LDSL_global, device));
            pipeDescSetLayouts[PDSL_textured].push_back(EWETexture::getSimpleDescriptorSetLayout());
            break;
        }
        
        case PDSL_orbOverlay: {
            //pipeDescSetLayouts[PDSL_orbOverlay].push_back(EWETexture::getOrbDescriptorSetLayout());
            pipeDescSetLayouts[PDSL_orbOverlay].push_back(EWETexture::getSimpleDescriptorSetLayout());
            break;
        }
        case PDSL_boned: {
            pipeDescSetLayouts[PDSL_boned].push_back(getDescSetLayout(LDSL_global, device));
            pipeDescSetLayouts[PDSL_boned].push_back(getDescSetLayout(LDSL_boned, device));
            pipeDescSetLayouts[PDSL_boned].push_back(EWETexture::getSimpleDescriptorSetLayout());
            break;
        }
        case PDSL_2d: {
            pipeDescSetLayouts[PDSL_2d].push_back(EWETexture::getSimpleDescriptorSetLayout());
            break;
        }
        /*
        case PDSL_boneWeapon: {
            pipeDescSetLayouts[PDSL_boneWeapon].push_back(getDescSetLayout(LDSL_global, device));
            pipeDescSetLayouts[PDSL_boneWeapon].push_back(EWETexture::getMaterialDescriptorSetLayout());
            pipeDescSetLayouts[PDSL_boneWeapon].push_back(getDescSetLayout(LDSL_boneWeapon, device));
            break;
        }
        */
        case PDSL_visualEffect: {
            pipeDescSetLayouts[PDSL_visualEffect].push_back(getDescSetLayout(LDSL_global, device));
            pipeDescSetLayouts[PDSL_visualEffect].push_back(EWETexture::getSimpleDescriptorSetLayout());
            break;
        }
        case PDSL_grass: {
            pipeDescSetLayouts[PDSL_grass].push_back(getDescSetLayout(LDSL_global, device));
            pipeDescSetLayouts[PDSL_grass].push_back(EWETexture::getSimpleVertDescriptorSetLayout());
            break;
        }
        case PDSL_loading: {
            //printf("PDSL loading???? \n");
            pipeDescSetLayouts[PDSL_loading].push_back(getDescSetLayout(LDSL_global, device));
            pipeDescSetLayouts[PDSL_loading].push_back(EWETexture::getSimpleDescriptorSetLayout());
            pipeDescSetLayouts[PDSL_loading].push_back(getDescSetLayout(LDSL_boned, device));
            break;
        }
        default: {
            printf("TRYING TO CREATE A PDSL THAT DOESNT EXIST : %d \n", PDLe);
            break;
        }
        }
        //printf("end of PDSL construction, returning : %d \n", PDLe);
        return &pipeDescSetLayouts[PDLe];
    }

    std::vector<VkDescriptorSetLayout>* DescriptorHandler::getDynamicPipeDescSetLayout(uint8_t textureCount, bool hasBones, bool instanced, EWEDevice& device) {
        //printf("get dynamic pipe desc set layout : %d \n", textureCount + (hasBones * MAX_SMART_TEXTURE_COUNT) + (instanced * (MAX_SMART_TEXTURE_COUNT * 2)));
        if (dynamicMaterialPipeDescSetLayouts[textureCount + (hasBones * MAX_SMART_TEXTURE_COUNT) + (instanced * (MAX_SMART_TEXTURE_COUNT * 2))].size() == 0) {
            dynamicMaterialPipeDescSetLayouts[textureCount + (hasBones * MAX_SMART_TEXTURE_COUNT) + (instanced * (MAX_SMART_TEXTURE_COUNT * 2))].push_back(getDescSetLayout(LDSL_global, device));
#ifdef _DEBUG
            printf("getting dynamic PDSL - %d:%d:%d \n", textureCount, hasBones, instanced);
#endif
            //testing if this is faster, if not return if(hasbones)
            if (hasBones && instanced) {
                //printf("Pushing back LDSL instanced monster \n");
                dynamicMaterialPipeDescSetLayouts[textureCount + (hasBones * MAX_SMART_TEXTURE_COUNT) + (instanced * (MAX_SMART_TEXTURE_COUNT * 2))].push_back(getDescSetLayout(LDSL_largeInstance, device));

            }
            else if (hasBones) {
                dynamicMaterialPipeDescSetLayouts[textureCount + (hasBones * MAX_SMART_TEXTURE_COUNT) + (instanced * (MAX_SMART_TEXTURE_COUNT * 2))].push_back(getDescSetLayout(LDSL_boned, device));
            }
            else if (instanced) {
                printf("currrently not supporting instancing without bones, THROWING ERROR \n");
                throw std::exception("instanced but doesn't have bones? \n");
            }
            dynamicMaterialPipeDescSetLayouts[textureCount + (hasBones * MAX_SMART_TEXTURE_COUNT) + (instanced * (MAX_SMART_TEXTURE_COUNT * 2))].push_back(EWETexture::getDynamicDescriptorSetLayout(textureCount));
        }
        //if (instanced) {
            //printf("returning instanced PDSL size : %d \n", dynamicMaterialPipeDescSetLayouts[textureCount + (hasBones * MAX_SMART_TEXTURE_COUNT) + (instanced * (MAX_SMART_TEXTURE_COUNT * 2))].size());
        //}
        return &dynamicMaterialPipeDescSetLayouts[textureCount + (hasBones * MAX_SMART_TEXTURE_COUNT) + (instanced * (MAX_SMART_TEXTURE_COUNT * 2))];
    }


    void DescriptorHandler::initGlobalDescriptors(std::shared_ptr<EWEDescriptorPool> globalPool, std::map<Buffer_Enum, std::vector<std::unique_ptr<EWEBuffer>>>& bufferMap, EWEDevice& device) {
        printf("init global descriptors \n");
        DescriptorHandler::getDescSetLayout(LDSL_global, device);
        DescriptorHandler::getDescSetLayout(LDSL_boned, device);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            //printf("init ars descriptors, loop : %d \n", i);
            descriptorSets[DS_global].push_back(VkDescriptorSet{});
            if (!
                EWEDescriptorWriter(DescriptorHandler::getLDSL(LDSL_global), *globalPool)
                .writeBuffer(0, bufferMap[Buff_ubo][i]->descriptorInfo())
                .writeBuffer(1, bufferMap[Buff_gpu][i]->descriptorInfo())
                .build(descriptorSets[DS_global].back())
                ) {
                printf("global desc failure \n");
            }

            descriptorSets[DS_loading].push_back(VkDescriptorSet{});
            if (!
                EWEDescriptorWriter(DescriptorHandler::getLDSL(LDSL_boned), *globalPool)
                .writeBuffer(0, bufferMap[Buff_loading][i]->descriptorInfo())
                .build(descriptorSets[DS_loading].back())
                ) {
                printf("loading desc set failure \n");
            }
        }
    }
    void DescriptorHandler::initDescriptors(std::shared_ptr<EWEDescriptorPool> globalPool, std::map<Buffer_Enum, std::vector<std::unique_ptr<EWEBuffer>>>& bufferMap) {

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
}