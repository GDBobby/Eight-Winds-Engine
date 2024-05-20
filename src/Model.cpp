#include "EWEngine/Graphics/Model/Model.h"

#include "EWEngine/Data/EWE_Utils.h"

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


//these are used for loading obj files and automatically indexing the vertices
template <>
struct std::hash<EWE::Vertex> {
    size_t operator()(EWE::Vertex const& vertex) const {
        size_t seed = 0;
        EWE::HashCombine(seed, vertex.position, vertex.normal, vertex.uv, vertex.tangent);
        return seed;
    }
};
template <>
struct std::hash<EWE::VertexNT> {
    size_t operator()(EWE::VertexNT const& vertex) const {
        size_t seed = 0;
        EWE::HashCombine(seed, vertex.position, vertex.normal, vertex.uv);
        return seed;
    }
};
template<>
struct std::hash<EWE::SimpleVertex> {
    size_t operator()(EWE::SimpleVertex const& vertex) const {
        size_t seed = 0;
        EWE::HashCombine(seed, vertex.position);
        return seed;
    }
};
template<>
struct std::hash<EWE::GrassVertex> {
    size_t operator()(EWE::GrassVertex const& vertex) const {
        size_t seed = 0;
        EWE::HashCombine(seed, vertex.position, vertex.color);
        return seed;
    }
};

namespace EWE {
    EWEModel::EWEModel(void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex, std::vector<uint32_t> const& indices) {
        assert(vertexCount >= 3 && "vertex count must be at least 3");
        VertexBuffers(vertexCount, sizeOfVertex, verticesData);
        CreateIndexBuffers(indices);
    }
    EWEModel::EWEModel(void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex) {
        assert(vertexCount >= 3 && "vertex count must be at least 3");
        VertexBuffers(vertexCount, sizeOfVertex, verticesData);
    }
    std::unique_ptr<EWEModel> EWEModel::CreateMesh(void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex, std::vector<uint32_t>const& indices) {
        return std::make_unique<EWEModel>(verticesData, vertexCount, sizeOfVertex, indices);
    }
    std::unique_ptr<EWEModel> EWEModel::CreateMesh(void const* verticesData, std::size_t vertexCount, std::size_t sizeOfVertex) {
        return std::make_unique<EWEModel>(verticesData, vertexCount, sizeOfVertex);
    }

    std::unique_ptr<EWEModel> EWEModel::CreateModelFromFile(const std::string& filepath) {
        Builder builder{};
        builder.LoadModel(filepath);
        return std::make_unique<EWEModel>(builder.vertices.data(), builder.vertices.size(), sizeof(builder.vertices[0]), builder.indices);
    }
    std::unique_ptr<EWEModel> EWEModel::CreateSimpleModelFromFile(const std::string& filePath) {
        SimpleBuilder builder{};
        builder.LoadModel(filePath);
        return std::make_unique<EWEModel>(builder.vertices.data(), builder.vertices.size(), sizeof(builder.vertices[0]), builder.indices);
    }
    std::unique_ptr<EWEModel> EWEModel::CreateGrassModelFromFile(const std::string& filePath) {
        GrassBuilder builder{};
        builder.LoadModel(filePath);
        return std::make_unique<EWEModel>(builder.vertices.data(), builder.vertices.size(), sizeof(builder.vertices[0]), builder.indices);
    }

