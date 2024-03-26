#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/Model/Vertex.h"
#include "EWEngine/Data/ReadEWEFromFile.h"

#include <fstream>
#include <vector>
#include <string>

#define EXPECTED_IMPORT_VERSION "2.1.0" //need to do some SHA256 key or some shit

namespace EWE {
    class ImportData {
    public:
#pragma 
        struct boneEData {
            uint32_t boneID{ 69420 };
            glm::mat4 boneTransform;

            boneEData() {}

            void readFromFile(std::ifstream& inFile);
            void readFromFileSwapEndian(std::ifstream& inFile);

        };

        template <typename V_Type>
        struct TemplateMeshData {
            std::string versionTracker{ "" };
            std::vector<MeshData<V_Type>> meshes;
            static constexpr size_t vertex_size = sizeof(V_Type);

            TemplateMeshData(std::vector<V_Type>& vertex, std::vector<uint32_t>& index) {
                meshes.emplace_back(vertex, index);
            }
            TemplateMeshData() {}

            void readFromFile(std::ifstream& inFile) {
                std::getline(inFile, versionTracker, (char)0);
                if (strcmp(versionTracker.c_str(), EXPECTED_IMPORT_VERSION)) {
                    printf("incorrect import version : %s \n", versionTracker.c_str());
                    throw std::runtime_error("incorrect import version");
                }
                if (inFile.peek() == '\n') {
                    printf(" foudn null after version \n");
                    inFile.seekg(1, std::ios::cur);
                }
                printf("after reading version file pos : %lu \n", static_cast<std::streamoff>(inFile.tellg()));

                uint64_t size;
                Reading::UInt64FromFile(inFile, &size);
                printf("after reading mesh count file pos : %lu \n", static_cast<std::streamoff>(inFile.tellg()));
                printf("size of meshes : %lu \n", size);
                meshes.resize(size);
                for (auto& mesh : meshes) {
                    mesh.readFromFile(inFile);
                }

            }
            void readFromFileSwapEndian(std::ifstream& inFile) {
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
        };

        struct AnimData {
            std::string versionTracker = "";
            std::vector<glm::mat4> defaultBoneValues; //T-POSE or something, for when an animation doesn't cover everything

            int32_t handBone = -1;

            //animation
            //1 duration will have 1 set of bones, 1 bone will hold a vector of matrix transforms by length of duration

            std::vector< //animation count
                std::vector< //animation duration
                std::vector< //bone count
                boneEData>>> //{bone id, bone transform}, bone ID will keep track of which bone as i clear useless bones. i could also use a map, might be better
                animations;

            void readFromFile(std::ifstream& inFile);
            void readFromFileSwapEndian(std::ifstream& inFile);

        };
        struct FullAnimData {
            std::string versionTracker = "";
            int32_t handBone = -1;

            std::vector< //each animation
                std::vector< //animation frame duration
                std::vector< //boneCount
                glm::mat4>>> animations;


            void readFromFile(std::ifstream& inFile);
            void readFromFileSwapEndian(std::ifstream& inFile);
        };
        struct NameExportData {
            std::string versionTracker = "";
            std::vector<std::string> meshNames;
            std::vector<std::string> meshNTNames;
            std::vector<std::string> meshSimpleNames;
            std::vector<std::string> meshNTSimpleNames;

            void readFromFile(std::ifstream& inFile);
        };

        TemplateMeshData<boneVertex> meshExport{};
        TemplateMeshData<boneVertexNoTangent> meshNTExport{};
        TemplateMeshData<Vertex> meshSimpleExport{};
        TemplateMeshData<VertexNT> meshNTSimpleExport{};
        AnimData animExport;
        NameExportData nameExport;

        template <typename T>
        static void readData(TemplateMeshData<T>& data, std::string meshPath, bool endian) {
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

        //static ImportData loadDataThreaded(std::string importPath);

        static ImportData loadData(std::string importPath);

    };
}