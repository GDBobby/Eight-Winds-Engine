#include "ShaderGenerationScene.h"
#include "../GUI/MenuEnums.h"

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

		printf("after updating pipelines load menu objects, returning \n");
	}
	void ShaderGenerationScene::entry() {
		soundEngine->stopMusic();
		//soundEngine->playMusic(Music_Menu);

		menuManager.changeMenuState(menu_ShaderGen, 0);


		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			ewEngine.camera.UpdateViewData({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f }, glm::vec3(0.f, 1.f, 0.f));
		}
	}
	void ShaderGenerationScene::exit() {
	}

	bool ShaderGenerationScene::render(double dt) {
		//printf("render main menu scene \n");


		auto frameInfo = ewEngine.BeginRender();
		if (frameInfo.cmdBuf != VK_NULL_HANDLE) {
			//printf("drawing \n");
			ewEngine.Draw2DObjects(frameInfo);
			ewEngine.DrawText(frameInfo, dt);

			currentTime += dt;
			if (currentTime >= saveTime) {
				SaveShader();
			}

			ewEngine.EndRender(frameInfo);
			ewEngine.EndFrame(frameInfo);
			return false;
		}
		return true;
	}

	void ShaderGenerationScene::SaveShader() {

	}
}