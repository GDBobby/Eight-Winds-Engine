#pragma once

#include "EWEngine/Graphics/Device_Buffer.h"
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Data/TransformInclude.h"
#include "Vertex.h"

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
            std::vector<VertexNT> vertices{};
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


        static std::unique_ptr<EWEModel> createMesh(void const* verticesData, size_t vertexCount, size_t sizeOfVertex, std::vector<uint32_t>const& indices) {
            return std::make_unique<EWEModel>(verticesData, vertexCount, sizeOfVertex, indices);
        }

       static std::unique_ptr<EWEModel> createMesh(void const* verticesData, size_t vertexCount, std::size_t sizeOfVertex) {
            return std::make_unique<EWEModel>(verticesData, vertexCount, sizeOfVertex);
        }

        EWEModel(void const* verticesData, size_t vertexCount, size_t sizeOfVertex, std::vector<uint32_t> const& indices) {
            assert(vertexCount >= 3 && "vertex count must be at least 3");
            VertexBuffers(vertexCount, sizeOfVertex, verticesData);
            createIndexBuffers(indices);
        }

        EWEModel(void const* verticesData, size_t vertexCount, size_t sizeOfVertex) {
            assert(vertexCount >= 3 && "vertex count must be at least 3");
            VertexBuffers(vertexCount, sizeOfVertex, verticesData);
        }


        void AddInstancing(uint32_t instanceCount, uint32_t instanceSize, void* data);


        EWEModel(const EWEModel&) = delete;
        EWEModel& operator=(const EWEModel&) = delete;

        static std::unique_ptr<EWEModel> createModelFromFile(const std::string& filepath);
        static std::unique_ptr<EWEModel> createSimpleModelFromFile(const std::string& filePath);
        static std::unique_ptr<EWEModel> createGrassModelFromFile(const std::string& filePath);

        void BindAndDraw(VkCommandBuffer commandBuffer);
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        void BindAndDrawInstance(VkCommandBuffer commandBuffer);
        void BindAndDrawInstanceNoIndex(VkCommandBuffer commandBuffer);
        void BindAndDrawInstanceNoBuffer(VkCommandBuffer commandBuffer, int instanceCount);

        uint32_t getVertexCount() { return vertexCount; }
        uint32_t getIndexCount() { return indexCount; }

        //delete needs to be called on this at destruction, or put it into a smart pointer
        static EWEBuffer* createIndexBuffer(std::vector<uint32_t> const& indices);

    protected:
        //void createVertexBuffers(const std::vector<Vertex>& vertices);
        //void createUIVertexBuffers(const std::vector<VertexUI>& vertices);
        //void createBoneVertexBuffers(const std::vector<boneVertex>& vertices);
        //void createBobVertexBuffers(const std::vector <bobVertex>& vertices);

        void VertexBuffers(uint32_t vertexCount, uint32_t vertexSize, void const* data);

        void createGrassIndexBuffer(void* indexData, uint32_t indexCount);
        void createIndexBuffers(const std::vector<uint32_t>& indices);

        std::unique_ptr<EWEBuffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<EWEBuffer> indexBuffer;
        uint32_t indexCount;

        bool hasInstanceBuffer = false;
        std::unique_ptr<EWEBuffer> instanceBuffer;
        uint32_t instanceCount;
    };
} //namespace EWE