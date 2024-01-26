#include "EWEngine/SkeletonHandlerBase.h"
#include "EWEngine/Systems/Rendering/Skin/SkinRS.h"
#include "EWEngine/Graphics/Textures/Texture_Manager.h"
#include "EWEngine/Graphics/Textures/Material_Textures.h"

#include <thread>


#define TEST_NO_MESH false

#define DEBUGGING_ANIMS false


//#define THREADED_LOAD false


namespace EWE {

    void SkeletonBase::readMeshData(std::string filePath, EWEDevice& device, ImportData::meshEData& importMesh) {
        std::ifstream inFile(filePath, std::ifstream::binary);
        //inFile.open();
        if (!inFile.is_open()) {
            printf("failed to open : %s \n", filePath.c_str());
            //std throw
        }
        //printf("before formatingg input file in mesh \n");
        boost::archive::binary_iarchive binary_input_archive(inFile, boost::archive::no_header);
        //printf("before synchronizing \n");
        //binary_input_archive& fileData;
        binary_input_archive& importMesh;
        inFile.close();
        //printf("file read successfully \n");
    }
    void SkeletonBase::readMeshNTData(std::string filePath, EWEDevice& device, ImportData::meshNTEData& importMesh) {
        std::ifstream inFile(filePath, std::ifstream::binary);
        //inFile.open();
        if (!inFile.is_open()) {
            printf("failed to open : %s \n", filePath.c_str());
            //std throw
        }
        // printf("before formatingg input file in mesh \n");
        boost::archive::binary_iarchive binary_input_archive(inFile, boost::archive::no_header);
        //printf("before synchronizing \n");
        //binary_input_archive& fileData;
        ;
        binary_input_archive& importMesh;
        inFile.close();
        //printf("file read successfully \n");
    }

    void SkeletonBase::readAnimData(std::string filePath, EWEDevice& device, bool partial) {
        std::ifstream inFile(filePath, std::ifstream::binary);
        if (!inFile.is_open()) {
            printf("failed to open : %s \n", filePath.c_str());
            //std throw
        }
        //printf("before opening anim archive \n");
        if (partial) {
            boost::archive::binary_iarchive bia_anim(inFile, boost::archive::no_header);
            ImportData::AnimData importData;
            bia_anim& importData;
            inFile.close();
            //printf("after loading anim archive \n");
            if (importData.versionTracker != "2.0.0") {
                printf("FAILED TO MATCH VERSION, DISCARD \n");
                return;
                //std throw maybe?
            }

            partialAnimationData.resize(importData.animations.size());
            //printf("animationData size? : %d \n", animationData.size());
            uint32_t biggestBoneCount = 0;
            for (int i = 0; i < importData.animations.size(); i++) { //animation count
                uint32_t currentBoneCount = static_cast<uint32_t>(importData.animations[i][0].size());
                if (currentBoneCount > biggestBoneCount) { biggestBoneCount = currentBoneCount; }

                partialAnimationData[i].resize(importData.animations[i].size());
                for (int j = 0; j < importData.animations[i].size(); j++) { //animation duration
                    for (int k = 0; k < importData.animations[i][j].size(); k++) { //bone id, bone transform ~ for that animation frame
                        partialAnimationData[i][j][importData.animations[i][j][k].boneID] = glm::make_mat4(importData.animations[i][j][k].boneTransform.bmat4);
                    }
                }
            }

            defaultMatrix.resize(importData.defaultBoneValues.size());
            for (int i = 0; i < importData.defaultBoneValues.size(); i++) {
                defaultMatrix[i] = glm::make_mat4(importData.defaultBoneValues[i].bmat4);
            }
            if (defaultMatrix.size() != biggestBoneCount) {
                //printf("default matrix bigger than biggest animation, difference : %d \n", defaultMatrix.size() - biggestBoneCount);
            }
            if (defaultMatrix.size() < biggestBoneCount) {
                //printf("~~~~~~~~~~~~~~~~ resizing default matrix  ~~~~~~~~~~~~~~\n");
                defaultMatrix.resize(biggestBoneCount, glm::mat4{ 1.f });
            }
            boneCount = static_cast<uint16_t>(defaultMatrix.size());
            handBone = importData.handBone;
        }
        else {
            boost::archive::binary_iarchive bia_anim(inFile, boost::archive::no_header);
            ImportData::FullAnimData importData;
            bia_anim& importData;
            inFile.close();
            //printf("after loading anim archive \n");
            if (importData.versionTracker != "2.0.0") {
                printf("FAILED TO MATCH VERSION, DISCARD \n");
                return;
                //std throw maybe?
            }

            fullAnimationData.resize(importData.animations.size());
            //printf("animationData size? : %d \n", animationData.size());
            //uint32_t biggestBoneCount = 0;
            for (int i = 0; i < importData.animations.size(); i++) { //animation count
                //uint32_t currentBoneCount = importData.animations[i][0].size();
                //if (currentBoneCount > biggestBoneCount) { biggestBoneCount = currentBoneCount; }

                fullAnimationData[i].resize(importData.animations[i].size());
                for (int j = 0; j < importData.animations[i].size(); j++) { //animation duration
                    for (int k = 0; k < importData.animations[i][j].size(); k++) { //bone id, bone transform ~ for that animation frame
                        fullAnimationData[i][j].emplace_back(glm::make_mat4(importData.animations[i][j][k].bmat4));
                    }
                }
            }
            boneCount = static_cast<uint16_t>(fullAnimationData[0][0].size());
        }

    }

