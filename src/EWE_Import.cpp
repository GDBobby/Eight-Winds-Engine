#include "EWEngine/Data/EWE_Import.h"

#include <thread>

#define MODEL_PATH "models/"

#include "EWEngine/Systems/ThreadPool.h"

namespace EWE {

    ImportData ImportData::LoadData(std::string importPath) {
        ImportData returnData;
        //printf("entering static load data function \n");
        std::string meshPath = MODEL_PATH;
        meshPath += importPath + "_mesh.ewe";

        bool meshThreadActive[2] = { false, false };
        bool meshThreadFinished[2] = { false, false };


        uint32_t testValue = 1;
        bool endian = *reinterpret_cast<uint8_t*>(&testValue) == 1;

        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[0] = true;
            ThreadPool::Enqueue(&ImportData::ReadData<boneVertex>, std::ref(returnData.meshExport), meshPath, endian);
            meshThreadFinished[0] = true;
        }
        else {
            //printf("mesh NT path doesn't exist : %s \n", meshPath.c_str());
        }

        meshPath = MODEL_PATH + importPath + "_meshNT.ewe";
        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[1] = true;
            ThreadPool::Enqueue(&ImportData::ReadData<boneVertexNoTangent>, std::ref(returnData.meshNTExport), meshPath, endian);
            meshThreadFinished[1] = true;
        }
        else {
            //printf("mesh NT path doesn't exist : %s \n", meshPath.c_str());
        }

        meshPath = MODEL_PATH + importPath + "_Names.ewe";
        if (std::filesystem::exists(meshPath)) {
            std::ifstream inFile(meshPath, std::ifstream::binary);
            if (!inFile.is_open()) {
                printf("failed to open : %s \n", meshPath.c_str());
                assert(false);
            }
            // printf("before formating input file in mesh \n");
            returnData.nameExport.ReadFromFile(inFile);

            inFile.close();
            //printf("file read successfully \n");
        }


        for (int i = 0; i < 2; i++) {
            if (meshThreadActive[i]) {
                while (!meshThreadFinished[i]) {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
#if EWE_DEBUG
                    printf("waiting on simple mesh thread : %d \n", i);
#endif
                }
                meshThreadActive[i] = false;
            }
        }

        meshPath = MODEL_PATH + importPath + "_simpleMesh.ewe";
        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[0] = true;
            ThreadPool::Enqueue(&ImportData::ReadData<Vertex>, std::ref(returnData.meshSimpleExport), meshPath, endian);
            meshThreadFinished[0] = true;
        }

        meshPath = MODEL_PATH + importPath + "_simpleMeshNT.ewe";
        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[1] = true;
            ThreadPool::Enqueue(&ImportData::ReadData<VertexNT>, std::ref(returnData.meshNTSimpleExport), meshPath, endian);
            meshThreadFinished[1] = true;
        }


        for (int i = 0; i < 2; i++) {
            if (meshThreadActive[i]) {
                while (!meshThreadFinished[i]) {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
#if EWE_DEBUG
                    printf("waiting on simple mesh thread : %d \n", i);
#endif
                }
                meshThreadActive[i] = false;
            }
        }

        //printf("returning from static import load function \n");
        return returnData;
    }


    void ImportData::NameExportData::ReadFromFile(std::ifstream& inFile) {
        std::getline(inFile, versionTracker, (char)0);
        assert(versionTracker == EXPECTED_IMPORT_VERSION);

        uint64_t size;
        inFile.read((char*)&size, sizeof(uint64_t));
        for (uint64_t i = 0; i < size; i++) {
            meshNames.emplace_back("");
            std::getline(inFile, meshNames.back(), (char)0);
        }

        inFile.read((char*)&size, sizeof(uint64_t));
        for (uint64_t i = 0; i < size; i++) {
            meshNTNames.emplace_back("");
            std::getline(inFile, meshNTNames.back(), (char)0);
        }

        inFile.read((char*)&size, sizeof(uint64_t));
        for (uint64_t i = 0; i < size; i++) {
            meshSimpleNames.emplace_back("");
            std::getline(inFile, meshSimpleNames.back(), (char)0);
        }

        inFile.read((char*)&size, sizeof(uint64_t));
        for (uint64_t i = 0; i < size; i++) {
            meshNTSimpleNames.emplace_back("");
            std::getline(inFile, meshNTSimpleNames.back(), (char)0);
        }
    }

    void ImportData::boneEData::ReadFromFile(std::ifstream& inFile) {
        Reading::UIntFromFile(inFile, &boneID);
        Reading::GLMMat4FromFile(inFile, &boneTransform);
    }
    void ImportData::boneEData::ReadFromFileSwapEndian(std::ifstream& inFile) {
        Reading::UIntFromFileSwapEndian(inFile, &boneID);
        Reading::GLMMat4FromFileSwapEndian(inFile, &boneTransform);
    }
    /*
    template <typename V_Type>
    void ImportData::TemplateMeshData<V_Type>::readFromFile(std::ifstream& inFile) {
        std::getline(inFile, versionTracker, '\r');
        if (versionTracker != EXPECTED_IMPORT_VERSION) {
            printf("incorrect import version \n");
            throw std::runtime_error("incorrect import version");
        }
        if (inFile.peek() == '\n') {
            inFile.seekg(1, std::ios::cur);
        }

        uint64_t size;
        Reading::UInt64FromFile(inFile, &size);
        meshes.resize(size);
        for (auto& mesh : meshes) {
            mesh.readFromFile(inFile);
        }

    }
    template <typename V_Type>
    void ImportData::TemplateMeshData<V_Type>::readFromFileSwapEndian(std::ifstream& inFile) {
        std::getline(inFile, versionTracker);
        if (versionTracker != EXPECTED_IMPORT_VERSION) {
            printf("incorrect import version \n");
            throw std::runtime_error("incorrect import version");
        }

        uint64_t size;
        Reading::UInt64FromFileSwapEndian(inFile, &size);
        meshes.resize(size);
        for (auto& mesh : meshes) {
            mesh.readFromFileSwapEndian(inFile);
            //mesh.swapEndian();
        }
    }
    */

    void ImportData::AnimData::ReadFromFile(std::ifstream& inFile) {
        std::getline(inFile, versionTracker, (char)0);
        assert(versionTracker == EXPECTED_IMPORT_VERSION);

        uint64_t size;
        inFile.read((char*)&size, sizeof(uint64_t));
        defaultBoneValues.resize(size);
        inFile.read(((char*)defaultBoneValues.data()), size * sizeof(glm::mat4));

        inFile.read((char*)&size, sizeof(uint64_t));
        animations.resize(size);
        for (auto& animationDuration : animations) {
            inFile.read((char*)&size, sizeof(uint64_t));
            animationDuration.resize(size);
            for (auto& boneCount : animationDuration) {
                inFile.read((char*)&size, sizeof(uint64_t));
                boneCount.resize(size);

                //if i pack the structure correctly, i can read it as a block. might impact runtime speed, which is the main concern.
                for (auto& boneData : boneCount) {
                    boneData.ReadFromFile(inFile);
                }
            }
        }
        Reading::IntFromFile(inFile, &handBone);
    }
    void ImportData::AnimData::ReadFromFileSwapEndian(std::ifstream& inFile) {
        std::getline(inFile, versionTracker, (char)0);
        assert(versionTracker == EXPECTED_IMPORT_VERSION);

        uint64_t size;
        Reading::UInt64FromFileSwapEndian(inFile, &size);

        defaultBoneValues.resize(size);
        inFile.read(((char*)defaultBoneValues.data()), size * sizeof(glm::mat4));
        for (auto& defaultBone : defaultBoneValues) {
            Reading::swapGLMMat4Endian(defaultBone);
        }

        Reading::UInt64FromFileSwapEndian(inFile, &size);
        animations.resize(size);
        for (auto& animationDuration : animations) {
            Reading::UInt64FromFileSwapEndian(inFile, &size);
            animationDuration.resize(size);
            for (auto& boneCount : animationDuration) {
                Reading::UInt64FromFileSwapEndian(inFile, &size);
                boneCount.resize(size);

                //if i pack the structure correctly, i can read it as a block. might impact runtime speed, which is the main concern.
                for (auto& boneData : boneCount) {
                    boneData.ReadFromFileSwapEndian(inFile);
                }
            }
        }
        Reading::IntFromFileSwapEndian(inFile, &handBone);
    }

    void ImportData::FullAnimData::ReadFromFile(std::ifstream& inFile) {
        std::getline(inFile, versionTracker, (char)0);
        assert(versionTracker == EXPECTED_IMPORT_VERSION);

        uint64_t size;
        Reading::UInt64FromFile(inFile, &size);
        animations.resize(size);

        for (auto& animationDuration : animations) {
            Reading::UInt64FromFile(inFile, &size);
            animationDuration.resize(size);

            for (auto& boneCount : animationDuration) {
                Reading::UInt64FromFile(inFile, &size);
                boneCount.resize(size);
                inFile.read(reinterpret_cast<char*>(&boneCount[0]), size * sizeof(glm::mat4));
            }
        }
        Reading::IntFromFile(inFile, &handBone);
    }
    void ImportData::FullAnimData::ReadFromFileSwapEndian(std::ifstream& inFile) {
        std::getline(inFile, versionTracker);
        assert(versionTracker == EXPECTED_IMPORT_VERSION);

        uint64_t size;
        Reading::UInt64FromFileSwapEndian(inFile, &size);
        animations.resize(size);

        for (auto& animationDuration : animations) {
            Reading::UInt64FromFileSwapEndian(inFile, &size);
            animationDuration.resize(size);

            for (auto& boneCount : animationDuration) {
                Reading::UInt64FromFileSwapEndian(inFile, &size);
                boneCount.resize(size);

                inFile.read((char*)boneCount.data(), size * sizeof(glm::mat4));
                for (auto& bone : boneCount) {
                    Reading::swapGLMMat4Endian(bone);
                }
            }
        }
        Reading::IntFromFileSwapEndian(inFile, &handBone);
    }
}