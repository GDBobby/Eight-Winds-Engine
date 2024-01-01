#include "EWEngine/graphics/model/EWE_Model.h"

#include "EWEngine/EWE_Utils.h"

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
    struct hash<EWE::Vertex> {
        size_t operator()(EWE::Vertex const& vertex) const {
            size_t seed = 0;
            EWE::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
    template<>
    struct hash<EWE::simpleVertex> {
        size_t operator()(EWE::simpleVertex const& vertex) const {
            size_t seed = 0;
            EWE::hashCombine(seed, vertex.position);
            return seed;
        }
    };
    template<>
    struct hash<EWE::GrassVertex> {
        size_t operator()(EWE::GrassVertex const& vertex) const {
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

    void EWEModel::AddInstancing(uint32_t instanceCount, uint32_t instanceSize, void* data) {
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


        instanceBuffer = std::make_unique<EWEBuffer>(
            eweDevice,
            instanceSize,
            instanceCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        

        eweDevice.copyBuffer(stagingBuffer.getBuffer(), instanceBuffer->getBuffer(), bufferSize);
    }
    /*
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
    */
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
    void EWEModel::BindAndDrawInstance(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[2] = { vertexBuffer->getBuffer(), instanceBuffer->getBuffer()};
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
