#include "MainMenuScene.h"

#include <EWEngine/Systems/Rendering/Pipelines/Pipe_SimpleTextured.h>

namespace EWE {
	MainMenuScene::MainMenuScene(EightWindsEngine& ewEngine, VkDescriptorImageInfo* skyboxImage)
		: ewEngine{ ewEngine }, 
			menuManager{ ewEngine.menuManager }, 
			soundEngine{ SoundEngine::getSoundEngineInstance() },
			rockSystem{},
			cameraControl{ ewEngine.mainWindow.getGLFWwindow() }
	{
		transform.rotation.x = 0.001f;
		transform.rotation.y = 0.001f;
		ocean = ConstructSingular<Ocean::Ocean>(ewe_call_trace, skyboxImage);
	}
	MainMenuScene::~MainMenuScene() {
		printf("deconstructing main menu \n");
	}


	void MainMenuScene::load() {
		menuManager.giveMenuFocus();
		PipelineSystem::emplace(Pipe_textured, ConstructSingular<Pipe_SimpleTextured>(ewe_call_trace));
	}
	void MainMenuScene::entry() {
		soundEngine->stopMusic();
		//soundEngine->playMusic(Music_Menu);

		menuManager.changeMenuState(menu_main, 0);

		//old method
		/*
		ewEngine.camera.setViewTarget({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f }, glm::vec3(0.f, 1.f, 0.f));
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			ewEngine.camera.BindUBO(i);
		}
		*/
		

		//handle threads in this scene, or a game specific class
	}
	void MainMenuScene::exit() {
		ewEngine.objectManager.eweObjects.clear();
	}
	bool MainMenuScene::render(double dt) {
		//printf("render main menu scene \n");


		auto frameInfo = ewEngine.BeginCompute();
		if (frameInfo.cmdBuf != VK_NULL_HANDLE) {
			//printf("drawing \n");
			ocean->ReinitUpdate(frameInfo, dt);
			

			ewEngine.eweRenderer.beginSwapChainRenderPass(frameInfo.cmdBuf);
			ewEngine.skinnedRS.setFrameIndex(frameInfo.index);

			//main controls

			cameraControl.Move(transform);
			cameraControl.rotateCam(transform);
			cameraControl.zoom(transform);

			ewEngine.camera.SetViewYXZ(transform.translation, transform.rotation);
			ewEngine.camera.BindUBO(frameInfo.index);

			ewEngine.DrawObjects(frameInfo, dt);
			rockSystem.update();
			rockSystem.render(frameInfo);
			ocean->RenderOcean(frameInfo);
			//printf("after displaying render info \n");
			ewEngine.EndRender(frameInfo);
			ocean->TransferGraphicsToCompute(frameInfo.cmdBuf);
			ewEngine.EndFrame(frameInfo);
			//std::cout << "after ending render \n";
			return false;
		}
		return true;
	}
}