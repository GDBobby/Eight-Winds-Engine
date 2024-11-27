#include "EWEngine/Graphics/Model/Basic_Model.h"


namespace EWE {
    namespace Basic_Model {
#if CALL_TRACING
        EWEModel* Circle(Queue::Enum queue, uint16_t const points, float radius, std::source_location srcLoc) {
            //utilizing a triangle fan
#if EWE_DEBUG
            if (points < 5) {
                std::cout << "yo wyd? making a circle with too few points : " << points << std::endl;
            }
#endif

            glm::vec3 color = { 1.f,1.f,1.f };
            glm::vec3 normal = { 0.f, -1.f, 0.f };
            std::vector<Vertex> vertices;
            vertices.push_back({ { 0.0f,0.0f,0.0f }, normal, { 0.5f,0.5f }, color });

            float angle = glm::two_pi<float>() / points;

            for (uint16_t i = 0; i < points; i++) {
                //gonna have to fuck with UV for a while
                // 
                //i dont have a better name
                float theSin = glm::sin(angle * i);
                float theCos = glm::cos(angle * i);
                //std::cout << "theSin:theCos ~ " << theSin << " : " << theCos << std::endl; //shit is tiling when i want it to stretch
                vertices.push_back({ {radius * theSin, 0.f, radius * theCos}, normal, {(theSin + 1.f) / 2.f, (theCos + 1.f) / 2.f}, color });
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
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue }, srcLoc);
        }

