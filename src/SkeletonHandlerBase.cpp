#include "EWEngine/SkeletonHandlerBase.h"
#include "EWEngine/Systems/Rendering/Skin/SkinRS.h"
#include "EWEngine/Graphics/Texture/Image_Manager.h"
#include "EWEngine/Graphics/Texture/Material_Textures.h"

#include <thread>


#define TEST_NO_MESH false

#define DEBUGGING_ANIMS false


//#define THREADED_LOAD false


namespace EWE {

    void SkeletonBase::ReadAnimData(std::string filePath, bool partial, bool endian) {
        std::ifstream inFile(filePath, std::ifstream::binary);
        assert(inFile.is_open() && "failed to open anim file");
        //printf("before opening anim archive \n");
        if (partial) {
            ImportData::AnimData importData;
            if (endian) {
                importData.ReadFromFile(inFile);
            }
            else {
                importData.ReadFromFileSwapEndian(inFile);
            }
            inFile.close();
            assert(importData.versionTracker == EXPECTED_IMPORT_VERSION);

            partialAnimationData.resize(importData.animations.size());
            //printf("animationData size? : %d \n", animationData.size());
            uint32_t biggestBoneCount = 0;

            for (int i = 0; i < importData.animations.size(); i++) { //animation count
                uint32_t currentBoneCount = static_cast<uint32_t>(importData.animations[i][0].size());
                if (currentBoneCount > biggestBoneCount) { biggestBoneCount = currentBoneCount; }

                partialAnimationData[i].resize(importData.animations[i].size());
                for (int j = 0; j < importData.animations[i].size(); j++) { //animation duration
                    auto& boneMap = partialAnimationData[i][j];
                    for (auto const& boneData : importData.animations[i][j]) {
                        boneMap.try_emplace(boneData.boneID, boneData.boneTransform);
                    }
                }
            }

            defaultMatrix = importData.defaultBoneValues;
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
            ImportData::FullAnimData importData;
            if (endian) {
                importData.ReadFromFile(inFile);
            }
            else {
                importData.ReadFromFileSwapEndian(inFile);
            }
            inFile.close();
            //printf("after loading anim archive \n");
            assert(importData.versionTracker == EXPECTED_IMPORT_VERSION && "failed tom match version");
            

            fullAnimationData.resize(importData.animations.size());
            //printf("animationData size? : %d \n", animationData.size());
            //uint32_t biggestBoneCount = 0;
            for (int i = 0; i < importData.animations.size(); i++) { //animation count
                //uint32_t currentBoneCount = importData.animations[i][0].size();
                //if (currentBoneCount > biggestBoneCount) { biggestBoneCount = currentBoneCount; }

                fullAnimationData[i].resize(importData.animations[i].size());
                for (int j = 0; j < importData.animations[i].size(); j++) { //animation duration
                    auto& fullAnimVec = fullAnimationData[i][j];
                    for (auto& boneData : importData.animations[i][j]) {
                        fullAnimVec.emplace_back(boneData);
                    }
                }
            }
            boneCount = static_cast<uint16_t>(fullAnimationData[0][0].size());
        }
    }

    void SkeletonBase::LoadTextures(std::string filePath, std::pair<std::vector<MaterialInfo>, std::vector<MaterialInfo>>& textureTracker, std::string texturePath) {
        //printf("failed to open : %s \n", filePath.c_str());
        std::ifstream inFile(filePath, std::ifstream::binary);
        if (!inFile.is_open()) {
            printf("failed to open : %s \n", filePath.c_str());
            //std throw
        }
        //printf("before opening name archive \n");
        ImportData::NameExportData importData;
        importData.ReadFromFile(inFile);
        inFile.close();
        //TEXTURES
        //this should be put in a separate function but im too lazy rn
        //printf("before textures \n");

        for (int i = 0; i < importData.meshNames.size(); i++) {
            importData.meshNames[i] = importData.meshNames[i].substr(0, importData.meshNames[i].find_first_of('.'));
            std::string finalDir = texturePath;
            finalDir += importData.meshNames[i];
            
            textureTracker.first.emplace_back(Material_Image::CreateMaterialImage(finalDir, true, true));
            
        }
        //printf("after mesh texutres \n");
        for (int i = 0; i < importData.meshNTNames.size(); i++) {
            importData.meshNTNames[i] = importData.meshNTNames[i].substr(0, importData.meshNTNames[i].find_first_of("."));
            std::string finalDir = texturePath;
            finalDir += importData.meshNTNames[i];

            textureTracker.second.emplace_back(Material_Image::CreateMaterialImage(finalDir, true, true));
        }
        //printf("after mesh nt texutres \n");
    }

