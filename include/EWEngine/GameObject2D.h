#pragma once

#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/PushConstants.h"
// std
#include <memory>

/*
I can only use this for 2d view ports
*/

namespace EWE {

struct Transform2dComponent {
    //glm::vec2 translation{};  // (position offset)
    //glm::vec2 scale{1.f, 1.f};
    union {
        struct {
            glm::vec2 scale;
            glm::vec2 translation;
        };
        glm::vec4 scaleOffset;
        
    };
    Transform2dComponent() : scaleOffset{ 0.f, 0.f, 1.f, 1.f } {}

    float rotation{ 0.f };

    void setPush(Single2DPushConstantData& push) {
        push.scaleOffset = scaleOffset;
    }
    glm::vec4 getScaleOffset() {

        return scaleOffset;
    }
};

    class GameObject2D {
    public:
        GameObject2D() {}

        GameObject2D(GameObject2D const&) = delete;
        GameObject2D &operator=(GameObject2D const&) = delete;
        GameObject2D(GameObject2D &&) = default;
        GameObject2D &operator=(GameObject2D &&) = default;


        //uint8_t model;
        glm::vec3 color{1.f};
        Transform2dComponent transform2d{};

        bool drawable = false;
    };
}  // namespace EWE