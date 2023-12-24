#include "ShaderGenerationScene.h"
#include "../gui/MenuEnums.h"

namespace EWE {
	ShaderGenerationScene::ShaderGenerationScene(EightWindsEngine& ewEngine)
		: ewEngine{ ewEngine },
		menuManager{ ewEngine.menuManager },
		soundEngine{ SoundEngine::getSoundEngineInstance() }
	{}
	ShaderGenerationScene::~ShaderGenerationScene() {
		printf("deconstructing main menu \n");
	}

	void ShaderGenerationScene::load() {
		menuManager.giveMenuFocus();

		ewEngine.advancedRS.updatePipelines(ewEngine.objectManager, ewEngine.eweRenderer.getPipelineInfo());
		printf("after updating pipelines load menu objects, returning \n");
	}
	void ShaderGenerationScene::entry() {
		soundEngine->stopMusic();
		//soundEngine->playMusic(Music_Menu);

		menuManager.changeMenuState(menu_ShaderGen, 0);


		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			ewEngine.camera.updateViewData({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f }, glm::vec3(0.f, 1.f, 0.f));
		}
	}
	void ShaderGenerationScene::exit() {
	}

	bool ShaderGenerationScene::render(double dt) {
		//printf("render main menu scene \n");


		auto cmdBufFrameIndex = ewEngine.beginRender();
		if (cmdBufFrameIndex.first != VK_NULL_HANDLE) {
			//printf("drawing \n");
			ewEngine.draw2DObjects(cmdBufFrameIndex);
			ewEngine.drawText(cmdBufFrameIndex, dt);

			currentTime += dt;
			if (frameInfo.time >= saveTime) {
				SaveShader();
			}

			ewEngine.endRender(cmdBufFrameIndex);
			return false;
		}
		return true;
	}

	void ShaderGenerationScene::SaveShader() {

	}
}