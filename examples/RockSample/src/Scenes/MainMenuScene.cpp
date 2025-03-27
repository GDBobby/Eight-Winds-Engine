#include "MainMenuScene.h"

#include <EWEngine/Systems/Rendering/Pipelines/Pipe_SimpleTextured.h>

namespace EWE {
	MainMenuScene::MainMenuScene(EightWindsEngine& ewEngine)
		: ewEngine{ ewEngine }, 
			menuManager{ ewEngine.menuManager }, 
			soundEngine{ SoundEngine::GetSoundEngineInstance() },
			rockSystem{},
			windowPtr{ ewEngine.mainWindow.getGLFWwindow() },
			camControl{ windowPtr }
	{}

	MainMenuScene::~MainMenuScene() {
#if DECONSTRUCTION_DEBUG
		printf("deconstructing main menu scene \n");
#endif
	}


	void MainMenuScene::Load() {
		menuManager.giveMenuFocus();
	}
	void MainMenuScene::Entry() {
		soundEngine->StopMusic();
		//soundEngine->playMusic(Music_Menu);

		menuManager.ChangeMenuState(menu_main, 0);
		ewEngine.camera.SetPerspectiveProjection(glm::radians(70.0f), ewEngine.eweRenderer.GetAspectRatio(), 0.1f, 1000000.0f);

		//old method
		//why is this done twice?
		//for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			ewEngine.camera.UpdateViewData({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f });
		//}
		

		//handle threads in this scene, or a game specific class
	}
	void MainMenuScene::Exit() {
		ewEngine.objectManager.eweObjects.clear();
	}
	bool MainMenuScene::Render(double dt) {
		//printf("render main menu scene \n");
		if (!paused && (glfwGetKey(windowPtr, GLFW_KEY_P) == GLFW_PRESS)) {
			paused = true;
		}
		if (paused && (glfwGetKey(windowPtr, GLFW_KEY_U) == GLFW_PRESS)) {
			paused = false;
		}
		camControl.Move(camTransform);
		camControl.RotateCam(camTransform);
		camControl.Zoom(camTransform);
		ewEngine.camera.SetViewYXZ(camTransform.translation, camTransform.rotation);
		
		if (ewEngine.BeginFrame()) {
			//printf("drawing \n");
			if (!paused) {
				rockSystem.Dispatch(dt);
			}
			ewEngine.camera.BindUBO();
			ewEngine.BeginRenderX();
			ewEngine.DrawObjects(dt);

			//rockSystem.Render();
			//printf("after displaying render info \n");
			ewEngine.EndRender();
			ewEngine.EndFrame();
			//std::cout << "after ending render \n";
			return false;
		}
		return true;
	}
}