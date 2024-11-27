#pragma once
#include "Model.h"

namespace EWE {
    namespace Basic_Model {
#if CALL_TRACING
        EWEModel* Quad(Queue::Enum queue, glm::vec2 uvScale = glm::vec2{ 1.f }, std::source_location = std::source_location::current());
        EWEModel* QuadPNU(Queue::Enum queue, glm::vec2 uvScale = glm::vec2{ 1.f }, std::source_location = std::source_location::current());
        EWEModel* Simple3DQuad(Queue::Enum queue, glm::vec2 uvScale = glm::vec2{ 1.f }, std::source_location = std::source_location::current());
        EWEModel* TileQuad3D(Queue::Enum queue, glm::vec2 uvScale, std::source_location = std::source_location::current());


        EWEModel* Grid2D(Queue::Enum queue, glm::vec2 scale = { 1.f,1.f }, std::source_location = std::source_location::current());
        EWEModel* Quad2D(Queue::Enum queue, glm::vec2 scale = { 1.f,1.f }, std::source_location = std::source_location::current());

        EWEModel* NineUIQuad(Queue::Enum queue, std::source_location = std::source_location::current());

        EWEModel* Circle(Queue::Enum queue, uint16_t const points, float radius = 0.5f, std::source_location = std::source_location::current());

        EWEModel* SkyBox(Queue::Enum queue, float scale, std::source_location = std::source_location::current());
#else
        EWEModel* Quad(Queue::Enum queue, glm::vec2 uvScale = glm::vec2{ 1.f });
        EWEModel* QuadPNU(Queue::Enum queue, glm::vec2 uvScale = glm::vec2{ 1.f });
        EWEModel* Simple3DQuad(Queue::Enum queue, glm::vec2 uvScale = glm::vec2{ 1.f });
        EWEModel* TileQuad3D(Queue::Enum queue, glm::vec2 uvScale);


        EWEModel* Grid2D(Queue::Enum queue, glm::vec2 scale = { 1.f,1.f });
        EWEModel* Quad2D(Queue::Enum queue, glm::vec2 scale = { 1.f,1.f });

        EWEModel* NineUIQuad(Queue::Enum queue);

        EWEModel* Circle(Queue::Enum queue, uint16_t const points, float radius = 0.5f);

        EWEModel* SkyBox(Queue::Enum queue, float scale);
#endif
    };
}

