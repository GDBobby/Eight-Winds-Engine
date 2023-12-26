#pragma once

#include "EWEngine/graphics/model/EWE_Model.h"
#include "EWEngine/graphics/PushConstants.h"
// std
#include <memory>

/*
I can only use this for 2d view ports
*/

namespace EWE {

struct Transform2dComponent {
    glm::vec2 translation{};  // (position offset)
    glm::vec2 scale{1.f, 1.f};
    float rotation{ 0.f };

    void setPush(Simple2DPushConstantData& push) {
        push.scaleOffset = glm::vec4(scale.x, scale.y, translation.x, translation.y);
    }

    glm::vec2 mat2() {
        return scale;
        /*
        const float s = glm::sin(rotation); //sin0 = 0
        const float c = glm::cos(rotation); //cos0 = 1
        glm::mat2 rotMatrix{{c, s}, {-s, c}};
                            //1, 0,    0  1

        glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
            return rotMatrix * scaleMat;
        */
    }
};

class GameObject2D {
 public:

    static GameObject2D createGameObject() {
        return GameObject2D{};
    }

    GameObject2D(const GameObject2D &) = delete;
    GameObject2D &operator=(const GameObject2D &) = delete;
    GameObject2D(GameObject2D &&) = default;
    GameObject2D &operator=(GameObject2D &&) = default;


    //uint8_t model;
    glm::vec3 color{1.f};
    Transform2dComponent transform2d{};

    bool drawable = false;

    private:
    GameObject2D() {}

};
}  // namespace EWE