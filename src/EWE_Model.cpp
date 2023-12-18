#include "EWEngine/EWE_Model.h"

#include "EWEngine/EWE_utils.h"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtc/type_ptr.hpp>

// std
#include <cassert>
#include <unordered_map>
#include <iostream>

#ifndef ENGINE_DIR
#define ENGINE_DIR "models/"
#endif

namespace std {
    template <>
    struct hash<EWE::EWEModel::Vertex> {
        size_t operator()(EWE::EWEModel::Vertex const& vertex) const {
            size_t seed = 0;
            EWE::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
    template<>
    struct hash<EWE::EWEModel::simpleVertex> {
        size_t operator()(EWE::EWEModel::simpleVertex const& vertex) const {
            size_t seed = 0;
            EWE::hashCombine(seed, vertex.position);
            return seed;
        }
    };
    template<>
    struct hash<EWE::EWEModel::GrassVertex> {
        size_t operator()(EWE::EWEModel::GrassVertex const& vertex) const {
            size_t seed = 0;
            EWE::hashCombine(seed, vertex.position, vertex.color);
            return seed;
        }
    };
}  // namespace std

namespace EWE {


    std::unique_ptr<EWEModel> EWEModel::createModelFromFile(EWEDevice& device, const std::string& filepath) {
        Builder builder{};
        builder.loadModel(filepath);
        return std::make_unique<EWEModel>(device, builder.vertices, builder.indices);
    }
    std::unique_ptr<EWEModel> EWEModel::createSimpleModelFromFile(EWEDevice& device, const std::string& filePath) {
        SimpleBuilder builder{};
        builder.loadModel(filePath);
        return std::make_unique<EWEModel>(device, builder.vertices, builder.indices);
    }
    std::unique_ptr<EWEModel> EWEModel::createGrassModelFromFile(EWEDevice& device, const std::string& filePath) {
        GrassBuilder builder{};
        builder.loadModel(filePath);
        return std::make_unique<EWEModel>(device, builder.vertices, builder.indices);
    }

    std::unique_ptr<EWEModel> EWEModel::generateCircle(EWEDevice& device, uint16_t const points, float radius) {
        //utilizing a triangle fan
        if (points < 5) {
            std::cout << "yo wyd? making a circle with too few points : " << points << std::endl;
        }

        glm::vec3 color = { 1.f,1.f,1.f };
        glm::vec3 normal = { 0.f, -1.f, 0.f };
        std::vector<EWEModel::Vertex> vertices;
        vertices.push_back({ { 0.0f,0.0f,0.0f }, normal, { 0.5f,0.5f }, color });

        float angle = glm::two_pi<float>() / points;

        for (uint16_t i = 0; i < points; i++) {
            //gonna have to fuck with UV for a while
            // 
            //i dont have a better name
            float theSin = glm::sin(angle * i);
            float theCos = glm::cos(angle * i);
            //std::cout << "theSin:theCos ~ " << theSin << " : " << theCos << std::endl; //shit is tiling when i want it to stretch
            vertices.push_back({ {radius * theSin, 0.f, radius * theCos}, normal, {(theSin + 1.f) / 2.f, (theCos + 1.f) /2.f}, color });
        }
        std::vector<uint32_t> indices;

        for (uint16_t i = 2; i < points; i++) {
            indices.push_back(0);
            indices.push_back(i - 1);
            indices.push_back(i);
        }
        indices.push_back(0);
        indices.push_back(points - 1);
        indices.push_back(1);
        return std::make_unique<EWEModel>(device, vertices, indices);
    }

