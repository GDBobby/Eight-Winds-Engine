#pragma once

#include "EWEngine/graphics/EWE_Buffer.h"
#include "EWEngine/graphics/EWE_Device.hpp"
#include "EWEngine/Data/TransformInclude.h"
#include "EWE_Vertex.h"

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



    class EWEModel {
    public:

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


        template <typename T>
        static std::unique_ptr<EWEModel> createMesh(EWEDevice& device, const std::vector<T>& vertices, const std::vector<uint32_t>& indices) {
            return std::make_unique<EWEModel>(device, vertices, indices);
        }

        template <typename T>
        static std::unique_ptr<EWEModel> createMesh(EWEDevice& device, const std::vector<T>& vertices) {
            return std::make_unique<EWEModel>(device, vertices);
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
