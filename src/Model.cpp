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
    EWEModel::EWEModel(void const* verticesData, const std::size_t vertexCount, const std::size_t sizeOfVertex, std::vector<uint32_t> const& indices, Queue::Enum queue) {
        assert(vertexCount >= 3 && "vertex count must be at least 3");
        VertexBuffers(vertexCount, sizeOfVertex, verticesData, queue);
        CreateIndexBuffers(indices, queue);
    }
    EWEModel::EWEModel(void const* verticesData, const std::size_t vertexCount, const std::size_t sizeOfVertex, Queue::Enum queue) {
        assert(vertexCount >= 3 && "vertex count must be at least 3");
        VertexBuffers(vertexCount, sizeOfVertex, verticesData, queue);
    }


    EWEModel* EWEModel::CreateMesh(void const* verticesData, const std::size_t vertexCount, const std::size_t sizeOfVertex, std::vector<uint32_t>const& indices, Queue::Enum queue) {
        return Construct<EWEModel>({ verticesData, vertexCount, sizeOfVertex, indices, queue });
    }
    EWEModel* EWEModel::CreateMesh(void const* verticesData, const std::size_t vertexCount, const std::size_t sizeOfVertex, Queue::Enum queue) {
        return Construct<EWEModel>({ verticesData, vertexCount, sizeOfVertex, queue });
    }

    EWEModel* EWEModel::CreateModelFromFile(const std::string& filepath, Queue::Enum queue) {
        Builder builder{};
        builder.LoadModel(filepath);
        return Construct<EWEModel>({ builder.vertices.data(), builder.vertices.size(), sizeof(builder.vertices[0]), builder.indices, queue });
    }
    EWEModel* EWEModel::CreateSimpleModelFromFile(const std::string& filePath, Queue::Enum queue) {
        SimpleBuilder builder{};
        builder.LoadModel(filePath);
        return Construct<EWEModel>({ (builder.vertices.data(), builder.vertices.size(), sizeof(builder.vertices[0]), builder.indices, queue });
    }
    EWEModel* EWEModel::CreateGrassModelFromFile(const std::string& filePath, Queue::Enum queue) {
        GrassBuilder builder{};
        builder.LoadModel(filePath);
        return Construct<EWEModel>({ builder.vertices.data(), builder.vertices.size(), sizeof(builder.vertices[0]), builder.indices, queue });
    }
    

    inline void CopyModelBuffer(StagingBuffer* stagingBuffer, VkBuffer dstBuffer, const VkDeviceSize bufferSize, const Queue::Enum queue) {
        SyncHub* syncHub = SyncHub::GetSyncHubInstance();
        VkCommandBuffer cmdBuf = syncHub->BeginSingleTimeCommand(queue);
        EWEDevice::GetEWEDevice()->CopyBuffer(cmdBuf, stagingBuffer->buffer, dstBuffer, bufferSize);

        if (queue == Queue::graphics) {
            syncHub->EndSingleTimeCommandGraphics(cmdBuf);
#if USING_VMA
            stagingBuffer->Free(EWEDevice::GetAllocator());
#else
            stagingBuffer->Free(EWEDevice::GetVkDevice());
#endif
            Deconstruct(stagingBuffer);
        }
        else if (queue == Queue::transfer) {
            //transitioning from transfer to compute not supported currently
            CommandWithCallback cb{};
            cb.cmdBuf = cmdBuf;
            cb.callback = [sb = stagingBuffer, 
#if USING_VMA
            memMgr = EWEDevice::GetAllocator()
#else
            memMgr = EWEDevice::GetVkDevice()
#endif
            ] {
                sb->Free(memMgr); Deconstruct(sb); 
            };
            syncHub->EndSingleTimeCommandTransfer(cb);
        }
    }

    void EWEModel::AddInstancing(const uint32_t instanceCount, const uint32_t instanceSize, void const* data, Queue::Enum queue) {
        VkDeviceSize bufferSize = instanceSize * instanceCount;
        this->instanceCount = instanceCount;

        uint64_t alignmentSize = EWEBuffer::GetAlignment(instanceSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT) * instanceCount;

#if USING_VMA
        StagingBuffer* stagingBuffer = Construct<StagingBuffer>({ alignmentSize, EWEDevice::GetAllocator(), data });
#else
        StagingBuffer* stagingBuffer = Construct<StagingBuffer>({ alignmentSize, EWEDevice::GetEWEDevice()->GetPhysicalDevice(), EWEDevice::GetVkDevice(), data });
#endif

        instanceBuffer = Construct<EWEBuffer>({
            instanceSize,
            instanceCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT 
        });
        
        CopyModelBuffer(stagingBuffer, instanceBuffer->GetBuffer(), bufferSize, queue);
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

    void EWEModel::VertexBuffers(uint32_t vertexCount, uint32_t vertexSize, void const* data, Queue::Enum queue){
        VkDeviceSize bufferSize = vertexSize * vertexCount;
        this->vertexCount = vertexCount;

#if USING_VMA
        StagingBuffer* stagingBuffer = Construct<StagingBuffer>({ bufferSize, EWEDevice::GetAllocator(), data });
#else
        StagingBuffer* stagingBuffer = Construct<StagingBuffer>({ bufferSize, EWEDevice::GetEWEDevice()->GetPhysicalDevice(), EWEDevice::GetVkDevice(), data });
#endif
#if DEBUGGING_MEMORY_WITH_VMA
        vertexBuffer = new EWEBuffer(
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
#else
        vertexBuffer = new EWEBuffer(
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
#endif

        CopyModelBuffer(stagingBuffer, vertexBuffer->GetBuffer(), bufferSize, queue);
    }

    void EWEModel::CreateIndexBuffer(const void* indexData, uint32_t indexCount, Queue::Enum queue){
        const uint32_t indexSize = sizeof(uint32_t);
        this->indexCount = indexCount;
        hasIndexBuffer = true;

        VkDeviceSize bufferSize = indexSize * indexCount;
#if USING_VMA
        StagingBuffer* stagingBuffer = Construct<StagingBuffer>({ bufferSize, EWEDevice::GetAllocator(), indexData });
#else
        StagingBuffer* stagingBuffer = Construct<StagingBuffer>({ bufferSize, EWEDevice::GetEWEDevice()->GetPhysicalDevice(), EWEDevice::GetVkDevice(), indexData });
#endif
#if DEBUGGING_MEMORY_WITH_VMA
        indexBuffer = Construct<EWEBuffer>({
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        });
#else
        indexBuffer = Construct<EWEBuffer>({
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        });
#endif
        CopyModelBuffer(stagingBuffer, indexBuffer->GetBuffer(), bufferSize, queue);
    }

    void EWEModel::CreateIndexBuffers(std::vector<uint32_t> const& indices, Queue::Enum queue){
        indexCount = static_cast<uint32_t>(indices.size());
        CreateIndexBuffer(static_cast<const void*>(indices.data()), indexCount, queue);
    }


    void EWEModel::Draw(VkCommandBuffer commandBuffer) {
#if EWE_DEBUG
        assert(hasIndexBuffer);
#endif
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }
    void EWEModel::DrawNoIndex(VkCommandBuffer commandBuffer) {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }

    void EWEModel::Bind(VkCommandBuffer commandBuffer) {
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer->GetBufferAddress(), offsets);
#if EWE_DEBUG
        assert(hasIndexBuffer);
#endif
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    void EWEModel::BindNoIndex(VkCommandBuffer commandBuffer) {
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer->GetBufferAddress(), offsets);
    }

    void EWEModel::BindAndDraw(VkCommandBuffer commandBuffer) {
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer->GetBufferAddress(), offsets);

#if EWE_DEBUG
        assert(hasIndexBuffer);
#endif
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        
    }
    void EWEModel::BindAndDrawNoIndex(VkCommandBuffer commandBuffer) {
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer->GetBufferAddress(), offsets);
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }

    void EWEModel::BindAndDrawInstance(VkCommandBuffer cmdBuf, uint32_t instanceCount) {
        VkBuffer buffers[2] = { vertexBuffer->GetBuffer(), instanceBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmdBuf, 0, 2, buffers, offsets);
        vkCmdBindIndexBuffer(cmdBuf, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmdBuf, indexCount, instanceCount, 0, 0, 0);
    }
    void EWEModel::BindAndDrawInstance(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[2] = { vertexBuffer->GetBuffer(), instanceBuffer->GetBuffer()};
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
    }
    void EWEModel::BindAndDrawInstanceNoIndex(VkCommandBuffer cmdBuf) {
        VkBuffer buffers[2] = { vertexBuffer->GetBuffer(), instanceBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmdBuf, 0, 2, buffers, offsets);
        //vkCmdBindIndexBuffer(cmdBuf, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDraw(cmdBuf, vertexCount, instanceCount, 0, 0);
    }


    void EWEModel::BindAndDrawInstanceNoBuffer(VkCommandBuffer commandBuffer, int instanceCount) {
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer->GetBufferAddress(), offsets);

#if EWE_DEBUG
        assert(hasIndexBuffer);
#endif
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
    }
    void EWEModel::BindAndDrawInstanceNoBufferNoIndex(VkCommandBuffer commandBuffer, int instanceCount) {
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer->GetBufferAddress(), offsets);

        vkCmdDraw(commandBuffer, vertexCount, instanceCount, 0, 0);
        
    }

    void EWEModel::Builder::LoadModel(const std::string& filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string enginePath = ENGINE_DIR + filepath;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, enginePath.c_str())) {
            printf("warning : %s - err :%s \n", warn.c_str(), err.c_str());
            std::string errString = warn + err;
            assert(false && errString.c_str());
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
                if (!uniqueVertices.contains(vertex)) {
                    uniqueVertices.try_emplace(vertex, static_cast<uint32_t>(vertices.size()));
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

                if (!uniqueVertices.contains(vertex)) {
                    uniqueVertices.try_emplace(vertex, static_cast<uint32_t>(vertices.size()));
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices.at(vertex));
            }
        }
        //printf("vertex count after loading simple model from file : %d \n", vertices.size());
        //printf("index count after loading simple model from file : %d \n", indices.size());
    }
#if DEBUG_NAMING
    void EWEModel::SetDebugNames(std::string const& name){
        std::string comboName{"vertex:"};
        comboName += name;
        DebugNaming::SetObjectName(EWEDevice::GetVkDevice(), vertexBuffer->GetBuffer(), VK_OBJECT_TYPE_BUFFER, comboName.c_str());
        if(hasIndexBuffer){
            comboName = "index:";
            comboName += name;
            DebugNaming::SetObjectName(EWEDevice::GetVkDevice(), indexBuffer->GetBuffer(), VK_OBJECT_TYPE_BUFFER, comboName.c_str());
        }
        if(hasInstanceBuffer){
           comboName = "instance:";
           comboName += name;   
           DebugNaming::SetObjectName(EWEDevice::GetVkDevice(), instanceBuffer->GetBuffer(), VK_OBJECT_TYPE_BUFFER, comboName.c_str());
        }
    }
#endif


    /*
    EWEModel* EWEModel::LoadGrassField() {
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
