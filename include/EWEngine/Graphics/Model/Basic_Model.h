#pragma once
#include "Model.h"

namespace EWE {
    namespace Basic_Model {
        EWEModel* Quad(Queue::Enum queue, glm::vec2 uvScale = glm::vec2{ 1.f });
        EWEModel* QuadPNU(Queue::Enum queue, glm::vec2 uvScale = glm::vec2{ 1.f });
        EWEModel* Simple3DQuad(Queue::Enum queue, glm::vec2 uvScale = glm::vec2{ 1.f });
        EWEModel* TileQuad3D(Queue::Enum queue, glm::vec2 uvScale);
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

        EWEModel* Grid2D(Queue::Enum queue, glm::vec2 scale = { 1.f,1.f });
        EWEModel* Quad2D(Queue::Enum queue, glm::vec2 scale = { 1.f,1.f });

        EWEModel* NineUIQuad(Queue::Enum queue);

        EWEModel* Circle(Queue::Enum queue, uint16_t const points, float radius = 0.5f);

        EWEModel* SkyBox(Queue::Enum queue, float scale);
    };
}