    void SkeletonBase::loadTextures(EWEDevice& device, std::string filePath, std::pair<std::vector<MaterialTextureInfo>, std::vector<MaterialTextureInfo>>& textureTracker, std::string texturePath) {
        //printf("failed to open : %s \n", filePath.c_str());
        std::ifstream inFile(filePath, std::ifstream::binary);
        if (!inFile.is_open()) {
            printf("failed to open : %s \n", filePath.c_str());
            //std throw
        }
        //printf("before opening name archive \n");
        boost::archive::binary_iarchive bia_anim(inFile, boost::archive::no_header);
        ImportData::NameExportData importData;
        bia_anim& importData;
        inFile.close();
        //TEXTURES
        //this should be put in a separate function but im too lazy rn
        //printf("before textures \n");

        for (int i = 0; i < importData.meshNames.size(); i++) {
            importData.meshNames[i] = importData.meshNames[i].substr(0, importData.meshNames[i].find_first_of("."));
            std::string finalDir = texturePath;
            finalDir += importData.meshNames[i];
            
            MaterialTextureInfo materialInfo{ Material_Texture::createMaterialTexture(device, finalDir, true) };
            textureTracker.first.push_back(materialInfo);
            
        }
        //printf("after mesh texutres \n");
        for (int i = 0; i < importData.meshNTNames.size(); i++) {
            importData.meshNTNames[i] = importData.meshNTNames[i].substr(0, importData.meshNTNames[i].find_first_of("."));
            std::string finalDir = texturePath;
            finalDir += importData.meshNTNames[i];

            MaterialTextureInfo materialInfo = Material_Texture::createMaterialTexture(device, finalDir, true);
            textureTracker.second.push_back(materialInfo);
        }
        //printf("after mesh nt texutres \n");
    }