    SkeletonBase::SkeletonBase(std::string importPath, std::string texturePath, bool instanced) {
        printf("skeleton : improting data : %s \n", importPath.c_str());

        mySkeletonID = SkinRenderSystem::GetSkinID();

        uint32_t endianTest = 1;
        bool endian = (*((char*)&endianTest) == 1);

        std::string meshPath = importPath;
        meshPath += "_mesh.ewe";
        std::thread meshThread1;
        bool meshThread1Exist = std::filesystem::exists(meshPath);
        ImportData::TemplateMeshData<boneVertex> importMesh;
#if !TEST_NO_MESH
        if (meshThread1Exist) {
            //printf("starting up mesh thread 1 \n");
            meshThread1 = std::thread(&ImportData::ReadData<boneVertex>, std::ref(importMesh), meshPath, endian);
        }
        else {
            //printf("skeleton mesh path doesn't exist : %s \n", meshPath.c_str());
        }

        meshPath = importPath + "_meshNT.ewe";
        std::thread meshThread2;
        bool meshThread2Exist = std::filesystem::exists(meshPath);
        ImportData::TemplateMeshData<boneVertexNoTangent> importMeshNT;
        if (meshThread2Exist) {
            //printf("starting up mesh thread 2 \n");
            meshThread2 = std::thread(&ImportData::ReadData<boneVertexNoTangent>, std::ref(importMeshNT), meshPath, endian);
        }
        else {
            //printf("skeleton mesh NT path doesn't exist : %s \n", meshPath.c_str());
        }
#endif
        bool partial = true;
        meshPath = importPath + "_anim.ewe";
        if (!std::filesystem::exists(meshPath)) {
            //printf("skeleton partial anim path doesn't exist : %s \n", meshPath.c_str());
            //printf("finna get an error \n");
            partial = false;
            meshPath = importPath + "_fullAnim.ewe";
            assert(std::filesystem::exists(meshPath) && "couldn't find either anim path for skeleton");
        }

        std::pair<std::vector<MaterialInfo>, std::vector<MaterialInfo>> textureMappingTracker;
        ReadAnimData(meshPath, partial, endian);

        LoadTextures(importPath + "_Names.ewe", textureMappingTracker, texturePath);

        //printf("textures created \n");

        //printf("handBone value : %d \n", handBone);
        //printf("thread existance? : %d:%d \n", meshThread1Exist, meshThread2Exist);
#if !TEST_NO_MESH
        if (meshThread1Exist) {
            //printf("waiting on mesh thread 1 \n");
            meshThread1.join();
            //printf("mesh thread 1 finished \n");

            for (auto const& mesh : importMesh.meshes) {
                meshes.push_back(Construct<EWEModel>({ reinterpret_cast<const void*>(mesh.vertices.data()), mesh.vertices.size(), importMesh.vertex_size, mesh.indices}));
            }
        }
        if (meshThread2Exist) {
            //printf("waiting on mesh thread 2 \n");
            meshThread2.join();

            for (auto const& mesh : importMeshNT.meshes) {
                meshes.push_back(Construct<EWEModel>({ reinterpret_cast<const void*>(mesh.vertices.data()), mesh.vertices.size(), importMeshNT.vertex_size, mesh.indices}));
            }
            //printf("mesh thread 2 finished \n");
        }
#endif

#if EWE_DEBUG
        assert(textureMappingTracker.first.size() == meshes.size() && "failed to match mesh to name");
        assert(textureMappingTracker.second.size() == meshesNT.size() && "failed to match meshNT to nameNT");
#endif

        for (int i = 0; i < meshes.size(); i++) {
            SkinRenderSystem::AddSkeleton(textureMappingTracker.first[i], boneCount, meshes[i], mySkeletonID, instanced);
        }
        for (int i = 0; i < meshesNT.size(); i++) {
            SkinRenderSystem::AddSkeleton(textureMappingTracker.second[i], boneCount, meshesNT[i], mySkeletonID, instanced);
        }

    }

}