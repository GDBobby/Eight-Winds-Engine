#pragma once

#include "EWE_GameObject.h"
//#include "MainWindow.h"
#include "GameObject2D.h"
//#include "EWEngine/Graphics/imGuiHandler.h"

#include <iostream>

namespace EWE {
    class CameraController {
    public:
        CameraController(GLFWwindow* wndw) {
            window = wndw;
            inputPtr = this;
        }
        ~CameraController() {
            inputPtr = nullptr;
        }
        void GiveFocus() {
            glfwSetKeyCallback(window, StaticKeyCallback);
            glfwSetScrollCallback(window, Scroll_callback);
        }
        static void Scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
            //printf("%.2f stored zoom \n", yoffset);
            //ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
            //ImGuiIO& io = ImGui::GetIO();
            //if (io.WantCaptureMouse) {
            //    return;
            //}
            inputPtr->storedZoom += static_cast<float>(yoffset);
        }


        static void StaticKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
            //ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
            //ImGuiIO& io = ImGui::GetIO();
            //if (io.WantCaptureKeyboard) {
            //    return;
            //}
            //std::cout << "key callback? : " << key << std::endl;
            //std::cout << "keycallback1? : " << scancode << std::endl;
            //std::cout << "keycallback2? : " << action << std::endl;
            //std::cout << "keycallback2? : " << mods << std::endl;
            //std::string tempString = glfwGetKeyName(codepoint, 0);

            if (inputPtr->selectedKey >= 0) {
                inputPtr->SetKey(key);
            }
            if ((inputPtr->selectedKey == -2) && (action == 1)) {
                inputPtr->Type(key);
            }
        }

        static CameraController* inputPtr;
        int16_t selectedKey = -1;
        void SetKey(int keyCode);
        void Type(int keyCode);
        std::string typedString;
        bool textSent = false;

        float storedZoom = 0.f;
        glm::vec3 forwardDirZoom;
        
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
            int click = GLFW_MOUSE_BUTTON_1;
            int menuButton = GLFW_KEY_ESCAPE;
            int moveFast = GLFW_KEY_LEFT_CONTROL;
            int moveSlow = GLFW_KEY_LEFT_SHIFT;
        };
        

        void Move(TransformComponent& transform);
        void Move2DPlaneXZ(float dt, Transform2D& transform2d);
        void Zoom(TransformComponent& transform);
        void RotateCam(TransformComponent& transform);

        //glm::vec2 MenuOperation();
        void DisableCursor(GLFWwindow* window);

        KeyMappings keys{};

        float moveSpeed{ .02f };
        float lookSpeed{ .01f };
        bool clickDown = false;
        bool menuButtonDown = false;
        bool menuActive = false;

        bool isMoveFast = false;
        bool isMoveSlow = false;

        std::pair<double, double>  mousePos;

    private:
        GLFWwindow* window;



    };
}  // namespace EWE