#pragma once

#include "EWEngine/Data/EWE_Import.h"
#include "EWEngine/Data/EngineDataTypes.h"


namespace EWE {
    class SkeletonBase {
    public:
        static uint32_t nextSkeletonID;

        SkeletonBase(std::string importPath, std::string texturePath, bool instanced = false);
#if DECONSTRUCTION_DEBUG
        ~SkeletonBase() {
            printf("Deconstructing player skeleton \n");
            for (auto& mesh : meshes) {
                delete mesh;
            }
            for (auto& mesh : meshesNT) {
                delete mesh;
            }
        }
#endif
        std::vector<EWEModel*> meshes{}; //this will hold the vertex/index buffers
        std::vector<EWEModel*> meshesNT{};
        std::vector<ImageID> textureIDs{};
        std::vector<ImageID> textureNTIDs{};

        std::vector<std::pair<MaterialFlags, //pipeline flags
            std::vector<std::pair<ImageID, //texture
            std::vector<EWEModel* //meshPtr
            >>>>> pipePairs{};

        //pipePairs.first == MaterialFlags
        //pipePairs.second = textureVector
        //pipePairs.second[0].second == meshID
        //so the drawing would look like this
        //for each pipeFlag, bind pipeline, for each texture, bind texture, for each mesh, draw mesh

        //std::unordered_map<uint8_t, std::vector<std::pair<uint8_t, int32_t>>> pipelinePairs; //in the pair, first is mesh index, second is texture id
       // std::unordered_map<uint8_t, std::vector<std::pair<uint8_t, int32_t>>> pipelinePairsNT; //2 pairs or an if statement to branch between mesh and meshNT?
        //struct for tangent mesh and meshNT?

        virtual void* GetFinalBones(uint8_t animState, uint16_t animFrames) = 0;
        const glm::mat4& GetSwordMatrix() { return swordMatrix; }

        uint32_t GetBoneCount() { return boneCount; }
        uint32_t GetSkeletonID() { return mySkeletonID; }

    protected:
        std::vector< //which animation
            std::vector< //which animation frame
            std::map<uint32_t, glm::mat4>>> partialAnimationData{}; //the bone transform

        std::vector< //which animation
            std::vector< //which animation frame
            std::vector<glm::mat4>>> fullAnimationData{}; //the bone transform

        std::vector<glm::mat4> finalMatrix{};
        std::vector<glm::mat4> defaultMatrix{}; //a more natural T-Pose
        glm::mat4 swordMatrix{ 1.f };

        uint32_t mySkeletonID = 0;
        uint16_t boneCount = 0;
        int32_t handBone = -1;

        void ReadAnimData(std::string filePath, bool partial, bool endian);
        void LoadTextures(std::string filePath, std::pair<std::vector<MaterialInfo>, std::vector<MaterialInfo>>& textureTracker, std::string texturePath);
    };
}