#include "OceanScene.h"


namespace EWE {
	OceanScene::OceanScene(EightWindsEngine& ewEngine, ImageID skyboxImgID)
		: ewEngine{ ewEngine },
		menuManager{ ewEngine.menuManager },
		soundEngine{ SoundEngine::GetSoundEngineInstance() },
		cameraControl{ ewEngine.mainWindow.getGLFWwindow() }
	{
		transform.rotation.x = 0.001f;
		transform.rotation.y = 0.001f;
		ocean = Construct<Ocean::Ocean>({ Image_Manager::GetDescriptorImageInfo(skyboxImgID)});
	}
	OceanScene::~OceanScene() {
#if DECONSTRUCTION_DEBUG
		printf("deconstructing ocean scene \n");
#endif
	}


	void OceanScene::Load() {
		menuManager.giveMenuFocus();
	}
	void OceanScene::Entry() {
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
	void OceanScene::Exit() {
		ewEngine.objectManager.eweObjects.clear();
	}
	bool OceanScene::Render(double dt) {
		//printf("render main menu scene \n");


		if (ewEngine.BeginRenderWithoutPass()) {
			//printf("drawing \n");
			ocean->ReinitUpdate(dt);


			ewEngine.eweRenderer.BeginSwapChainRenderPass();

			//main controls

			cameraControl.Move(transform);
			cameraControl.RotateCam(transform);
			cameraControl.Zoom(transform);

			ewEngine.camera.SetViewYXZ(transform.translation, transform.rotation);
			ewEngine.camera.BindUBO();

			ewEngine.DrawObjects( dt);
			ocean->RenderOcean();
			//printf("after displaying render info \n");
			ewEngine.EndRender();
			ocean->TransferGraphicsToCompute();
			ewEngine.EndFrame();
			//std::cout << "after ending render \n";
			return false;
		}
		return true;
	}
}