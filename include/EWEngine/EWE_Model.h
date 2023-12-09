#pragma once

#include "graphics/EWE_Buffer.h"
#include "graphics/EWE_Device.hpp"
#include "Data/TransformInclude.h"

// libs

/*
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
*/
#define MAX_BONE_INFLUENCE 4

#define LEVEL_BUILDER false
// std
#include <memory>
#include <vector>
#include <map>
#include <format>
#include <iostream>

namespace EWE {
    constexpr size_t FLOAT_SIZE3 = sizeof(float) * 3;
    constexpr size_t FLOAT_SIZE2 = sizeof(float) * 2;

    struct MaterialComponent {
        //
        //glm::vec3 ambient{ 0.f };
        //glm::vec3 diffuse{ 0.f };
        //glm::vec3 specular{ 0.f };
        //float shininess{ 0.f };
        glm::vec4 metallicRoughnessOpacity;
        //float metallic;
        //float roughness;
    };
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
        bobvec3 normal{0.f,1.f,0.f};
        bobvec2 uv{};
        bobvec3 tangent{1.f,0.f,0.f};

        //static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();// { return EWEModel::AVertex::getAttributeDescriptions(); }

        bobAVertex() {}
        bobAVertex(float posx, float posy, float posz, float normalx, float normaly, float normalz, float uvx, float uvy, float tangentx, float tangenty, float tangentz) : 
        position{posx, posy, posz}, normal{normalx, normaly, normalz}, uv{uvx, uvy}, tangent{tangentx, tangenty, tangentz}
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

        //static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();// { return EWEModel::AVertex::getAttributeDescriptions(); }

        bobAVertexNT() {}
        bobAVertexNT(float posx, float posy, float posz, float normalx, float normaly, float normalz, float uvx, float uxy) :
            position{ posx, posy, posz }, normal{ normalx, normaly, normalz }, uv{uvx, uxy}
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