    //                            (const std::string& importPath, EWEDevice& device)
    SkeletonBase::SkeletonBase(std::string importPath, EWEDevice& device, std::string texturePath, bool instanced) {
        printf("skeleton : improting data : %s \n", importPath.c_str());

        mySkeletonID = SkinRenderSystem::getSkinID();

        std::string meshPath = importPath;
        meshPath += "_mesh.ewe";
        std::thread meshThread1;
        bool meshThread1Exist = std::filesystem::exists(meshPath);
        ImportData::meshEData importMesh;
#if !TEST_NO_MESH
        if (meshThread1Exist) {
            //printf("starting up mesh thread 1 \n");
            meshThread1 = std::thread{ &SkeletonBase::readMeshData, this, meshPath, std::ref(device), std::ref(importMesh) };
        }
        else {
            printf("skeleton mesh path doesn't exist : %s \n", meshPath.c_str());
        }

        meshPath = importPath + "_meshNT.ewe";
        std::thread meshThread2;
        bool meshThread2Exist = std::filesystem::exists(meshPath);
        ImportData::meshNTEData importMeshNT;
        if (meshThread2Exist) {
            //printf("starting up mesh thread 2 \n");
            meshThread2 = std::thread{ &SkeletonBase::readMeshNTData, this, meshPath, std::ref(device), std::ref(importMeshNT) };
        }
        else {
            printf("skeleton mesh NT path doesn't exist : %s \n", meshPath.c_str());
        }
#endif
        bool partial = true;
        meshPath = importPath + "_anim.ewe";
        if (!std::filesystem::exists(meshPath)) {
            printf("skeleton partial anim path doesn't exist : %s \n", meshPath.c_str());
            //printf("finna get an error \n");
            partial = false;
            meshPath = importPath + "_fullAnim.ewe";
            if (!std::filesystem::exists(meshPath)) {
                printf("skeleton full anim path doesn't exist : %s \n", meshPath.c_str());
                throw std::runtime_error("couldn't find either anim path for skeleton");
            }
        }

        std::pair<std::vector<MaterialTextureInfo>, std::vector<MaterialTextureInfo>> textureMappingTracker;
        readAnimData(meshPath, device, partial);

        loadTextures(device, importPath + "_Names.ewe", textureMappingTracker, texturePath);

        //printf("textures created \n");

        //printf("handBone value : %d \n", handBone);
        //printf("thread existance? : %d:%d \n", meshThread1Exist, meshThread2Exist);
#if !TEST_NO_MESH
        if (meshThread1Exist) {
            //printf("waiting on mesh thread 1 \n");
            meshThread1.join();
            //printf("mesh thread 1 finished \n");
            for (int i = 0; i < importMesh.meshes.size(); i++) {
                meshes.push_back(EWEModel::createMesh(device, importMesh.meshes[i].first, importMesh.meshes[i].second));
            }
        }
        if (meshThread2Exist) {
            //printf("waiting on mesh thread 2 \n");
            meshThread2.join();

            for (int i = 0; i < importMeshNT.meshesNT.size(); i++) {
                meshesNT.push_back(EWEModel::createMesh(device, importMeshNT.meshesNT[i].first, importMeshNT.meshesNT[i].second));
            }
            //printf("mesh thread 2 finished \n");
        }
#endif

        if (textureMappingTracker.first.size() != meshes.size()) {
            std::cout << "mesh to name mismatch - " << textureMappingTracker.first.size() << ":" << meshes.size() << std::endl;
            //std throw
            throw std::runtime_error("failed to match mesh to name");
        }
        if (textureMappingTracker.second.size() != meshesNT.size()) {
            std::cout << "meshNT to name mismatch - " << textureMappingTracker.second.size() << ":" << meshesNT.size() << std::endl;
            //std throw
            throw std::runtime_error("failed to match meshNT to nameNT");
        }
        for (int i = 0; i < meshes.size(); i++) {
            SkinRenderSystem::addSkeleton(textureMappingTracker.first[i], boneCount, meshes[i].get(), mySkeletonID, instanced);
        }
        for (int i = 0; i < meshesNT.size(); i++) {
            SkinRenderSystem::addSkeleton(textureMappingTracker.second[i], boneCount, meshesNT[i].get(), mySkeletonID, instanced);
        }
        return;
        //returning here ~~~~~~~~~~~~~~~~~~~~~~~~


        /*
        if (instanced) {
            
            for (int i = 0; i < textureMappingTracker.first.size(); i++) {//texture counts
                bool foundAPipeMatch = false;
                for (int j = 0; j < pipePairs.size(); j++) {
                    if (pipePairs[j].first == textureMappingTracker.first[i].first) {
                        foundAPipeMatch = true;
                        bool foundATextureMatch = false;
                        for (int k = 0; k < pipePairs[j].second.size(); k++) {
                            if (textureMappingTracker.first[i].second == pipePairs[j].second[k].first) {
                                foundATextureMatch = true;
                                pipePairs[j].second[k].second.push_back(meshes[i].get());

                                break;
                            }
                        }
                        if (!foundATextureMatch) {
                            //probably comment this out first if theres an error
                            pipePairs[j].second.emplace_back(textureMappingTracker.first[i].second, std::vector<EWEModel*>{meshes[i].get()});
                        }

                        break;
                    }
                }
                if (!foundAPipeMatch) {
                    *
                        std::vector<std::pair<uint8_t, //pipeline flags
                        std::vector<std::pair<int32_t, //textureID
                        std::vector<uint8_t //meshid
                        >>>>> pipePairs;
                    *

                    //wtf im confusing myself
                    pipePairs.emplace_back(textureMappingTracker.first[i].first,
                        std::vector<std::pair<int32_t,
                        std::vector<EWEModel*>>>{});
                    pipePairs.back().second.emplace_back(textureMappingTracker.first[i].second, std::vector<EWEModel*>{meshes[i].get()});

                }
                //pipelinePairs[textureMappingTracker.first[i].first].emplace_back(i, textureMappingTracker.first[i].second);
                //NT is no normal map?

            }
            


        }
        else {



            std::shared_ptr<RigidRenderingSystem> materialInstance = RigidRenderingSystem::getRigidRSInstance();
            //printf("filling data into material map in materialhandler \n");
            for (int i = 0; i < meshes.size(); i++) {
                //printf("pushing mesh into materialhandler, flags,&8 : %d :%d \n", textureMappingTracker.first[i].first, textureMappingTracker.first[i].first & 8);
                materialInstance->addMaterialMonster(textureMappingTracker.first[i].first + 128, actorType, meshes[i].get(), textureMappingTracker.first[i].second);
            }
            for (int i = 0; i < meshesNT.size(); i++) {
                //printf("pushing mesh into materialhandler, flags,&8 : %d :%d \n", textureMappingTracker.second[i].first, textureMappingTracker.second[i].first & 8);
                materialInstance->addMaterialMonster(textureMappingTracker.second[i].first + 128, actorType, meshesNT[i].get(), textureMappingTracker.second[i].second);
            }
        }
        */

        // printf("mesh sizes - %d:%d \n", meshes.size(), meshesNT.size());

         //printf("found anim size, expected ~ %d:%d \n", animationData.size(), anim_undefined);
        /* anim testing
        if (partial) {
            printf("actor, bone count - %d:%d \n", actorType, defaultMatrix.size());
            //if (actorType == Monster_Skeleton) {
            printf("bone count of each animation - actorType:%d \n ~~~~~~~~~~~~ \n", actorType);
            for (int i = 0; i < partialAnimationData.size(); i++) {
                printf("frame coutn of animations : //%d \n", partialAnimationData[i].size());
                printf("\t animDuration[%d] : %d \n", i, partialAnimationData[i][0].size());
                int position = 0;
                for (auto iter = partialAnimationData[i][0].begin(); iter != partialAnimationData[i][0].end(); iter++) {
                    printf("bone value, map position : %d:%d \n", iter->first, position++);
                }
            }
        }
        else {
            printf("full animation, actorType:boneCount - %d:%d ~~~~~~~~~~~~~ \n", actorType, fullAnimationData[0].size());
            for (int i = 0; i < fullAnimationData.size(); i++) {
                printf("\t animDuration[%d] : %d \n", i, fullAnimationData[i].size());
                printf("\t anim bone count :%d \n", fullAnimationData[i][0].size());
            }
        }
        */

    }

}