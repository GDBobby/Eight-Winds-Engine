#pragma once

#include "EWGraphics/Model/Model.h"
#include "EWGraphics/Model/Vertex.h"
#include "EWEngine/Data/ReadEWEFromFile.h"

#include <fstream>
#include <vector>
#include <string>

#define EXPECTED_IMPORT_VERSION "2.1.0" //need to do some SHA256 key or some shit

#define DEBUGGING_MESH_LOAD false

namespace EWE {
    class ImportData {
    public:
        struct boneEData {
            uint32_t boneID{ 69420 };
            lab::mat4 boneTransform;

            boneEData() {}

            void ReadFromFile(std::ifstream& inFile);

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

            void ReadFromFile(std::ifstream& inFile) {
                std::getline(inFile, versionTracker, (char)0);
                if (strcmp(versionTracker.c_str(), EXPECTED_IMPORT_VERSION)) {
                    printf("incorrect import version : %s \n", versionTracker.c_str());
                    assert(false && "incorrect import version");
                }
                if (inFile.peek() == '\n') {
#if DEBUGGING_MESH_LOAD
                    printf(" foudn null after version \n");
#endif
                    inFile.seekg(1, std::ios::cur);
                }
#if DEBUGGING_MESH_LOAD
                printf("after reading version file pos : %zu \n", static_cast<std::streamoff>(inFile.tellg()));
#endif

                uint64_t size;
                Reading::UInt64FromFile(inFile, &size);
#if DEBUGGING_MESH_LOAD
                printf("after reading mesh count file pos : %zu \n", static_cast<std::streamoff>(inFile.tellg()));
                printf("size of meshes : %zu \n", size);
#endif
                meshes.resize(size);
                for (auto& mesh : meshes) {

                    uint64_t fileSize;
                    Reading::UInt64FromFile(inFile, &fileSize);
                    mesh.vertices.resize(fileSize);
                    inFile.read(reinterpret_cast<char*>(&mesh.vertices[0]), fileSize * sizeof(V_Type));

                    Reading::UInt64FromFile(inFile, &fileSize);
                    mesh.indices.resize(fileSize);
                    inFile.read(reinterpret_cast<char*>(&mesh.indices[0]), fileSize * sizeof(uint32_t));
                }

            }
        };

        struct AnimData {
            std::string versionTracker = "";
            std::vector<lab::mat4> defaultBoneValues; //T-POSE or something, for when an animation doesn't cover everything

            int32_t handBone = -1;

            //animation
            //1 duration will have 1 set of bones, 1 bone will hold a vector of matrix transforms by length of duration

            std::vector< //animation count
                std::vector< //animation duration
                std::vector< //bone count
                boneEData>>> //{bone id, bone transform}, bone ID will keep track of which bone as i clear useless bones. i could also use a map, might be better
                animations;

            void ReadFromFile(std::ifstream& inFile);

        };
        struct FullAnimData {
            std::string versionTracker = "";
            int32_t handBone = -1;

            std::vector< //each animation
                std::vector< //animation frame duration
                std::vector< //boneCount
                lab::mat4>>> animations;


            void ReadFromFile(std::ifstream& inFile);
        };
        struct NameExportData {
            std::string versionTracker = "";
            std::vector<std::string> meshNames;
            std::vector<std::string> meshNTNames;
            std::vector<std::string> meshSimpleNames;
            std::vector<std::string> meshNTSimpleNames;

            void ReadFromFile(std::ifstream& inFile);
        };

        TemplateMeshData<boneVertex> meshExport{};
        TemplateMeshData<boneVertexNoTangent> meshNTExport{};
        TemplateMeshData<Vertex> meshSimpleExport{};
        TemplateMeshData<VertexNT> meshNTSimpleExport{};
        AnimData animExport;
        NameExportData nameExport;

        template <typename T>
        static void ReadData(TemplateMeshData<T>& data, std::string meshPath) {
            //printf("starting up mesh thread :%s \n", meshPath.c_str());
            std::ifstream inFile(meshPath, std::ifstream::binary);
            //inFile.open();
            assert(inFile.is_open() && "failed to open file");  
#if EWE_DEBUG
            printf("reading templatemeshdata : %s\n", meshPath.c_str());
#endif
            data.ReadFromFile(inFile);

            inFile.close();
            //printf("file read successfully \n");
        }

        //static ImportData loadDataThreaded(std::string importPath);

        static ImportData LoadData(std::string importPath);

    };
}