    class EWEModel {
    public:

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
            glm::vec2 uv{0.f};
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
            glm::vec3 position{0.f};
            glm::vec3 normal{0.f};
            glm::vec2 uv{0.f};
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

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);
        };
        struct SimpleBuilder {
            std::vector<simpleVertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);
        };
        struct GrassBuilder {
            std::vector<GrassVertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);
        };

        template <typename T>
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
            std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
            bindingDescriptions[0].binding = 0;
            //printf("size of BVNT:T - %d:%d \n", sizeof(boneVertexNoTangent),sizeof(T));
            bindingDescriptions[0].stride = sizeof(T);
            bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescriptions;
        }

        /*
                {{0.5f,0.0f, -0.5f}, {1.f, 1.f, 1.f}, {0.f,-1.f,0.f}, {1.0f,1.f}},
                {{-0.5f,0.0f, -0.5f}, {1.f, 1.f, 1.f}, {0.f,-1.f,0.f}, {0.0f,1.f}},
                {{-0.5f,0.0f, 0.5f}, {1.f, 1.f, 1.f}, {0.f,-1.f,0.f}, {0.0f,0.f}},
                {{0.5f,0.0f, 0.5f}, {1.f, 1.f, 1.f}, {0.f,-1.f,0.f}, {1.0f,0.f}},

        */
        static std::unique_ptr<EWEModel> generateQuad(EWEDevice& device, glm::vec2 uvScale = glm::vec2{1.f}) {
            std::vector<EWEModel::Vertex> vertices{
                {{0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {uvScale.x,uvScale.y}, {1.f, 1.f, 1.f}},
                {{-0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {0.0f,uvScale.y}, {1.f, 1.f, 1.f}},
                {{-0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {0.0f,0.f}, {1.f, 1.f, 1.f}},
                {{0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {uvScale.x,0.f}, {1.f, 1.f, 1.f}},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2,3,0 };
            return std::make_unique<EWEModel>(device, vertices, indices);
        }
        static std::unique_ptr<EWEModel> generateQuadPNU(EWEDevice& device, glm::vec2 uvScale = glm::vec2{ 1.f }) {
            std::vector<EWEModel::AVertexNT> vertices{
                {{0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {uvScale.x,uvScale.y}},
                {{-0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {0.0f,uvScale.y}},
                {{-0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {0.0f,0.f}},
                {{0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {uvScale.x,0.f}},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2,3,0 };
            return std::make_unique<EWEModel>(device, vertices, indices);
        }
        static std::unique_ptr<EWEModel> generateSimple3DQuad(EWEDevice& device, glm::vec2 uvScale = glm::vec2{ 1.f }) {
            std::vector<EWEModel::EffectVertex> vertices{
                {{0.5f,0.0f, -0.5f}, {uvScale.x,uvScale.y}},
                {{-0.5f,0.0f, -0.5f}, {0.0f,uvScale.y}},
                {{-0.5f,0.0f, 0.5f}, {0.0f,0.f}},
                {{0.5f,0.0f, 0.5f}, {uvScale.x,0.f}},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };
            return std::make_unique<EWEModel>(device, vertices, indices);
        }
        /*
        static std::unique_ptr<EWEModel> generateSimpleZedQuad(EWEDevice& device, glm::vec2 uvScale = glm::vec2{ 1.f }) {
            std::vector<EWEModel::EffectVertex> vertices{
                {{0.5f,0.0f, -0.5f}, {uvScale.x,uvScale.y}},
                {{-0.5f,0.0f, -0.5f}, {0.0f,uvScale.y}},
                {{-0.5f,0.0f, 0.5f}, {0.0f,0.f}},
                {{0.5f,0.0f, 0.5f}, {uvScale.x,0.f}},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };
            return std::make_unique<EWEModel>(device, vertices, indices);
        }
        */

        static std::unique_ptr<EWEModel> generate2DQuad(EWEDevice& device, glm::vec2 scale = {1.f,1.f}) {
            std::vector<EWEModel::VertexUI> vertices{
                {{-0.5f, -0.5f}, {0.f, 0.f}},
                {{0.5f, -0.5f}, {scale.x, 0.f}},
                {{0.5f, 0.5f}, {scale.x, scale.y}},
                {{-0.5f, 0.5f}, {0.f, scale.y}}
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };
            return std::make_unique<EWEModel>(device, vertices, indices);
        }
        
        static std::unique_ptr<EWEModel> generateNineUIQuad(EWEDevice& device) {
            std::vector<EWEModel::VertexUI> vertices{
                {{-0.5f, -0.5f}, {0.f, 0.f}}, //top left corner
                {{-.5f, -.5f}, {.0625f, .0625f}}, //inner top left corner

                {{0.5f, -0.5f}, {1.f, 0.f}}, //bottom left
                {{0.5f, -.5f}, {1.f - .0625f, .0625f}}, //inner bottom left

                {{0.5f, 0.5f}, {1.f, 1.f}}, //bottom right
                {{.5f, .5f}, {1.f - .0625f,1.f - .0625f}}, //inner bottom right

                {{-0.5f, 0.5f}, {0.f, 1.f}}, //top right
                {{-.5f, .5f}, {.0625f, 1.f - .0625f}}, //inner top right
            };
            std::vector<uint32_t> indices{1,0,6,1,6,7,1,7,3,1,3,2,1,2,0,5,4,2,5,2,3,5,3,7,5,7,6,5,6,4};
            return std::make_unique<EWEModel>(device, vertices, indices);
        }
        

        //static std::unique_ptr<EWEModel> generate3DCircle(EWEDevice& device);

        static std::unique_ptr<EWEModel> generateCircle(EWEDevice& device, uint16_t const points, float radius = 0.5f);

        template <typename T>
        static std::unique_ptr<EWEModel> createMesh(EWEDevice& device, const std::vector<T>& vertices, const std::vector<uint32_t>& indices) {
            return std::make_unique<EWEModel>(device, vertices, indices);
        }

        template <typename T>
        static std::unique_ptr<EWEModel> createMesh(EWEDevice& device, const std::vector<T>& vertices) {
            return std::make_unique<EWEModel>(device, vertices);
        }

        static std::unique_ptr<EWEModel> createSkyBox(EWEDevice& device, float scale) {
            //hopefully never have to look at this again
            
            std::vector<skyVertex> vertices = {
                {{ -1.0f, -1.0f,  1.0f}}, //0
                {{ -1.0f,  1.0f,  1.0f}}, //1
                {{  1.0f,  1.0f,  1.0f}}, //2
                {{  1.0f, -1.0f,  1.0f}}, //3
                {{ -1.0f,  1.0f, -1.0f}}, //4
                {{ -1.0f, -1.0f, -1.0f}}, //5
                {{  1.0f,  1.0f, -1.0f}}, //6
                {{  1.0f, -1.0f, -1.0f}}, //7
            };
            
            std::vector<uint32_t> indices = {
                0, 1, 2,
                2,3, 0,
                4, 1, 0,
                0,5,4,
                2, 6, 7,
                7, 3, 2,
                4, 5, 7,
                7, 6, 4,
                0, 3, 7,
                7,5, 0,
                1, 4, 2,
                2, 4, 6
            };
            
            for (int i = 0; i < vertices.size(); i++) {
                vertices[i].position *= scale;
            }
            
            //printf("vertex size ? : %d \n", vertices.size());
            return std::make_unique<EWEModel>(device, vertices, indices);
        }
        
        
        template<typename T>
        EWEModel(EWEDevice& device, const std::vector<T>& vertices, const std::vector<uint32_t>& indices) : eweDevice{ device } {
            vertexCount = static_cast<uint32_t>(vertices.size());
            assert(vertexCount >= 3 && "vertex count must be at least 3, template");
            uint32_t vertexSize = sizeof(vertices[0]);
            VertexBuffers(vertexCount, vertexSize, (void*)vertices.data());
            createIndexBuffers(indices);
            //IndexBuffers(indexCount, vertexSize, (void*)indices.data());
        }
        template<typename T>
        EWEModel(EWEDevice& device, const std::vector<T>& vertices) : eweDevice{ device } {
            vertexCount = static_cast<uint32_t>(vertices.size());
            assert(vertexCount >= 3 && "vertex count must be at least 3, template");
            uint32_t vertexSize = sizeof(vertices[0]);
            VertexBuffers(vertexCount, vertexSize, (void*)vertices.data());
        }

        void AddInstancing(uint32_t instanceCount, uint32_t instanceSize, void* data, uint8_t instanceIndex);
        void updateInstancing(uint32_t instanceCount, uint32_t instanceSize, void* data, uint8_t instanceIndex, VkCommandBuffer cmdBuf);

        //virtual ~EWEModel() {}
        

        EWEModel(const EWEModel&) = delete;
        EWEModel& operator=(const EWEModel&) = delete;

        static std::unique_ptr<EWEModel> createModelFromFile(EWEDevice& device, const std::string& filepath);
        static std::unique_ptr<EWEModel> createSimpleModelFromFile(EWEDevice& device, const std::string& filePath);
        static std::unique_ptr<EWEModel> createGrassModelFromFile(EWEDevice& device, const std::string& filePath);

        void BindAndDraw(VkCommandBuffer commandBuffer);
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        void BindAndDrawInstance(VkCommandBuffer commandBuffer, uint8_t instanceIndex);
        void BindAndDrawInstanceNoBuffer(VkCommandBuffer commandBuffer, int instanceCount);

        uint32_t getVertexCount() { return vertexCount; }
        uint32_t getIndexCount() { return indexCount; }
        

    protected:
        //void createVertexBuffers(const std::vector<Vertex>& vertices);
        //void createUIVertexBuffers(const std::vector<VertexUI>& vertices);
        //void createBoneVertexBuffers(const std::vector<boneVertex>& vertices);
        //void createBobVertexBuffers(const std::vector <bobVertex>& vertices);

        void VertexBuffers(uint32_t vertexCount, uint32_t vertexSize, void* data);

        void createGrassIndexBuffer(void* indexData, uint32_t indexCount);
        void createIndexBuffers(const std::vector<uint32_t>& indices);

        EWEDevice& eweDevice;

        std::unique_ptr<EWEBuffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<EWEBuffer> indexBuffer;
        uint32_t indexCount;
        
        bool hasInstanceBuffer = false;
        std::vector<std::unique_ptr<EWEBuffer>> instanceBuffer;
        uint32_t instanceCount;
    };







#if LEVEL_BUILDER
    class BuilderModel : public EWEModel {
    public:

        //std::vector<bobSimpleVertex> simpleVertices;
        std::vector<bobAVertex> verticesTangent;
        std::vector<bobAVertexNT> verticesNT;
        std::vector<uint32_t> indices;

        //need bobAVertex, and bobAVertexNT
        
        enum VertexType{
            vT_tangent,
            vT_NT
        };
        VertexType vType = vT_NT;

        static std::unique_ptr<EWEModel> generateNTQuad(EWEDevice& device, glm::vec2 uvScale = glm::vec2{ 1.f }) {
            std::vector<bobAVertexNT> vertices{
                bobAVertexNT{0.5f,0.0f, -0.5f,      0.f, 1.f,0.f,       1.f, 1.f},
                bobAVertexNT{-0.5f,0.0f, -0.5f,     0.f, 1.f,0.f,       0.f, 1.f},
                bobAVertexNT{-0.5f,0.0f, 0.5f,      0.f, 1.f,0.f,       0.f, 0.f},
                bobAVertexNT{0.5f,0.0f, 0.5f,       0.f ,1.f,0.f,       1.f, 0.f},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };

            return std::make_unique<BuilderModel>(device, vertices, indices);
        }
        static std::unique_ptr<EWEModel>generateTangentQuad(EWEDevice& device, glm::vec2 uvScale = glm::vec2{ 1.f }) {
            std::vector<bobAVertex> vertices{
                bobAVertex{0.5f, 0.0f, -0.5f,       0.f, 1.f, 0.f,      1.f, 1.f,       1.f, 0.f, 0.f},
                bobAVertex{-0.5f, 0.0f, -0.5f,      0.f, 1.f, 0.f,      0.f, 1.f,       1.f, 0.f, 0.f},
                bobAVertex{-0.5f, 0.0f, 0.5f,       0.f, 1.f, 0.f,      0.f, 0.f,       1.f, 0.f, 0.f},
                bobAVertex{0.5f, 0.0f, 0.5f,        0.f, 1.f, 0.f,      1.f, 0.f,       1.f, 0.f, 0.f},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };
            return std::make_unique<BuilderModel>(device, vertices, indices);
        }
        /*
        template<typename T>
        BuilderModel(EWEDevice& device, std::vector<T>& vertices, std::vector<uint32_t>& indices) : EWEModel{ device, vertices, indices } {
            if (std::is_same<T, bobSimpleVertex>::value) {
                simpleVertices = &vertices;
            }
            else if (std::is_same<T, bobAVertex>::value) {
                materiaEWErtices = &vertices;
            }
            this->indices = indices;
        }
        */
        
        BuilderModel(EWEDevice& device, std::vector<bobAVertexNT>& vertices, std::vector<uint32_t>& indices) : EWEModel{ device, vertices, indices } {
            verticesNT = vertices;
            this->indices = indices;
        }
        BuilderModel(EWEDevice& device, std::vector<bobAVertex>& vertices, std::vector<uint32_t>& indices) : EWEModel{ device, vertices, indices } {
            verticesTangent = vertices;
            this->indices = indices;
            vType = vT_tangent;
        }
        
        void addTangent() {

            verticesTangent.clear();
            verticesTangent.reserve(verticesNT.size());
            for (int i = 0; i < verticesNT.size(); i++) {
                verticesTangent.push_back({});
                memcpy(&verticesTangent[i], &verticesNT[i], FLOAT_SIZE3 * 2 + FLOAT_SIZE2);
                //verticesTangent[i].tangent = { 1.f,0.f,0.f };
                
            }
            verticesNT.clear();
            wantsToBeChanged = true;
            //VertexBuffers(verticesTangent.size(), sizeof(verticesTangent[0]), (void*)verticesTangent.data());
            vType = vT_tangent;
        }
        void removeTangent() {
            verticesNT.clear();
            verticesNT.resize(verticesTangent.size());
            for (int i = 0; i < verticesNT.size(); i++) {
                //verticesNT.push_back({});
                memcpy(&verticesNT[i], &verticesTangent[i], FLOAT_SIZE3 * 2 + FLOAT_SIZE2);
            }
            verticesTangent.clear();
            wantsToBeChanged = true;
            //VertexBuffers(verticesNT.size(), sizeof(verticesNT[0]), (void*)verticesNT.data());
            vType = vT_NT;
        }
        /*
        template<typename T>
        void replaceQuad(const std::vector<T>& vertices) {
            if (std::is_same<T, bobSimpleVertex>::value) {
                simpleVertices = (bobSimpleVertex*)vertices;
            }
            else if (std::is_same<T,bobAVertex>::value) {
                materiaEWErtices = vertices;
            }
            VertexBuffers(vertices.size(), sizeof(vertices[0]), (void*)vertices.data());
        }
        */
        void replaceQuad(const std::vector<bobAVertexNT>& vertices) {
            verticesNT = vertices;
            wantsToBeChanged = true;
        }
        void replaceQuad(const std::vector<bobAVertex>& vertices) {
            verticesTangent = vertices;
            wantsToBeChanged = true;
        }

        bool WantsChange() {
            changeTime += wantsToBeChanged + wantsToBeDeleted;
            if (changeTime >= 2) {
                if (wantsToBeDeleted > 0) {
                    printf("wants change, wantsToBeDeleted \n");
                    wantsToBeDeleted = 2;
                    return true;
                }
                changeTime = 0;
                wantsToBeChanged = false;
#ifdef _DEBUG
                if (!(((verticesNT.size() > 0) ^ (verticesTangent.size())) > 0)) {
                    //std::cout << std::format("Usage: {} [step(=0.1)] [image_size_factor(=1)]\n", std::filesystem::path{*argv}.stem().string());
                    std::cout << std::format("both vertices are 0 on replace, or both are filled - {}:{} \n", verticesNT.size(), verticesTangent.size());
                    //printf("both vertices are 0 on replace, or both are filled - %d:%d \n", verticesNT.size(), verticesTangent.size());
                }
#endif
                if (verticesNT.size() > 0) {
                    vertexBuffer.reset();
                    VertexBuffers(verticesNT.size(), sizeof(verticesNT[0]), (void*)verticesNT.data());
                    //this shouldnt return until the CPU operation is complete, BUT the gpu operation might not complete before the next draw
                }
                else if(verticesTangent.size() > 0) {
                    vertexBuffer.reset();
                    VertexBuffers(verticesTangent.size(), sizeof(verticesTangent[0]), (void*)verticesTangent.data());
                }
                return true;
            }
            return wantsToBeChanged;
        }
        void initiateDeletion() {
            printf("initiation deletion \n");
            wantsToBeDeleted = 1;
        }
        bool ReadyForDeletion() { 
            printf("ready foir deletion \n");
            return wantsToBeDeleted == 2;
        }
        
    private:
        bool wantsToBeChanged = false;
        uint8_t wantsToBeDeleted = 0;//0 for does not want delete, 1 for wants deleton, 2 for ready to be deleted
        uint8_t changeTime = 0;
        //uint8_t deletedTime = 0;
        //std::unique_ptr<EWEBuffer> vertexBufferSwap;
        //std::unique_ptr<EWEBuffer> indexBufferSwap;

    };
#endif
}  // namespace EWE
