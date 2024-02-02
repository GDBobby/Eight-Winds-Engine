#include "EWEngine/Data/EWE_Import.h"
#include "EWEngine/Data/ReadEWEFromFile.h"

#include <thread>

#define MODEL_PATH "models\\"

namespace EWE {
    template <typename T>
    void ImportData::readData(TemplateMeshData<T>& data, std::string meshPath, bool endian) {
        //printf("starting up mesh thread :%s \n", meshPath.c_str());
        std::ifstream inFile(meshPath, std::ifstream::binary);
        //inFile.open();
        if (!inFile.is_open()) {
            printf("failed to open : %s \n", meshPath.c_str());
            //std throw
        }
        if (endian) {
            data.readFromFile(inFile);
        }
        else {
            data.readFromFileSwapEndian(inFile);
        }
        inFile.close();
        //printf("file read successfully \n");
    }

    /*
    ImportData ImportData::loadDataThreaded(std::string importPath) {
        ImportData returnData;
        printf("entering static load data function \n");
        std::thread meshThread[2];
        std::string meshPath = importPath;
        meshPath += "_mesh.ewe";

        bool meshThreadActive[2] = { false, false };

        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[0] = true;
            meshThread[0] = std::thread(&ImportData::readData<meshEData>, std::ref(returnData.meshExport), meshPath);
        }
        else {
            printf("mesh path doesn't exist : %s \n", meshPath.c_str());
        }

        meshPath = importPath + "_meshNT.ewe";
        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[1] = true;
            meshThread[1] = std::thread(&ImportData::readData<meshNTEData>, std::ref(returnData.meshNTExport), meshPath);
        }
        else {
            printf("mesh NT path doesn't exist : %s \n", meshPath.c_str());
        }

        meshPath = importPath + "_Names.ewe";
        if (std::filesystem::exists(meshPath)) {
            std::ifstream inFile(meshPath, std::ifstream::binary);
            if (!inFile.is_open()) {
                printf("failed to open : %s \n", meshPath.c_str());
            }
            printf("before formatingg input file in mesh \n");
            boost::archive::binary_iarchive binary_input_archive(inFile, boost::archive::no_header);
            binary_input_archive& returnData.nameExport;
            inFile.close();
            printf("file read successfully \n");
        }


        for (int i = 0; i < 2; i++) {
            if (meshThreadActive[i]) {
                if (meshThread[i].joinable()) {
                    meshThread[i].join();
                }
                meshThreadActive[i] = false;
            }
        }

        meshPath = importPath + "_simpleMesh.ewe";
        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[0] = true;
            meshThread[0] = std::thread(&ImportData::readData<meshSimpleData>, std::ref(returnData.meshSimpleExport), meshPath);
        }

        meshPath = importPath + "_simpleMeshNT.ewe";
        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[1] = true;
            meshThread[1] = std::thread(&ImportData::readData<meshNTSimpleData>, std::ref(returnData.meshNTSimpleExport), meshPath);
        }

        for (int i = 0; i < 2; i++) {
            if (meshThreadActive[i]) {
                if (meshThread[i].joinable()) {
                    meshThread[i].join();
                }
                meshThreadActive[i] = false;
            }
        }

        printf("returning from static import load function \n");
        return returnData;
    }
    */

    ImportData ImportData::loadData(std::string importPath) {
        ImportData returnData;
        //printf("entering static load data function \n");
        std::thread meshThread[2];
        std::string meshPath = MODEL_PATH;
        meshPath += importPath + "_mesh.ewe";

        bool meshThreadActive[2] = { false, false };


        uint32_t testValue = 1;
        bool endian = *reinterpret_cast<uint8_t*>(&testValue) == 1;

        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[0] = true;
            meshThread[0] = std::thread(&ImportData::readData<boneVertex>, std::ref(returnData.meshExport), meshPath, endian);
        }
        else {
            //printf("mesh NT path doesn't exist : %s \n", meshPath.c_str());
        }

