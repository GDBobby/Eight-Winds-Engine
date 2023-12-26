#pragma once


#include "EWEngine/graphics/EWE_Device.hpp"
#include "EWEngine/Data/TransformInclude.h"

#include <vector>
#include <map>

#define MAX_BONE_INFLUENCE 4
namespace EWE {
    struct BoneInfo {
        /*id is index in finalBoneMatrices*/
        int id;

        /*offset matrix transforms vertex from model space to bone space*/
        glm::mat4 offset;

    };
    struct bobvec3 {
        float x{ 0.f };
        float y{ 0.f };
        float z{ 0.f };

        bobvec3() {}
        bobvec3(float all) : x{ all }, y{ all }, z{ all } {}
        bobvec3(float x, float y, float z) :x{ x }, y{ y }, z{ z } {}
        bobvec3(glm::vec3& glmVec) {
            x = glmVec.x;
            y = glmVec.y;
            z = glmVec.z;
        }

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& x;
            archive& y;
            archive& z;
        }
        bool operator == (const bobvec3& other) {
            return (this->x == other.x) && (this->y == other.y) && (this->z == other.z);
        }
        void operator=(const bobvec3& other) {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
        }
    };
    struct bobvec2 {
        float x{ 0.f };
        float y{ 0.f };

        bobvec2() {}
        bobvec2(float all) : x{ all }, y{ all } {}
        bobvec2(float x, float y) :x{ x }, y{ y } {}
        bobvec2(glm::vec2 glmVec) {
            x = glmVec.x;
            y = glmVec.y;
        }

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& x;
            archive& y;
        }
        bool operator == (const bobvec2& other) {
            return (this->x == other.x) && (this->y == other.y);
        }
        void operator = (const bobvec2& other) {
            this->x = other.x;
            this->y = other.y;
        }
    };
    struct bobVertex {

        // position
        bobvec3 position{ 0.f };
        bobvec3 normal{ 0.f };
        bobvec2 uv{ 0.f };
        bobvec3 tangent{ 0.f };

        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE];
        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE];

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const bobVertex& other) {
            if (!(this->position == other.position)) {
                printf("position fail bonevertex \n");
                return false;
            }
            if (!(this->normal == other.normal)) {
                printf("normal fail bonevertex \n");
                return false;
            }
            if (!(this->uv == other.uv)) {
                printf("uv fail, bonevertex \n");
                return false;
            }
            if (!(this->tangent == other.tangent)) {
                printf("tangent fail, bonevertex \n");
                return false;
            }
            for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                if (this->m_BoneIDs[i] != other.m_BoneIDs[i]) {
                    printf("bone id fail, bone vertex : %d \n", i);
                    return false;
                }
                if (this->m_Weights[i] != other.m_Weights[i]) {

                    printf("bone weight fail, bone vertex : %d \n", i);
                    return false;
                }
            }
            return true;
        }

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& position;
            archive& normal;
            archive& uv;
            archive& tangent;
            archive& m_BoneIDs;
            archive& m_Weights;
        }
    };
    struct bobSimpleVertex {
        bobvec3 position;
        bobvec3 normal;
        bobvec2 uv;
        bobvec3 color{ 1.f };

        bool operator==(const bobSimpleVertex& other) {
            return (position == other.position) && (color == other.color) && (normal == other.normal) && (uv == other.uv);
        }
        bobSimpleVertex() {}// : color{ 1.f } {}
        bobSimpleVertex(glm::vec3& position, glm::vec3& color, glm::vec3& normal, glm::vec2& uv) {
            this->position = position;
            this->normal = normal;

            this->uv = uv;

            this->color = color;
        }
        bobSimpleVertex(bobvec3 position, bobvec3 normal, bobvec2 uv, bobvec3 color) : position{ position }, normal{ normal }, uv{ uv }, color{ color } {}
        //bobSimpleVertex(bobAVertex* other);

        //template<typename T>
        void operator=(const bobSimpleVertex& other) {
            memcpy(this, &other, sizeof(bobSimpleVertex));
            //memcpy(&position, &other.position, FLOAT_SIZE3);
            //memcpy(&normal, &other.normal, FLOAT_SIZE3);
            //memcpy(&uv, &other.uv, FLOAT_SIZE2);
            //memcpy(&color, &other.color, FLOAT_SIZE3);
        }
        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& position;
            archive& normal;
            archive& uv;
            archive& color;
        }
    };
    struct bobAVertex {
        bobvec3 position{};
        bobvec3 normal{ 0.f,1.f,0.f };
        bobvec2 uv{};
        bobvec3 tangent{ 1.f,0.f,0.f };

        //static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();// { return AVertex::getAttributeDescriptions(); }

        bobAVertex() {}
        bobAVertex(float posx, float posy, float posz, float normalx, float normaly, float normalz, float uvx, float uvy, float tangentx, float tangenty, float tangentz) :
            position{ posx, posy, posz }, normal{ normalx, normaly, normalz }, uv{ uvx, uvy }, tangent{ tangentx, tangenty, tangentz }
        {}
        bobAVertex(bobvec3& position, bobvec3& normal, bobvec2& uv, bobvec3& tangent) : position{ position }, normal{ normal }, uv{ uv }, tangent{ tangent } {}

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& position;
            archive& normal;
            archive& uv;
            archive& tangent;
        }

        bool operator==(const bobAVertex& other) {
            if (!(this->position == other.position)) {
                printf("position fail bonevertex \n");
                return false;
            }
            if (!(this->normal == other.normal)) {
                printf("normal fail bonevertex \n");
                return false;
            }
            if (!(this->uv == other.uv)) {
                printf("uv fail, bonevertex \n");
                return false;
            }
            if (!(this->tangent == other.tangent)) {
                printf("tangent fail, bonevertex \n");
                return false;
            }
            return true;
        }
    };
    struct bobAVertexNT {
        bobvec3 position{};
        bobvec3 normal{};
        bobvec2 uv{};

        //static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();// { return AVertex::getAttributeDescriptions(); }

        bobAVertexNT() {}
        bobAVertexNT(float posx, float posy, float posz, float normalx, float normaly, float normalz, float uvx, float uxy) :
            position{ posx, posy, posz }, normal{ normalx, normaly, normalz }, uv{ uvx, uxy }
        {}
        bobAVertexNT(bobvec3& position, bobvec3& normal, bobvec2& uv) : position{ position }, normal{ normal }, uv{ uv } {}

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& position;
            archive& normal;
            archive& uv;
        }

        bool operator==(const bobAVertexNT& other) {
            if (!(this->position == other.position)) {
                printf("position fail bonevertex \n");
                return false;
            }
            if (!(this->normal == other.normal)) {
                printf("normal fail bonevertex \n");
                return false;
            }
            if (!(this->uv == other.uv)) {
                printf("uv fail, bonevertex \n");
                return false;
            }
            return true;
        }
    };

    struct boneVertex {
        glm::vec3 position{ 0.f };
        glm::vec3 normal{ 0.f };
        glm::vec2 uv{ 0.f };
        glm::vec3 tangent{ 0.f };

        int m_BoneIDs[MAX_BONE_INFLUENCE];
        float m_Weights[MAX_BONE_INFLUENCE];

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    };
    struct glmVertexNoTangent {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
        int m_BoneIDs[MAX_BONE_INFLUENCE];
        float m_Weights[MAX_BONE_INFLUENCE];
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    struct boneVertexNoTangent {
        bobvec3 position;
        bobvec3 normal;
        bobvec2 uv;

        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE];
        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE];
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() { return glmVertexNoTangent::getAttributeDescriptions(); }

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& position;
            archive& normal;
            archive& uv;
            archive& m_BoneIDs;
            archive& m_Weights;
        }
    };
    struct skyVertex {
        glm::vec3 position{ 0.f };

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    struct AVertex {
        glm::vec3 position{ 0.f };
        glm::vec3 normal{ 0.f };
        glm::vec2 uv{ 0.f };
        glm::vec3 tangent{ 0.f };

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    struct AVertexNT {
        glm::vec3 position{ 0.f };
        glm::vec3 normal{ 0.f };
        glm::vec2 uv{ 0.f };
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    struct simpleVertex {
        glm::vec3 position{ 0.f };
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        bool operator ==(const simpleVertex& other) const {
            return position == other.position;
        }
    };
    struct GrassVertex {
        glm::vec3 position{ 0.f };
        glm::vec3 color{ 0.f };
        //float uv;
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        bool operator ==(const GrassVertex& other) const {
            return position == other.position && color == other.color;
        }
    };
    struct GrassInstance {
        glm::mat4 transform;
        glm::vec2 uv;
        GrassInstance(glm::mat4 transform, glm::vec2 uv) : transform{ transform }, uv{ uv } {}
    };
    struct LeafVertex {
        bobvec3 position{ 0.f };
        bobvec3 normal{ 0.f };
        bobvec2 uv{ 0.f };
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& position;
            archive& normal;
            archive& uv;
        }
    };
    struct LeafInstance {
        glm::mat4 transform;
        LeafInstance(glm::mat4 transform) : transform{ transform } {}
    };

    struct EffectVertex {
        glm::vec3 position{ 0.f };
        glm::vec2 uv{ 0.f };
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        bool operator ==(const EffectVertex& other) const {
            return position == other.position && uv == other.uv;
        }
    };
    struct Vertex {
        glm::vec3 position{ 0.f };
        glm::vec3 normal{ 0.f };
        glm::vec2 uv{ 0.f };
        glm::vec3 color{ 0.f };

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();



        bool operator==(const Vertex& other) const {
            return position == other.position && color == other.color && normal == other.normal &&
                uv == other.uv;
        }
    };
    struct VertexUI {
        glm::vec2 position{ 0.f };
        glm::vec2 uv{ 0.f };

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
}