    void EWEModel::AddInstancing(uint32_t instanceCount, uint32_t instanceSize, void* data, uint8_t instanceIndex) {
        VkDeviceSize bufferSize = instanceSize * instanceCount;
        this->instanceCount = instanceCount;
        EWEBuffer stagingBuffer{
            eweDevice,
            instanceSize,
            instanceCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer(data);

        if (instanceIndex == instanceBuffer.size()) {
            instanceBuffer.emplace_back(std::make_unique<EWEBuffer>(
                eweDevice,
                instanceSize,
                instanceCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        }
        else {
            std::cout << std::format("instanceIndex and instanceBuffer getting KINDA STRNAGE, values - {}:{} \n", instanceIndex, instanceBuffer.size());
            assert(instanceIndex == instanceBuffer.size());
        }

        eweDevice.copyBuffer(stagingBuffer.getBuffer(), instanceBuffer[instanceIndex]->getBuffer(), bufferSize);
    }
    void EWEModel::updateInstancing(uint32_t instanceCount, uint32_t instanceSize, void* data, uint8_t instanceIndex, VkCommandBuffer cmdBuf) {
        assert(instanceIndex < instanceBuffer.size());
        VkDeviceSize bufferSize = instanceSize * instanceCount;
        this->instanceCount = instanceCount;
        EWEBuffer stagingBuffer{
            eweDevice,
            instanceSize,
            instanceCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer(data);

        instanceBuffer[instanceIndex] = std::make_unique<EWEBuffer>(
            eweDevice,
            instanceSize,
            instanceCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        eweDevice.copySecondaryBuffer(stagingBuffer.getBuffer(), instanceBuffer[instanceIndex]->getBuffer(), bufferSize, cmdBuf);
    }
    void EWEModel::VertexBuffers(uint32_t vertexCount, uint32_t vertexSize, void* data) {
        VkDeviceSize bufferSize = vertexSize * vertexCount;

        EWEBuffer stagingBuffer{
            eweDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer(data);

        vertexBuffer = std::make_unique<EWEBuffer>(
            eweDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        eweDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }
    void EWEModel::createGrassIndexBuffer(void* indexData, uint32_t indexCount) {
        VkDeviceSize bufferSize = indexCount * 4;
        uint32_t indexSize = 4;

        EWEBuffer stagingBuffer{
            eweDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer(indexData);

        indexBuffer = std::make_unique<EWEBuffer>(
            eweDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        eweDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }
    void EWEModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer) {
            std::cout << "no index" << std::endl;
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        EWEBuffer stagingBuffer{
            eweDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices.data());

        indexBuffer = std::make_unique<EWEBuffer>(
            eweDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        eweDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

    void EWEModel::draw(VkCommandBuffer commandBuffer) {
        if (hasIndexBuffer) {
            //printf("Drawing indexed : %d \n", indexCount);
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
            //an error here could be due to textures or other descriptors
        }
        else {
            //printf("Drawing not indexed, vertexCount : %d \n", vertexCount);
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

    void EWEModel::bind(VkCommandBuffer commandBuffer) {
        // do a second buffer, and switch to it when changign the object
        //count a couple of frames then delete the old buffer
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void EWEModel::BindAndDraw(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        }
        else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }
    void EWEModel::BindAndDrawInstance(VkCommandBuffer commandBuffer, uint8_t instanceIndex) {
        VkBuffer buffers[2] = { vertexBuffer->getBuffer(), instanceBuffer[instanceIndex]->getBuffer()};
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffers[0], offsets);
        vkCmdBindVertexBuffers(commandBuffer, 1, 1, &buffers[1], offsets);
        //vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets); //test this with grass later
        //if no index buffer, indexing wont work
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
    }
    void EWEModel::BindAndDrawInstanceNoBuffer(VkCommandBuffer commandBuffer, int instanceCount) {
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
        }
        else {
            vkCmdDraw(commandBuffer, vertexCount, instanceCount, 0, 0);
        }
    }

    std::vector<VkVertexInputAttributeDescription> EWEModel::simpleVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(simpleVertex, position) });

        return attributeDescriptions;
    }


    std::vector<VkVertexInputBindingDescription> EWEModel::GrassVertex::getBindingDescriptions() { //still here because instanced
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(2);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(GrassVertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; 
        
        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].stride = sizeof(GrassInstance);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> EWEModel::GrassVertex::getAttributeDescriptions() {

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GrassVertex, position) },
            { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GrassVertex, color) },
            //{ 1, 0, VK_FORMAT_R32_SFLOAT, sizeof(glm::vec3) * 3 },

            //instance
            { 2, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
            { 3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4)},
            { 4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 2},
            { 5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 3},
            {6, 1, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec4) * 4}
        };
        return attributeDescriptions;
    }
    std::vector<VkVertexInputBindingDescription> EWEModel::LeafVertex::getBindingDescriptions() { //still here because instanced
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(LeafVertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        /*
        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].stride = sizeof(LeafInstance);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        */
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> EWEModel::LeafVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(LeafVertex, position) },
            { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(LeafVertex, normal) },
            { 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(LeafVertex, uv) },

            //instance
            /*
            { 3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
            { 4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4)},
            { 5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 2},
            { 6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 3},
            */
        };

        return attributeDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> EWEModel::EffectVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(EffectVertex, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(EffectVertex, uv) });

        return attributeDescriptions;
    }


    std::vector<VkVertexInputAttributeDescription> EWEModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });

