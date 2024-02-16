#include "EWEngine/Graphics/Model/Vertex.h"

namespace EWE {


    /*
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
    */

    std::vector<VkVertexInputAttributeDescription> simpleVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(simpleVertex, position) });

        return attributeDescriptions;
    }

    std::vector<VkVertexInputBindingDescription> GrassVertex::getBindingDescriptions() { //still here because instanced
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(2);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(GrassVertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].stride = sizeof(GrassInstance);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> GrassVertex::getAttributeDescriptions() {

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GrassVertex, position) },
            { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GrassVertex, color) },
            //{ 1, 0, VK_FORMAT_R32_SFLOAT, sizeof(glm::vec3) * 3 },

            //instance
            { 2, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
            { 3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4)},
            { 4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 2},
            { 5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 3}
            //{ 6, 1, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec4) * 4} removing this, going to use world position for uvscroll calculation instead
        };
        return attributeDescriptions;
    }
    std::vector<VkVertexInputBindingDescription> TileVertex::getBindingDescriptions() { //still here because instanced
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(2);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(TileVertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].stride = sizeof(TileInstance);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> TileVertex::getAttributeDescriptions() {

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
            //vertex
            { 0, 0, VK_FORMAT_R32G32_SFLOAT, 0},

            //instance
            { 1, 1, VK_FORMAT_R32G32_SFLOAT, 0}
        };
        return attributeDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> EffectVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(EffectVertex, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(EffectVertex, uv) });

        return attributeDescriptions;
    }


    std::vector<VkVertexInputAttributeDescription> VertexColor::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexColor , position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexColor, normal) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexColor, uv) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexColor, color) });

        return attributeDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> skyVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(skyVertex, position) });

        return attributeDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> boneVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(boneVertex, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(boneVertex, normal) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(boneVertex, uv) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(boneVertex, tangent) });
        attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(boneVertex, m_BoneIDs) });
        attributeDescriptions.push_back({ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(boneVertex, m_Weights) });

        return attributeDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> boneVertexNoTangent::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(boneVertexNoTangent, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(boneVertexNoTangent, normal) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(boneVertexNoTangent, uv) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(boneVertexNoTangent, m_BoneIDs) });
        attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(boneVertexNoTangent, m_Weights) });

        return attributeDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent) });

        return attributeDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> VertexNT::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexNT, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexNT, normal) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexNT, uv) });

        return attributeDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> VertexUI::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexUI, position);

        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexUI, uv) });

        return attributeDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> VertexGrid2D::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexGrid2D, position);

        return attributeDescriptions;
    }

    void boneVertex::swapEndian() {
        Reading::swapGLMVec3Endian(position);
        Reading::swapGLMVec3Endian(normal);
        Reading::swapGLMVec2Endian(uv);
        Reading::swapGLMVec3Endian(tangent);
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            Reading::swapBasicEndian(m_BoneIDs + i, sizeof(int));
            Reading::swapBasicEndian(m_Weights + i, sizeof(int));
        }
    }

    void boneVertexNoTangent::swapEndian() {
        Reading::swapGLMVec3Endian(position);
        Reading::swapGLMVec3Endian(normal);
        Reading::swapGLMVec2Endian(uv);
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            Reading::swapBasicEndian(m_BoneIDs + i, sizeof(int));
            Reading::swapBasicEndian(m_Weights + i, sizeof(int));
        }
    }
    void Vertex::swapEndian() {
        Reading::swapGLMVec3Endian(position);
        Reading::swapGLMVec3Endian(normal);
        Reading::swapGLMVec2Endian(uv);
        Reading::swapGLMVec3Endian(tangent);
    }
    void VertexNT::swapEndian() {
        Reading::swapGLMVec3Endian(position);
        Reading::swapGLMVec3Endian(normal);
        Reading::swapGLMVec2Endian(uv);
    }
    
    /*
    template <typename V_Type>
    void MeshData<V_Type>::swapEndian() {
        for (auto& vertex : vertices) {
            vertex.swapEndian();
        }
        for (auto& index : indices) {
            Reading::swapBasicEndian(&index, sizeof(uint32_t));
        }
    }
    template <typename V_Type>
    void MeshData<V_Type>::readFromFile(std::ifstream& inFile) {
        
        uint64_t size;
        Reading::UInt64FromFile(inFile, size);
        vertices.resize(size);
        inFile.read(reinterpret_cast<char*>(&vertices[0]), size * sizeof(vertices[0]));

        Reading::UInt64FromFile(inFile, size);
        indices.resize(size);
        inFile.read(reinterpret_cast<char*>(&indices[0]), size * sizeof(uint32_t));
        
    }
    template <typename V_Type>
    void MeshData<V_Type>::readFromFileSwapEndian(std::ifstream& inFile) {
        
        uint64_t size;
        Reading::UInt64FromFileSwapEndian(inFile, size);
        vertices.resize(size);
        inFile.read(reinterpret_cast<char*>(&vertices[0]), size * sizeof(vertices[0]));

        Reading::UInt64FromFileSwapEndian(inFile, size);
        indices.resize(size);
        inFile.read(reinterpret_cast<char*>(&indices[0]), size * sizeof(uint32_t));

        swapEndian();
    }
    */
}