        EWEModel* Quad(Queue::Enum queue, glm::vec2 uvScale, std::source_location srcLoc) {
            std::vector<Vertex> vertices{
                {{0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {uvScale.x,uvScale.y}, {1.f, 0.f, 0.f}},
                {{-0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {0.0f,uvScale.y}, {1.f, 0.f, 0.f}},
                {{-0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {0.0f,0.f}, {1.f, 0.f, 0.f}},
                {{0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {uvScale.x,0.f}, {1.f, 0.f, 0.f}},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2,3,0 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue }, srcLoc);
        }
        EWEModel* QuadPNU(Queue::Enum queue, glm::vec2 uvScale, std::source_location srcLoc) {
            std::vector<VertexNT> vertices{
                {{0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {uvScale.x,uvScale.y}},
                {{-0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {0.0f,uvScale.y}},
                {{-0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {0.0f,0.f}},
                {{0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {uvScale.x,0.f}},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2,3,0 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue }, srcLoc);
        }
        EWEModel* Simple3DQuad(Queue::Enum queue, glm::vec2 uvScale, std::source_location srcLoc) {
            std::vector<EffectVertex> vertices{
                {{0.5f,0.0f, -0.5f}, {uvScale.x,uvScale.y}},
                {{-0.5f,0.0f, -0.5f}, {0.0f,uvScale.y}},
                {{-0.5f,0.0f, 0.5f}, {0.0f,0.f}},
                {{0.5f,0.0f, 0.5f}, {uvScale.x,0.f}},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue }, srcLoc);
        }

        EWEModel* TileQuad3D(Queue::Enum queue, glm::vec2 uvScale, std::source_location srcLoc) {
            std::vector<TileVertex> vertices{
                {{uvScale.x,uvScale.y}},
                {{0.0f,uvScale.y}},
                {{0.0f,0.f}},
                {{0.0f,0.f}},
                {{uvScale.x,0.f}},
                {{uvScale.x,uvScale.y}},
            };
            std::vector<uint32_t> indices{};// 0, 1, 2, 2, 3, 0 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue }, srcLoc);
        }

        EWEModel* Grid2D(Queue::Enum queue, glm::vec2 scale, std::source_location srcLoc) {
            const float leftX = -1.f * scale.x;
            const float rightX = 1.f * scale.x;
            const float topY = -1.f * scale.y;
            const float botY = 1.f * scale.y;

            const std::vector<VertexGrid2D> vertices{
                {leftX, topY},
                {leftX, botY},
                {rightX, topY},
                {rightX, topY},
                {leftX, botY},
                {rightX, botY}
            };
            std::vector<uint32_t> indices{};
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue }, srcLoc);
        }

        EWEModel* Quad2D(Queue::Enum queue, glm::vec2 scale, std::source_location srcLoc) {
            std::vector<VertexUI> vertices{
                {{-0.5f, -0.5f}, {0.f, 0.f}},
                {{0.5f, -0.5f}, {scale.x, 0.f}},
                {{0.5f, 0.5f}, {scale.x, scale.y}},
                {{-0.5f, 0.5f}, {0.f, scale.y}}
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue }, srcLoc);
        }
        EWEModel* NineUIQuad(Queue::Enum queue, std::source_location srcLoc) {
            std::vector<VertexUI> vertices{
                {{-0.5f, -0.5f}, {0.f, 0.f}}, //top left corner
                {{-.5f, -.5f}, {.0625f, .0625f}}, //inner top left corner

                {{0.5f, -0.5f}, {1.f, 0.f}}, //bottom left
                {{0.5f, -.5f}, {1.f - .0625f, .0625f}}, //inner bottom left

                {{0.5f, 0.5f}, {1.f, 1.f}}, //bottom right
                {{.5f, .5f}, {1.f - .0625f,1.f - .0625f}}, //inner bottom right

                {{-0.5f, 0.5f}, {0.f, 1.f}}, //top right
                {{-.5f, .5f}, {.0625f, 1.f - .0625f}}, //inner top right
            };
            std::vector<uint32_t> indices{ 1,0,6,1,6,7,1,7,3,1,3,2,1,2,0,5,4,2,5,2,3,5,3,7,5,7,6,5,6,4 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue }, srcLoc);
        }
        EWEModel* SkyBox(Queue::Enum queue, float scale, std::source_location srcLoc) {
            //hopefully never have to look at this again

            std::vector<SkyVertex> vertices = {
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
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue }, srcLoc);
        }

#else
        EWEModel* Circle(Queue::Enum queue, uint16_t const points, float radius) {
            //utilizing a triangle fan
#if EWE_DEBUG
            if (points < 5) {
                std::cout << "yo wyd? making a circle with too few points : " << points << std::endl;
            }
#endif

            glm::vec3 color = { 1.f,1.f,1.f };
            glm::vec3 normal = { 0.f, -1.f, 0.f };
            std::vector<Vertex> vertices;
            vertices.push_back({ { 0.0f,0.0f,0.0f }, normal, { 0.5f,0.5f }, color });

            float angle = glm::two_pi<float>() / points;

            for (uint16_t i = 0; i < points; i++) {
                //gonna have to fuck with UV for a while
                // 
                //i dont have a better name
                float theSin = glm::sin(angle * i);
                float theCos = glm::cos(angle * i);
                //std::cout << "theSin:theCos ~ " << theSin << " : " << theCos << std::endl; //shit is tiling when i want it to stretch
                vertices.push_back({ {radius * theSin, 0.f, radius * theCos}, normal, {(theSin + 1.f) / 2.f, (theCos + 1.f) / 2.f}, color });
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
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue });
        }

        EWEModel* Quad(Queue::Enum queue, glm::vec2 uvScale) {
            std::vector<Vertex> vertices{
                {{0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {uvScale.x,uvScale.y}, {1.f, 0.f, 0.f}},
                {{-0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {0.0f,uvScale.y}, {1.f, 0.f, 0.f}},
                {{-0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {0.0f,0.f}, {1.f, 0.f, 0.f}},
                {{0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {uvScale.x,0.f}, {1.f, 0.f, 0.f}},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2,3,0 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue });
        }
        EWEModel* QuadPNU(Queue::Enum queue, glm::vec2 uvScale) {
            std::vector<VertexNT> vertices{
                {{0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {uvScale.x,uvScale.y}},
                {{-0.5f,0.0f, -0.5f}, {0.f,1.f,0.f}, {0.0f,uvScale.y}},
                {{-0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {0.0f,0.f}},
                {{0.5f,0.0f, 0.5f}, {0.f,1.f,0.f}, {uvScale.x,0.f}},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2,3,0 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue });
        }
        EWEModel* Simple3DQuad(Queue::Enum queue, glm::vec2 uvScale) {
            std::vector<EffectVertex> vertices{
                {{0.5f,0.0f, -0.5f}, {uvScale.x,uvScale.y}},
                {{-0.5f,0.0f, -0.5f}, {0.0f,uvScale.y}},
                {{-0.5f,0.0f, 0.5f}, {0.0f,0.f}},
                {{0.5f,0.0f, 0.5f}, {uvScale.x,0.f}},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue });
        }

        EWEModel* TileQuad3D(Queue::Enum queue, glm::vec2 uvScale) {
            std::vector<TileVertex> vertices{
                {{uvScale.x,uvScale.y}},
                {{0.0f,uvScale.y}},
                {{0.0f,0.f}},
                {{0.0f,0.f}},
                {{uvScale.x,0.f}},
                {{uvScale.x,uvScale.y}},
            };
            std::vector<uint32_t> indices{};// 0, 1, 2, 2, 3, 0 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue });
        }

        EWEModel* Grid2D(Queue::Enum queue, glm::vec2 scale) {
            const float leftX = -1.f * scale.x;
            const float rightX = 1.f * scale.x;
            const float topY = -1.f * scale.y;
            const float botY = 1.f * scale.y;

            const std::vector<VertexGrid2D> vertices{
                {leftX, topY},
                {leftX, botY},
                {rightX, topY},
                {rightX, topY},
                {leftX, botY},
                {rightX, botY}
            };
            std::vector<uint32_t> indices{};
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue });
        }

        EWEModel* Quad2D(Queue::Enum queue, glm::vec2 scale) {
            std::vector<VertexUI> vertices{
                {{-0.5f, -0.5f}, {0.f, 0.f}},
                {{0.5f, -0.5f}, {scale.x, 0.f}},
                {{0.5f, 0.5f}, {scale.x, scale.y}},
                {{-0.5f, 0.5f}, {0.f, scale.y}}
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue });
        }
        EWEModel* NineUIQuad(Queue::Enum queue) {
            std::vector<VertexUI> vertices{
                {{-0.5f, -0.5f}, {0.f, 0.f}}, //top left corner
                {{-.5f, -.5f}, {.0625f, .0625f}}, //inner top left corner

                {{0.5f, -0.5f}, {1.f, 0.f}}, //bottom left
                {{0.5f, -.5f}, {1.f - .0625f, .0625f}}, //inner bottom left

                {{0.5f, 0.5f}, {1.f, 1.f}}, //bottom right
                {{.5f, .5f}, {1.f - .0625f,1.f - .0625f}}, //inner bottom right

                {{-0.5f, 0.5f}, {0.f, 1.f}}, //top right
                {{-.5f, .5f}, {.0625f, 1.f - .0625f}}, //inner top right
            };
            std::vector<uint32_t> indices{ 1,0,6,1,6,7,1,7,3,1,3,2,1,2,0,5,4,2,5,2,3,5,3,7,5,7,6,5,6,4 };
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue });
        }
        EWEModel* SkyBox(Queue::Enum queue, float scale) {
            //hopefully never have to look at this again

            std::vector<SkyVertex> vertices = {
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
            return Construct<EWEModel>({ vertices.data(), vertices.size(), sizeof(vertices[0]), indices, queue });
        }
#endif
        /*
        EWEModel* generateSimpleZedQuad(glm::vec2 uvScale = glm::vec2{ 1.f }) {
            std::vector<EffectVertex> vertices{
                {{0.5f,0.0f, -0.5f}, {uvScale.x,uvScale.y}},
                {{-0.5f,0.0f, -0.5f}, {0.0f,uvScale.y}},
                {{-0.5f,0.0f, 0.5f}, {0.0f,0.f}},
                {{0.5f,0.0f, 0.5f}, {uvScale.x,0.f}},
            };
            std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };
            return std::make_unique<EWEModel>(device, vertices, indices);
        }
        */
    }//namespace Basic_Model
} //namespace EWE