    void EWEModel::AddInstancing(uint32_t instanceCount, uint32_t instanceSize, void* data) {
        VkDeviceSize bufferSize = instanceSize * instanceCount;
        this->instanceCount = instanceCount;
        EWEBuffer stagingBuffer{
            instanceSize,
            instanceCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer(data);


        instanceBuffer = std::make_unique<EWEBuffer>(
            instanceSize,
            instanceCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        

        SyncHub* syncHub = SyncHub::GetSyncHubInstance();
        VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommandTransfer();
        EWEDevice::GetEWEDevice()->CopyBuffer(cmdBuf, stagingBuffer.GetBuffer(), instanceBuffer->GetBuffer(), bufferSize);

        BufferQueueTransitionData transitionData{instanceBuffer->GetBuffer(), EWEDevice::GetEWEDevice()->GetGraphicsIndex()};
        syncHub->EndSingleTimeCommandTransfer(cmdBuf, transitionData);

        //this staging buffer needs to be destroyed
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
    void EWEModel::VertexBuffers(uint32_t vertexCount, uint32_t vertexSize, void const* data) {
        SyncHub* syncHub = SyncHub::GetSyncHubInstance();
        VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommandTransfer();
        VertexBuffers(cmdBuf, vertexCount, vertexSize, data);
        BufferQueueTransitionData transitionData{vertexBuffer->GetBuffer(), EWEDevice::GetEWEDevice()->GetGraphicsIndex()};
        syncHub->EndSingleTimeCommandTransfer(cmdBuf, transitionData);
    }
    void EWEModel::VertexBuffers(VkCommandBuffer cmdBuf, uint32_t vertexCount, uint32_t vertexSize, void const* data){
        VkDeviceSize bufferSize = vertexSize * vertexCount;

        EWEBuffer stagingBuffer{
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer(data);

        vertexBuffer = std::make_unique<EWEBuffer>(
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        EWEDevice::GetEWEDevice()->CopyBuffer(cmdBuf, stagingBuffer.GetBuffer(), vertexBuffer->GetBuffer(), bufferSize);
    }

    void EWEModel::CreateIndexBuffer(void* indexData, uint32_t indexCount) {
        SyncHub* syncHub = SyncHub::GetSyncHubInstance();
        VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommandTransfer();
        CreateIndexBuffer(cmdBuf, indexData, indexCount);
        BufferQueueTransitionData transitionData{indexBuffer->GetBuffer(), EWEDevice::GetEWEDevice()->GetGraphicsIndex()};
        syncHub->EndSingleTimeCommandTransfer(cmdBuf, transitionData);
    }
    void EWEModel::CreateIndexBuffer(VkCommandBuffer cmdBuf, void* indexData, uint32_t indexCount){
        const uint32_t indexSize = sizeof(uint32_t);
        const VkDeviceSize bufferSize = indexCount * indexSize;

        EWEBuffer stagingBuffer{
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer(indexData);

        indexBuffer = std::make_unique<EWEBuffer>(
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        EWEDevice::GetEWEDevice()->CopyBuffer(cmdBuf, stagingBuffer.GetBuffer(), indexBuffer->GetBuffer(), bufferSize);
    }

    void EWEModel::CreateIndexBuffers(std::vector<uint32_t> const& indices) {

        SyncHub* syncHub = SyncHub::GetSyncHubInstance();
        VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommandTransfer();
        CreateIndexBuffers(cmdBuf, indices);
        BufferQueueTransitionData transitionData{indexBuffer->GetBuffer(), EWEDevice::GetEWEDevice()->GetGraphicsIndex()};
        syncHub->EndSingleTimeCommandTransfer(cmdBuf, transitionData);
    }
    void EWEModel::CreateIndexBuffers(VkCommandBuffer cmdBuf, std::vector<uint32_t> const& indices){
                indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer) {
            std::cout << "no index" << std::endl;
            return;
        }

        VkDeviceSize bufferSize = sizeof(uint32_t) * indexCount;
        uint32_t indexSize = sizeof(uint32_t);

        EWEBuffer stagingBuffer{
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer((void*)indices.data());

        indexBuffer = std::make_unique<EWEBuffer>(
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        EWEDevice::GetEWEDevice()->CopyBuffer(cmdBuf, stagingBuffer.GetBuffer(), indexBuffer->GetBuffer(), bufferSize);
    }


    void EWEModel::Draw(VkCommandBuffer commandBuffer) {
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

    void EWEModel::Bind(VkCommandBuffer commandBuffer) {
        // do a second buffer, and switch to it when changign the object
        //count a couple of frames then remove the old buffer
        VkBuffer buffers[] = { vertexBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void EWEModel::BindAndDraw(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = { vertexBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        }
        else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }
    void EWEModel::BindAndDrawInstance(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[2] = { vertexBuffer->GetBuffer(), instanceBuffer->GetBuffer()};
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets);
        //vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets); //test this with grass later
        //if no index buffer, indexing wont work
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
    }
    void EWEModel::BindAndDrawInstanceNoIndex(VkCommandBuffer cmdBuf) {
        VkBuffer buffers[2] = { vertexBuffer->GetBuffer(), instanceBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmdBuf, 0, 2, buffers, offsets);
        //vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets); //test this with grass later
        //if no index buffer, indexing wont work
        //vkCmdBindIndexBuffer(cmdBuf, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDraw(cmdBuf, vertexCount, instanceCount, 0, 0);
    }


    void EWEModel::BindAndDrawInstanceNoBuffer(VkCommandBuffer commandBuffer, int instanceCount) {
        VkBuffer buffers[] = { vertexBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
        }
        else {
            vkCmdDraw(commandBuffer, vertexCount, instanceCount, 0, 0);
        }
    }

    void EWEModel::Builder::LoadModel(const std::string& filepath) {
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

        std::unordered_map<VertexNT, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                VertexNT vertex;

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };
                    /*
                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                    */
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
                else {
                    vertex.uv = {
                        0.f,
                        0.f
                    };
                }

                if (!uniqueVertices.contains(vertex)) {
                    uniqueVertices.try_emplace(vertex, static_cast<uint32_t>(vertices.size()));
                    //uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices.at(vertex));
            }
        }
        //printf("vertex count after loading model from file : %d \n", vertices.size());
        //printf("index count after loading model froom file : %d \n", indices.size());
    }
    void EWEModel::GrassBuilder::LoadModel(const std::string& filepath) {
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

    void EWEModel::SimpleBuilder::LoadModel(const std::string& filepath) {
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

        std::unordered_map<SimpleVertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                SimpleVertex vertex{};

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
    std::unique_ptr<EWEModel> EWEModel::LoadGrassField() {
        printf("beginning load grass field \n");
        std::ifstream grassFile{ "..//models//grassField.gs" };
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