        meshPath = MODEL_PATH + importPath + "_meshNT.ewe";
        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[1] = true;
            meshThread[1] = std::thread(&ImportData::readData<boneVertexNoTangent>, std::ref(returnData.meshNTExport), meshPath, endian);
        }
        else {
            //printf("mesh NT path doesn't exist : %s \n", meshPath.c_str());
        }

        meshPath = MODEL_PATH + importPath + "_Names.ewe";
        if (std::filesystem::exists(meshPath)) {
            std::ifstream inFile(meshPath, std::ifstream::binary);
            if (!inFile.is_open()) {
                printf("failed to open : %s \n", meshPath.c_str());
            }
            // printf("before formating input file in mesh \n");
            returnData.nameExport.readFromFile(inFile);

            inFile.close();
            //printf("file read successfully \n");
        }


        for (int i = 0; i < 2; i++) {
            if (meshThreadActive[i]) {
                if (meshThread[i].joinable()) {
                    printf("waiting on mesh thread : %d \n", i);
                    meshThread[i].join();
                }
                meshThreadActive[i] = false;
            }
        }

        meshPath = MODEL_PATH + importPath + "_simpleMesh.ewe";
        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[0] = true;
            meshThread[0] = std::thread(&ImportData::readData<Vertex>, std::ref(returnData.meshSimpleExport), meshPath, endian);
        }

        meshPath = MODEL_PATH + importPath + "_simpleMeshNT.ewe";
        if (std::filesystem::exists(meshPath)) {
            meshThreadActive[1] = true;
            meshThread[1] = std::thread(&ImportData::readData<VertexNT>, std::ref(returnData.meshNTSimpleExport), meshPath, endian);
        }

        for (int i = 0; i < 2; i++) {
            if (meshThreadActive[i]) {
                if (meshThread[i].joinable()) {
                    printf("waiting on simple mesh thread : %d \n", i);
                    meshThread[i].join();
                }
                meshThreadActive[i] = false;
            }
        }

        //printf("returning from static import load function \n");
        return returnData;
    }


    void ImportData::NameExportData::readFromFile(std::ifstream& inFile) {
        std::getline(inFile, versionTracker);
        if (versionTracker != EXPECTED_IMPORT_VERSION) {
            printf("incorrect import version \n");
            throw std::runtime_error("incorrect import version");
        }

        uint64_t size;
        inFile.read((char*)&size, sizeof(uint64_t));
        for (uint64_t i = 0; i < size; i++) {
            meshNames.emplace_back("");
            std::getline(inFile, meshNames.back());
        }

        inFile.read((char*)&size, sizeof(uint64_t));
        for (uint64_t i = 0; i < size; i++) {
            meshNTNames.emplace_back("");
            std::getline(inFile, meshNTNames.back());
        }

        inFile.read((char*)&size, sizeof(uint64_t));
        for (uint64_t i = 0; i < size; i++) {
            meshSimpleNames.emplace_back("");
            std::getline(inFile, meshSimpleNames.back());
        }

        inFile.read((char*)&size, sizeof(uint64_t));
        for (uint64_t i = 0; i < size; i++) {
            meshNTSimpleNames.emplace_back("");
            std::getline(inFile, meshNTSimpleNames.back());
        }
    }

    void ImportData::boneEData::readFromFile(std::ifstream& inFile) {
        Reading::UIntFromFile(inFile, &boneID);
        Reading::GLMMat4FromFile(inFile, &boneTransform);
    }
    void ImportData::boneEData::readFromFileSwapEndian(std::ifstream& inFile) {
        Reading::UIntFromFileSwapEndian(inFile, &boneID);
        Reading::GLMMat4FromFileSwapEndian(inFile, &boneTransform);
    }

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


    void ImportData::AnimData::readFromFile(std::ifstream& inFile) {
        std::getline(inFile, versionTracker);
        if (versionTracker != EXPECTED_IMPORT_VERSION) {
            printf("incorrect import version \n");
            throw std::runtime_error("incorrect import version");
        }

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
                    boneData.readFromFile(inFile);
                }
            }
        }
        Reading::IntFromFile(inFile, &handBone);
    }
    void ImportData::AnimData::readFromFileSwapEndian(std::ifstream& inFile) {
        std::getline(inFile, versionTracker);
        if (versionTracker != EXPECTED_IMPORT_VERSION) {
            printf("incorrect import version \n");
            throw std::runtime_error("incorrect import version");
        }

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
                    boneData.readFromFileSwapEndian(inFile);
                }
            }
        }
        Reading::IntFromFileSwapEndian(inFile, &handBone);
    }

    void ImportData::FullAnimData::readFromFile(std::ifstream& inFile) {
        std::getline(inFile, versionTracker);
        if (versionTracker != EXPECTED_IMPORT_VERSION) {
            printf("incorrect import version \n");
            throw std::runtime_error("incorrect import version");
        }

        uint64_t size;
        inFile.read((char*)&size, sizeof(uint64_t));
        animations.resize(size);

        for (auto& animationDuration : animations) {
            inFile.read((char*)&size, sizeof(uint64_t));
            animationDuration.resize(size);

            for (auto& boneCount : animationDuration) {
                inFile.read((char*)&size, sizeof(uint64_t));
                boneCount.resize(size);
                inFile.read((char*)boneCount.data(), size * sizeof(glm::mat4));
            }
        }
        Reading::IntFromFile(inFile, &handBone);
    }
    void ImportData::FullAnimData::readFromFileSwapEndian(std::ifstream& inFile) {
        std::getline(inFile, versionTracker);
        if (versionTracker != EXPECTED_IMPORT_VERSION) {
            printf("incorrect import version \n");
            throw std::runtime_error("incorrect import version");
        }

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