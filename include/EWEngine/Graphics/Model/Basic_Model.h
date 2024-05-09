#pragma once
#include "Model.h"

namespace EWE {
    struct Basic_Model {
        static std::unique_ptr<EWEModel> generateQuad(glm::vec2 uvScale = glm::vec2{ 1.f });
        static std::unique_ptr<EWEModel> generateQuadPNU(glm::vec2 uvScale = glm::vec2{ 1.f });
        static std::unique_ptr<EWEModel> GenerateSimple3DQuad(glm::vec2 uvScale = glm::vec2{ 1.f });
        static std::unique_ptr<EWEModel> generate3DTileQuad(glm::vec2 uvScale);
        /*
        static std::unique_ptr<EWEModel> generateSimpleZedQuad(glm::vec2 uvScale = glm::vec2{ 1.f }) {
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

        static std::unique_ptr<EWEModel> generate2DGrid(glm::vec2 scale = { 1.f,1.f });
        static std::unique_ptr<EWEModel> generate2DQuad(glm::vec2 scale = { 1.f,1.f });

        static std::unique_ptr<EWEModel> generateNineUIQuad();

        static std::unique_ptr<EWEModel> generateCircle(uint16_t const points, float radius = 0.5f);

        static std::unique_ptr<EWEModel> createSkyBox(float scale);
    };
}

