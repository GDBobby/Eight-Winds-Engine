#pragma once
#include "EWE_Model.h"

namespace EWE {
    struct Basic_Model {
        static std::unique_ptr<EWEModel> generateQuad(EWEDevice& device, glm::vec2 uvScale = glm::vec2{ 1.f });
        static std::unique_ptr<EWEModel> generateQuadPNU(EWEDevice& device, glm::vec2 uvScale = glm::vec2{ 1.f });
        static std::unique_ptr<EWEModel> generateSimple3DQuad(EWEDevice& device, glm::vec2 uvScale = glm::vec2{ 1.f });
        /*
        static std::unique_ptr<EWEModel> generateSimpleZedQuad(EWEDevice& device, glm::vec2 uvScale = glm::vec2{ 1.f }) {
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

        static std::unique_ptr<EWEModel> generate2DQuad(EWEDevice& device, glm::vec2 scale = { 1.f,1.f });

        static std::unique_ptr<EWEModel> generateNineUIQuad(EWEDevice& device);


        //static std::unique_ptr<EWEModel> generate3DCircle(EWEDevice& device);

        static std::unique_ptr<EWEModel> generateCircle(EWEDevice& device, uint16_t const points, float radius = 0.5f);

        static std::unique_ptr<EWEModel> createSkyBox(EWEDevice& device, float scale);
    };
}