        return attributeDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> EWEModel::skyVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(skyVertex, position) });

        return attributeDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> EWEModel::boneVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(boneVertex, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(boneVertex, normal) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(boneVertex, uv) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(boneVertex, tangent) });
        attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(boneVertex, m_BoneIDs) });
        attributeDescriptions.push_back({ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(boneVertex, m_Weights) });

        return attributeDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> EWEModel::glmVertexNoTangent::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(glmVertexNoTangent, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(glmVertexNoTangent, normal) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(glmVertexNoTangent, uv) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(glmVertexNoTangent, m_BoneIDs) });
        attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(glmVertexNoTangent, m_Weights) });

        return attributeDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> EWEModel::AVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(AVertex, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(AVertex, normal) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(AVertex, uv) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(AVertex, tangent) });

        return attributeDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> EWEModel::AVertexNT::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(AVertexNT, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(AVertexNT, normal) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(AVertexNT, uv) });

        return attributeDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> bobVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(bobVertex, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(bobVertex, normal) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(bobVertex, uv) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(bobVertex, tangent) });
        attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(bobVertex, m_BoneIDs) });
        attributeDescriptions.push_back({ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(bobVertex, m_Weights) });

        return attributeDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> EWEModel::VertexUI::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexUI, position);

        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexUI, uv) });

        return attributeDescriptions;
    }

    void EWEModel::Builder::loadModel(const std::string& filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string enginePath = ENGINE_DIR + filepath;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, enginePath.c_str())) {
            printf("warning : %s - err :%s \n", warn.c_str(), err.c_str());
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        //printf("vertex count after loading model from file : %d \n", vertices.size());
        //printf("index count after loading model froom file : %d \n", indices.size());
    }
    void EWEModel::GrassBuilder::loadModel(const std::string& filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string enginePath = ENGINE_DIR + filepath;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, enginePath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<GrassVertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                GrassVertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    
                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                    
                    /*
                    if (index.texcoord_index >= 0) {
                        vertex.uv = {
                            attrib.texcoords[2 * index.texcoord_index + 0]
                        };
                    }
                    */
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        //printf("vertex count after loading model from file : %d \n", vertices.size());
        //printf("index count after loading model froom file : %d \n", indices.size());
    }

    void EWEModel::SimpleBuilder::loadModel(const std::string& filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string enginePath = ENGINE_DIR + filepath;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, enginePath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<simpleVertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                simpleVertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        //printf("vertex count after loading simple model from file : %d \n", vertices.size());
        //printf("index count after loading simple model from file : %d \n", indices.size());
    }
    /*
    std::unique_ptr<EWEModel> EWEModel::LoadGrassField(EWEDevice& device) {
        printf("beginning load grass field \n");
        std::fstream grassFile{ "..//models//grassField.gs" };
        if (!grassFile.is_open()) {
            printf("failed to open grass field file \n");
            return nullptr;
        }
        uint32_t vertexCount = 2500 * 13;
        uint32_t vertexSize = 12;

        std::vector<bobvec3> vertexBuffer(vertexCount);
        grassFile.seekg(0);
        grassFile.read(reinterpret_cast<char*>(&vertexBuffer[0]), vertexCount * vertexSize);

        uint32_t indexCount = 2500 * 42;
        std::vector<uint32_t> indexBuffer(indexCount);

        grassFile.seekg(vertexCount * vertexSize);
        grassFile.read(reinterpret_cast<char*>(&indexBuffer[0]), indexCount * 4);
        grassFile.close();
        printf("successfuly read file \n");
        //printf("size of vertices and indices in grass field  - %d : %d \n", vertices.size(), indices.size());
        return std::make_unique<EWEModel>(device, vertexCount, 12, vertexBuffer.data(), indexBuffer.data(), indexCount);
    }
    */
}  // namespace EWE
