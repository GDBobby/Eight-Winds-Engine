#include "MainMenuScene.h"

namespace EWE {
	MainMenuScene::MainMenuScene(EightWindsEngine& ewEngine)
		: ewEngine{ ewEngine }, 
			menuManager{ ewEngine.menuManager }, 
			soundEngine{ SoundEngine::getSoundEngineInstance() },
			rockSystem{ ewEngine.eweDevice }
		{}
	MainMenuScene::~MainMenuScene() {
		printf("deconstructing main menu \n");
	}


	void MainMenuScene::load() {
		menuManager.giveMenuFocus();

		

		ewEngine.advancedRS.updatePipelines(ewEngine.objectManager, ewEngine.eweRenderer.getPipelineInfo());
		printf("after updating pipelines load menu objects, returning \n");
	}
	void MainMenuScene::entry() {
		soundEngine->stopMusic();
		//soundEngine->playMusic(Music_Menu);

		menuManager.changeMenuState(menu_main, 0);
		ewEngine.camera.newViewTarget({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f }, glm::vec3(0.f, 1.f, 0.f));

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			ewEngine.camera.bindUBO(i);
		}
		//ewEngine.renderThread.reset(new std::thread(&MainMenuScene::Render, this));
		//ewEngine.logicThread.reset();

		//handle threads in this scene, or a game specific class
	}
	void MainMenuScene::exit() {
		ewEngine.objectManager.eweObjects.clear();
	}
	bool MainMenuScene::render(double dt) {
		//printf("render main menu scene \n");

		
		auto cmdBufFrameIndex = ewEngine.beginRender();
		if (cmdBufFrameIndex.first != VK_NULL_HANDLE) {
			//printf("drawing \n");
			ewEngine.drawObjects(cmdBufFrameIndex, dt);
			FrameInfo frameInfo;
			frameInfo.cmdIndexPair = cmdBufFrameIndex;
			frameInfo.time = static_cast<float>(dt);
			rockSystem.update();
			rockSystem.render(frameInfo);
			//printf("after displaying render info \n");
			ewEngine.endRender(cmdBufFrameIndex);
			//std::cout << "after ending render \n";
			return false;
		}
		return true;
	}
}