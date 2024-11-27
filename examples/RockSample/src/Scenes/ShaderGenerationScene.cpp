#include "ShaderGenerationScene.h"
#include "../GUI/MenuEnums.h"

namespace EWE {
	ShaderGenerationScene::ShaderGenerationScene(EightWindsEngine& ewEngine)
		: ewEngine{ ewEngine },
		menuManager{ ewEngine.menuManager },
		soundEngine{ SoundEngine::GetSoundEngineInstance() }
	{}
	ShaderGenerationScene::~ShaderGenerationScene() {
		printf("deconstructing shader scene \n");
	}

	void ShaderGenerationScene::Load() {
		menuManager.giveMenuFocus();

		printf("after updating pipelines load menu objects, returning \n");
	}
	void ShaderGenerationScene::Entry() {
		soundEngine->StopMusic();
		//soundEngine->playMusic(Music_Menu);

		menuManager.changeMenuState(menu_ShaderGen, 0);


		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			ewEngine.camera.UpdateViewData({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f }, glm::vec3(0.f, 1.f, 0.f));
		}
	}
	void ShaderGenerationScene::Exit() {
	}

	bool ShaderGenerationScene::Render(double dt) {
		//printf("render main menu scene \n");


		if (ewEngine.BeginRender()) {
			//printf("drawing \n");
			ewEngine.Draw2DObjects();
			ewEngine.DrawText(dt);

			currentTime += dt;
			if (currentTime >= saveTime) {
				SaveShader();
			}

			ewEngine.EndRender();
			ewEngine.EndFrame();
			return false;
		}
		return true;
	}

	void ShaderGenerationScene::SaveShader() {

	}
}