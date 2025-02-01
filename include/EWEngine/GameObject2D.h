#pragma once

#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Data/TransformInclude.h"

/*
I can only use this for 2d view ports
*/

namespace EWE {

    class GameObject2D {
    public:
        GameObject2D() {}

        GameObject2D(GameObject2D const&) = delete;
        GameObject2D &operator=(GameObject2D const&) = delete;
        GameObject2D(GameObject2D &&) = default;
        GameObject2D &operator=(GameObject2D &&) = default;


        //uint8_t model;
        glm::vec3 color{1.f};
        Transform2D transform2d{};

        bool drawable = false;
    };
}  // namespace EWE