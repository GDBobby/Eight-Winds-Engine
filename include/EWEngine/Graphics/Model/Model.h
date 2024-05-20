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
    constexpr std::size_t FLOAT_SIZE3 = sizeof(float) * 3;
    constexpr std::size_t FLOAT_SIZE2 = sizeof(float) * 2;

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

            void LoadModel(const std::string& filepath);
        };
        struct SimpleBuilder {
            std::vector<SimpleVertex> vertices{};
            std::vector<uint32_t> indices{};

            void LoadModel(const std::string& filepath);
        };
        struct GrassBuilder {
            std::vector<GrassVertex> vertices{};
            std::vector<uint32_t> indices{};

            void LoadModel(const std::string& filepath);
        };

        template <typename T>
        static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions() {
            std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
            bindingDescriptions[0].binding = 0;
            //printf("size of BVNT:T - %d:%d \n", sizeof(boneVertexNoTangent),sizeof(T));
            bindingDescriptions[0].stride = sizeof(T);
            bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescriptions;
        }


        static std::unique_ptr<EWEModel> CreateMesh(void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex, std::vector<uint32_t>const& indices);
        static std::unique_ptr<EWEModel> CreateMesh(void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex);
        static std::unique_ptr<EWEModel> CreateMesh(VkCommandBuffer cmdBuf, void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex, std::vector<uint32_t>const& indices);
        static std::unique_ptr<EWEModel> CreateMesh(VkCommandBuffer cmdBuf, void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex);

        EWEModel(void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex, std::vector<uint32_t> const& indices);
        EWEModel(void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex);
        EWEModel(VkCommandBuffer cmdBuf, void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex, std::vector<uint32_t> const& indices);
        EWEModel(VkCommandBuffer cmdBuf, void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex);


        void AddInstancing(uint32_t instanceCount, uint32_t instanceSize, void* data);


        EWEModel(const EWEModel&) = delete;
        EWEModel& operator=(const EWEModel&) = delete;

        static std::unique_ptr<EWEModel> CreateModelFromFile(const std::string& filepath);
        static std::unique_ptr<EWEModel> CreateSimpleModelFromFile(const std::string& filePath);
        static std::unique_ptr<EWEModel> CreateGrassModelFromFile(const std::string& filePath);

        void BindAndDraw(VkCommandBuffer commandBuffer);
        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);
        void BindAndDrawInstance(VkCommandBuffer commandBuffer);
        void BindAndDrawInstanceNoIndex(VkCommandBuffer commandBuffer);
        void BindAndDrawInstanceNoBuffer(VkCommandBuffer commandBuffer, int instanceCount);

        uint32_t GetVertexCount() { return vertexCount; }
        uint32_t GetIndexCount() { return indexCount; }

        //delete needs to be called on this at destruction, or put it into a smart pointer
        //static EWEBuffer* CreateIndexBuffer(std::vector<uint32_t> const& indices);
        //static EWEBuffer* CreateIndexBuffer(VkCommandBuffer cmdBuf, std::vector<uint32_t> const& indices);

    protected:
        //void createVertexBuffers(const std::vector<Vertex>& vertices);
        //void createUIVertexBuffers(const std::vector<VertexUI>& vertices);
        //void createBoneVertexBuffers(const std::vector<boneVertex>& vertices);
        //void createBobVertexBuffers(const std::vector <bobVertex>& vertices);

        void VertexBuffers(uint32_t vertexCount, uint32_t vertexSize, void const* data);
        void VertexBuffers(VkCommandBuffer cmdBuf, uint32_t vertexCount, uint32_t vertexSize, void const* data);

        void CreateIndexBuffer(void* indexData, uint32_t indexCount);
        void CreateIndexBuffers(const std::vector<uint32_t>& indices);
        void CreateIndexBuffer(VkCommandBuffer cmdBuf, void* indexData, uint32_t indexCount);
        void CreateIndexBuffers(VkCommandBuffer cmdBuf, const std::vector<uint32_t>& indices);

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