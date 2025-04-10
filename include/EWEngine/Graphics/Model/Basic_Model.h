#pragma once
#include "EWEngine/Graphics/Model/Model.h"

namespace EWE {
    namespace Basic_Model {
        EWEModel* Quad(glm::vec2 uvScale = glm::vec2{ 1.f } SRC_HEADER_PARAM);
        EWEModel* QuadPNU(glm::vec2 uvScale = glm::vec2{ 1.f } SRC_HEADER_PARAM);
        EWEModel* Simple3DQuad(glm::vec2 uvScale = glm::vec2{ 1.f } SRC_HEADER_PARAM);
        EWEModel* TileQuad3D(glm::vec2 uvScale SRC_HEADER_PARAM);

        EWEModel* Grid2D(glm::vec2 scale = { 1.f,1.f } SRC_HEADER_PARAM);
        EWEModel* Grid3DTrianglePrimitive(const uint32_t patchSize, const glm::vec2 uvScale = { 1.f, 1.f } SRC_HEADER_PARAM);
        EWEModel* Grid3DQuadPrimitive(const uint32_t patchSize, glm::vec2 uvScale = {1.f, 1.f} SRC_HEADER_PARAM);
        EWEModel* Quad2D(glm::vec2 scale = { 1.f,1.f } SRC_HEADER_PARAM);

        EWEModel* NineUIQuad(SRC_HEADER_FIRST_PARAM);

        EWEModel* Circle(uint16_t const points, float radius = 0.5f SRC_HEADER_PARAM);

        EWEModel* SkyBox(float scale SRC_HEADER_PARAM);
        EWEModel* Sphere(uint8_t const subdivisions, float const radius SRC_HEADER_PARAM);
    };
}

