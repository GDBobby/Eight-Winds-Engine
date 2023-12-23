#pragma once

#include "EWE_GameObject.h"
//#include "MainWindow.h"
#include "GameObject2D.h"
//#include "EWEngine/graphics/imGuiHandler.h"

#include <iostream>

namespace EWE {
    class CameraController {
    public:
        CameraController(GLFWwindow* wndw) {
            window = wndw;
            inputPtr = this;
            glfwSetKeyCallback(wndw, staticKeyCallback);
            glfwSetScrollCallback(window, scroll_callback);
        }
        ~CameraController() {
            inputPtr = nullptr;
        }
        void giveFocus() {
            glfwSetKeyCallback(window, staticKeyCallback);
            glfwSetScrollCallback(window, scroll_callback);
        }
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
            //printf("%.2f stored zoom \n", yoffset);
            //ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
            //ImGuiIO& io = ImGui::GetIO();
            //if (io.WantCaptureMouse) {
            //    return;
            //}
            inputPtr->storedZoom += static_cast<float>(yoffset);
        }


        static void staticKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
                inputPtr->setKey(key);
            }
            if ((inputPtr->selectedKey == -2) && (action == 1)) {
                inputPtr->type(key);
            }
        }

        static CameraController* inputPtr;
        short selectedKey = -1;
        void setKey(int keyCode);
        void type(int keyCode);
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
        

        void moveInPlaneXZ(EWEGameObject& gameObject);
        void move2DPlaneXZ(float dt, Transform2dComponent& transform2d);
        void zoom(EWEGameObject* gameObject);
        void rotateCam(EWEGameObject& gameObject);

        glm::vec2 menuOperation();
        void disableMenu(GLFWwindow* window);

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