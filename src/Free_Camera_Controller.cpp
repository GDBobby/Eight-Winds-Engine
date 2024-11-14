#include "EWEngine/Free_Camera_Controller.h"

// std
#include <limits>
#include <iostream>

#define GK_MACRO(key) glfwGetKey(window, key) == GLFW_PRESS

namespace EWE {
    //std::vector<CameraController> CameraController::keyboardVector;
    //CameraController::CameraController(GLFWwindow* window)
    CameraController* CameraController::inputPtr;

    
    void CameraController::SetKey(int keyCode) {
        switch (selectedKey) {
            case 0: {
                //keys.moveForward = keyCode;
                break;
            }
            case 1: {
                //keys.moveLeft = keyCode;
                break;
            }
            case 2: {
                //keys.moveBackward = keyCode;
                break;
            }
            case 3: {
                //keys.moveRight = keyCode;
                break;
            }
            default: {
                std::cout << "default selected key?? " << +selectedKey << std::endl;
            }
        }

        selectedKey = -1;
    }
    void CameraController::Type(int keyCode) {
        if (keyCode == GLFW_KEY_ENTER) {
            selectedKey = -1;
            textSent = true;
        }
        else if (keyCode != GLFW_KEY_BACKSPACE) {
            typedString += glfwGetKeyName(keyCode, 0);
        }
        else if (typedString.size() > 0) {
            typedString.pop_back();
        }
    }
    
    /*
    void CameraController::setKey() {
        if (selectedKey != -1) {
            switch (selectedKey) {
            case 0: {
                keys.moveForward = staticKeys.moveForward;
                break;
            }
            case 1: {
                keys.moveLeft = staticKeys.moveLeft;
                break;
            }
            case 2: {
                keys.moveBackward = staticKeys.moveBackward;
                break;
            }
            case 3: {
                keys.moveRight = staticKeys.moveRight;
                break;
            }
            default: {
                std::cout << "default selected key?? " << selectedKey << std::endl;
            }
            }
            selectedKey = -1;
        }
    }
    */


    void CameraController::Move(TransformComponent& transform) {
        //ImGuiIO& io = ImGui::GetIO();
        //if (io.WantCaptureKeyboard) {
        //    return;
        //}

        glm::vec3 rotate{ 0.f };

        isMoveFast = (glfwGetKey(window, keys.moveFast) == GLFW_PRESS);
        isMoveSlow = (glfwGetKey(window, keys.moveSlow) == GLFW_PRESS);
        
        if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y -= 1.f;
        if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y += 1.f;
        if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x -= 1.f;
        if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x += 1.f;
        

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            transform.rotation += lookSpeed * glm::normalize(rotate);
        }

        // limit pitch values between about +/- 85ish degrees
        transform.rotation.x = glm::clamp(transform.rotation.x, -glm::half_pi<float>(), glm::half_pi<float>());
        transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

        const glm::vec3 forwardDir{ sin(transform.rotation.y), 0.f, cos(transform.rotation.y) };
        const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
        //glm::vec3 upDir{ 0.f, 1.f, 0.f };

        glm::vec3 moveDir{ 0.f };
        
        if (GK_MACRO(keys.moveForward)) {
            //printf("forward \n");
            moveDir -= forwardDir;
        }
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) {
            //printf("back \n");
            moveDir += forwardDir;
        }
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) {
            //printf("right \n");
            moveDir += rightDir;
        }
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) {
            //printf("left \n");
            moveDir -= rightDir;
        }
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) {
            //printf("up \n");
            moveDir.y -= 1.f;
        }
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) {
            //printf("down \n");
            moveDir.y += 1.f;
        }

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            //printf("moving camera? \n");
            transform.translation += ((moveSpeed + (isMoveFast * 4.f * moveSpeed))) * (1.f - (isMoveSlow * 0.8f)) * glm::normalize(moveDir);
        }
    }
    void CameraController::RotateCam(TransformComponent& transform) {
        double xPos = 0.0;
        double yPos = 0.0;
        glfwGetCursorPos(window, &xPos, &yPos);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
            float xDiff = static_cast<float>(xPos - mousePos.first);
            float yDiff = static_cast<float>(yPos - mousePos.second);
            transform.rotation.x += lookSpeed * yDiff;
            transform.rotation.y -= lookSpeed * xDiff;

            transform.rotation.x = glm::clamp(transform.rotation.x, -glm::half_pi<float>(), glm::half_pi<float>());
            transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());
        }


        mousePos.first = xPos;
        mousePos.second = yPos;
    }

    void CameraController::Zoom(TransformComponent& transform) {
        forwardDirZoom = { glm::sin(transform.rotation.y), -glm::sin(transform.rotation.x), glm::cos(transform.rotation.y) };

        //forwardDirZoom.y = -forwardDirZoom.y;
        forwardDirZoom = glm::normalize(forwardDirZoom);

        transform.translation -= forwardDirZoom * storedZoom * ((.1f + (isMoveFast * .4f)) * (1.f - (isMoveSlow * 0.8f)));

        storedZoom = 0.0;
    }

    void CameraController::Move2DPlaneXZ(float dt, Transform2dComponent& transform2d) {

        /*
        glm::vec3 moveDir{ 0.f };
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) transform2d.translation.y += 0.01f;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) transform2d.translation.y -= 0.01f;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) transform2d.translation.x += 0.01f;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) transform2d.translation.x -= 0.01f;
        */

    }


    void CameraController::DisableCursor(GLFWwindow *window) {
        //printf("camera controller \n");
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        menuActive = false;
    }

}  // namespace EWE