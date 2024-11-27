#pragma once
#include <EWEngine/EightWindsEngine.h>
#include <EWEngine/Scene.h>
#include <EWEngine/imgui/imGuiHandler.h>
#include "../GUI/imGUI/LevelCreationIMGUI.h"

#include "../Systems/TileMap.h"
#include "../Systems/TileMapDevelopment.h"

namespace EWE {
	class LevelCreationScene : public Scene {
	private:
		static LevelCreationScene* lcPtr;

		EightWindsEngine& ewEngine;
		MenuManager& menuManager;
		std::shared_ptr<SoundEngine> soundEngine;

	public:
		LevelCreationScene(EightWindsEngine& ewEngine);
		~LevelCreationScene();

		void Load() final;
		void Entry() final;
		void Exit() final;
		bool Render(double dt) final;

		void giveGLFWCallbackReturns(GLFWmousebuttonfun mouseReturnFunction, GLFWkeyfun keyReturnFunction) {
			this->mouseReturnFunction = mouseReturnFunction;
			this->keyReturnFunction = keyReturnFunction;
		}

	protected:
		std::unique_ptr<ImGUIHandler> imguiHandler{nullptr};

		bool show_demo_window = true;
		bool logicActive = false;
		//void logicThreadFunction();
		std::unique_ptr<std::thread> logicThread;
		LevelCreationIMGUI levelCreationIMGUI;

		void renderBackgroundGrid();
		void renderTiles(uint8_t frameIndex);
		glm::vec2 pushScale;
		glm::vec2 pushTrans;
		glm::vec2 pushGridScale{1.f};

		static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void rightClickDragCallback(GLFWwindow* window, double xpos, double ypos);
		static void leftClickDragCallback(GLFWwindow* window, double xpos, double ypos);

		int64_t lastTileDragPos;


		GLFWmousebuttonfun mouseReturnFunction;
		GLFWkeyfun keyReturnFunction;

		double currentMouseXPos;
		double currentMouseYPos;

		static void createLevel(uint16_t width, uint16_t height);

		void fitToScreen();

		std::unique_ptr<EWEModel> gridModel{ nullptr };

		std::unique_ptr<TileMapDevelopment> tileMapD{nullptr};
	};
}