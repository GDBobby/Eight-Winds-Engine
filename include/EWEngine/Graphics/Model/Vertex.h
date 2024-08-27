#pragma once


#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Data/TransformInclude.h"
#include "EWEngine/Data/ReadEWEFromFile.h"

#include <vector>
#include <map>

#define DEBUGGING_MODEL_LOAD false




#define MAX_BONE_INFLUENCE 4
namespace EWE {
    struct BoneInfo {
        /*id is index in finalBoneMatrices*/
        int id;

        /*offset matrix transforms vertex from model space to bone space*/
        glm::mat4 offset;

    };
    struct boneVertex {
        glm::vec3 position{ 0.f };
        glm::vec3 normal{ 0.f };
        glm::vec2 uv{ 0.f };
        glm::vec3 tangent{ 0.f };

        int m_BoneIDs[MAX_BONE_INFLUENCE];
        float m_Weights[MAX_BONE_INFLUENCE];

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

        void swapEndian();
    };
    struct boneVertexNoTangent {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;

        int m_BoneIDs[MAX_BONE_INFLUENCE];
        float m_Weights[MAX_BONE_INFLUENCE];

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

        void swapEndian();
    };
    struct SkyVertex {
        glm::vec3 position{ 0.f };

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
    };
    struct Vertex {
        glm::vec3 position{ 0.f };
        glm::vec3 normal{ 0.f };
        glm::vec2 uv{ 0.f };
        glm::vec3 tangent{ 0.f };

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

        void swapEndian();
    };
    struct VertexNT {
        glm::vec3 position{ 0.f };
        glm::vec3 normal{ 0.f };
        glm::vec2 uv{ 0.f };
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

        bool operator==(VertexNT const& other) const {
            return (position == other.position) && (normal == other.normal) && (uv == other.uv);
        }

        void swapEndian();
    };
    struct SimpleVertex {
        glm::vec3 position{ 0.f };
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
        bool operator ==(const SimpleVertex& other) const {
            return position == other.position;
        }
    };
    struct GrassVertex {
        glm::vec3 position{ 0.f };
        glm::vec3 color{ 0.f };
        //float uv;
        static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
        bool operator ==(const GrassVertex& other) const {
            return position == other.position && color == other.color;
        }
    };
    struct GrassInstance {
        glm::mat4 transform;
        GrassInstance(glm::mat4 transform) : transform{ transform } {}
    };

    struct TransformInstance {
        glm::mat4 transform;
        TransformInstance(glm::mat4 transform) : transform{ transform } {}
    };

    struct EffectVertex {
        glm::vec3 position{ 0.f };
        glm::vec2 uv{ 0.f };
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
        bool operator ==(const EffectVertex& other) const {
            return position == other.position && uv == other.uv;
        }
    };
    struct TileVertex {
        glm::vec2 uv{ 0.f };
        static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
        bool operator ==(const TileVertex& other) const {
            return uv == other.uv;
        }
    };
    struct TileInstance{
        glm::vec2 uvOffset;
        TileInstance(glm::vec2 uv) : uvOffset{ uv } {}
    };
    //struct Vertex {
    //    glm::vec3 position{ 0.f };
    //    glm::vec3 normal{ 0.f };
    //    glm::vec2 uv{ 0.f };
    //    glm::vec3 color{ 0.f };

    //    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

    //    bool operator==(const Vertex& other) const {
    //        return position == other.position && color == other.color && normal == other.normal &&
    //            uv == other.uv;
    //    }
    //};
    struct VertexColor {
        glm::vec3 position{ 0.f };
        glm::vec3 normal{ 0.f };
        glm::vec2 uv{ 0.f };
        glm::vec3 color{ 0.f };

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

        bool operator==(const VertexColor& other) const {
            return position == other.position && color == other.color && normal == other.normal &&
                uv == other.uv;
        }
    };

    struct VertexUI {
        glm::vec2 position{ 0.f };
        glm::vec2 uv{ 0.f };

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
    };
    struct VertexGrid2D {
        glm::vec2 position;
        VertexGrid2D() : position{ 0.f } {}
        VertexGrid2D(float x, float y) : position{ x, y } {}

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
    };

    template <typename V_Type>
    struct MeshData {
        std::vector<V_Type> vertices{};
        std::vector<uint32_t> indices{};

        MeshData() {}
        MeshData(std::vector<V_Type> const& vertices, std::vector<uint32_t> const& indices) : vertices{ vertices }, indices{ indices } {}
        MeshData(std::pair<std::vector<V_Type>, std::vector<uint32_t>> const& pairData) : vertices{ pairData.first }, indices{ pairData.second } {}



        void swapEndian() {
            for (auto& vertex : vertices) {
                vertex.swapEndian();
            }
            for (auto& index : indices) {
                Reading::swapBasicEndian(&index, sizeof(uint32_t));
            }
        }
        //EVENTUALLY swap to a point where attribute descriptions are read, with a switch statement, instead of hard coding the vertex read.
        //this will allow for a vertex to be popped in without writign a specific read statement for it
        void readFromFile(std::ifstream& inFile) {

            uint64_t size;
            Reading::UInt64FromFile(inFile, &size);
#if DEBUGGING_MODEL_LOAD
            printf("after reading vertex count file pos : %zu \n", static_cast<std::streamoff>(inFile.tellg()));
#endif
            vertices.resize(size);
#if DEBUGGING_MODEL_LOAD
            printf("vertex size : %zu:%zu \n", sizeof(V_Type), size);
#endif
            inFile.read(reinterpret_cast<char*>(&vertices[0]), size * sizeof(V_Type));
#if DEBUGGING_MODEL_LOAD
            printf("after reading vertices data file pos : %zu \n", static_cast<std::streamoff>(inFile.tellg()));
            printf("before reading index size \n");
#endif
            Reading::UInt64FromFile(inFile, &size);

#if DEBUGGING_MODEL_LOAD
            printf("after reading index count file pos : %zu \n", static_cast<std::streamoff>(inFile.tellg()));
#endif
            indices.resize(size);

#if DEBUGGING_MODEL_LOAD
            printf("indices size : %zu \n", size);
#endif
            inFile.read(reinterpret_cast<char*>(&indices[0]), size * sizeof(uint32_t));

#if DEBUGGING_MODEL_LOAD
            printf("after reading indices data file pos : %zu \n", static_cast<std::streamoff>(inFile.tellg()));
#endif

        }
        void readFromFileSwapEndian(std::ifstream& inFile) {

            uint64_t size;
            Reading::UInt64FromFileSwapEndian(inFile, &size);
            vertices.resize(size);
            inFile.read(reinterpret_cast<char*>(&vertices[0]), size * sizeof(V_Type));

            Reading::UInt64FromFileSwapEndian(inFile, &size);
            indices.resize(size);
            inFile.read(reinterpret_cast<char*>(&indices[0]), size * sizeof(uint32_t));

            swapEndian();
        }

    protected:
    };
}