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
		printf("deconstructing main menu \n");
#endif
	}


	void MainMenuScene::load() {
		menuManager.giveMenuFocus();
	}
	void MainMenuScene::entry() {
		soundEngine->StopMusic();
		//soundEngine->playMusic(Music_Menu);

		menuManager.changeMenuState(menu_main, 0);
		ewEngine.camera.SetPerspectiveProjection(glm::radians(70.0f), ewEngine.eweRenderer.GetAspectRatio(), 0.1f, 1000000.0f);

		//old method
		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			ewEngine.camera.UpdateViewData({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f });
		}
		

		//handle threads in this scene, or a game specific class
	}
	void MainMenuScene::exit() {
		PipelineSystem::DestructAt(Pipe::textured);
		ewEngine.objectManager.eweObjects.clear();
	}
	bool MainMenuScene::render(double dt) {
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

		FrameInfo frameInfo = ewEngine.BeginRenderWithoutPass();
		
		if (frameInfo.cmdBuf != VK_NULL_HANDLE) {
			//printf("drawing \n");
			rockSystem.Dispatch(dt * !paused, frameInfo);
			
			ewEngine.camera.BindUBO(frameInfo.index);
			ewEngine.eweRenderer.BeginSwapChainRenderPass(frameInfo.cmdBuf);
			ewEngine.DrawObjects(frameInfo, dt);

			//rockSystem.Render(frameInfo);
			//printf("after displaying render info \n");
			ewEngine.EndRender(frameInfo);
			ewEngine.EndFrame(frameInfo);
			//std::cout << "after ending render \n";
			return false;
		}
		return true;
	}
}