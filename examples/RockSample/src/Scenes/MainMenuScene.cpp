#include "MainMenuScene.h"

#include <EWEngine/Systems/Rendering/Pipelines/Pipe_SimpleTextured.h>

namespace EWE {
	MainMenuScene::MainMenuScene(EightWindsEngine& ewEngine)
		: ewEngine{ ewEngine }, 
			menuManager{ ewEngine.menuManager }, 
			soundEngine{ SoundEngine::GetSoundEngineInstance() },
			rockSystem{}
	{
	}
	MainMenuScene::~MainMenuScene() {
		printf("deconstructing main menu \n");
	}


	void MainMenuScene::load() {
		menuManager.giveMenuFocus();
		PipelineSystem::Emplace(Pipe::textured, ConstructSingular<Pipe_SimpleTextured>(ewe_call_trace));
	}
	void MainMenuScene::entry() {
		soundEngine->StopMusic();
		//soundEngine->playMusic(Music_Menu);

		menuManager.changeMenuState(menu_main, 0);
		ewEngine.camera.SetPerspectiveProjection(glm::radians(70.0f), ewEngine.eweRenderer.GetAspectRatio(), 0.1f, 1000000.0f);

		//old method
		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			ewEngine.camera.UpdateViewData({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f }, glm::vec3(0.f, 1.f, 0.f));
		}
		

		//handle threads in this scene, or a game specific class
	}
	void MainMenuScene::exit() {
		PipelineSystem::DestructAt(Pipe::textured);
		ewe_free(PipelineSystem::At(Pipe::textured));
		ewEngine.objectManager.eweObjects.clear();
	}
	bool MainMenuScene::render(double dt) {
		//printf("render main menu scene \n");


		FrameInfo frameInfo = ewEngine.BeginRender();
		if (frameInfo.cmdBuf != VK_NULL_HANDLE) {
			//printf("drawing \n");

			ewEngine.DrawObjects(frameInfo, dt);

			rockSystem.update();
			rockSystem.render(frameInfo);
			//printf("after displaying render info \n");
			ewEngine.EndRender(frameInfo);
			ewEngine.EndFrame(frameInfo);
			//std::cout << "after ending render \n";
			return false;
		}
		return true;
	}
}