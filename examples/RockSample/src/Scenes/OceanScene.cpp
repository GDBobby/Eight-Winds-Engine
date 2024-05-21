#include "OceanScene.h"


namespace EWE {
	OceanScene::OceanScene(EightWindsEngine& ewEngine, VkDescriptorImageInfo* skyboxImage)
		: ewEngine{ ewEngine },
		menuManager{ ewEngine.menuManager },
		soundEngine{ SoundEngine::GetSoundEngineInstance() },
		cameraControl{ ewEngine.mainWindow.getGLFWwindow() }
	{
		transform.rotation.x = 0.001f;
		transform.rotation.y = 0.001f;
		ocean = ConstructSingular<Ocean::Ocean>(ewe_call_trace, skyboxImage);
	}
	OceanScene::~OceanScene() {
		printf("deconstructing main menu \n");
	}


	void OceanScene::load() {
		menuManager.giveMenuFocus();
	}
	void OceanScene::entry() {
		soundEngine->StopMusic();
		//soundEngine->playMusic(Music_Menu);

		menuManager.changeMenuState(menu_main, 0);
		ewEngine.camera.SetPerspectiveProjection(glm::radians(70.0f), ewEngine.eweRenderer.GetAspectRatio(), 0.1f, 100000.0f);

		//old method
		/*
		ewEngine.camera.setViewTarget({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f }, glm::vec3(0.f, 1.f, 0.f));
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			ewEngine.camera.BindUBO(i);
		}
		*/


		//handle threads in this scene, or a game specific class
	}
	void OceanScene::exit() {
		ewEngine.objectManager.eweObjects.clear();
	}
	bool OceanScene::render(double dt) {
		//printf("render main menu scene \n");


		auto frameInfo = ewEngine.BeginRenderWithoutPass();
		if (frameInfo.cmdBuf != VK_NULL_HANDLE) {
			//printf("drawing \n");
			ocean->ReinitUpdate(frameInfo, dt);


			ewEngine.eweRenderer.BeginSwapChainRenderPass(frameInfo.cmdBuf);
			ewEngine.skinnedRS.setFrameIndex(frameInfo.index);

			//main controls

			cameraControl.Move(transform);
			cameraControl.rotateCam(transform);
			cameraControl.zoom(transform);

			ewEngine.camera.SetViewYXZ(transform.translation, transform.rotation);
			ewEngine.camera.BindUBO(frameInfo.index);

			ewEngine.DrawObjects(frameInfo, dt);
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