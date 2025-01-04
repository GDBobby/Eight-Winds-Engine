#pragma once
#include "EWEngine/Graphics/Model/Model.h"

namespace EWE {
    namespace Basic_Model {
#if CALL_TRACING
        EWEModel* Quad(glm::vec2 uvScale = glm::vec2{ 1.f }, std::source_location = std::source_location::current());
        EWEModel* QuadPNU(glm::vec2 uvScale = glm::vec2{ 1.f }, std::source_location = std::source_location::current());
        EWEModel* Simple3DQuad(glm::vec2 uvScale = glm::vec2{ 1.f }, std::source_location = std::source_location::current());
        EWEModel* TileQuad3D(glm::vec2 uvScale, std::source_location = std::source_location::current());

        EWEModel* Grid2D(glm::vec2 scale = { 1.f,1.f }, std::source_location = std::source_location::current());
        EWEModel* Quad2D(glm::vec2 scale = { 1.f,1.f }, std::source_location = std::source_location::current());

        EWEModel* NineUIQuad(std::source_location = std::source_location::current());

        EWEModel* Circle(uint16_t const points, float radius = 0.5f, std::source_location = std::source_location::current());

        EWEModel* SkyBox(float scale, std::source_location = std::source_location::current());
#else
        EWEModel* Quad(glm::vec2 uvScale = glm::vec2{ 1.f });
        EWEModel* QuadPNU(glm::vec2 uvScale = glm::vec2{ 1.f });
        EWEModel* Simple3DQuad(glm::vec2 uvScale = glm::vec2{ 1.f });
        EWEModel* TileQuad3D(glm::vec2 uvScale);


        EWEModel* Grid2D(glm::vec2 scale = { 1.f,1.f });
        EWEModel* Quad2D(glm::vec2 scale = { 1.f,1.f });

        EWEModel* NineUIQuad();

        EWEModel* Circle(uint16_t const points, float radius = 0.5f);

        EWEModel* SkyBox(float scale);
#endif
    };
}

