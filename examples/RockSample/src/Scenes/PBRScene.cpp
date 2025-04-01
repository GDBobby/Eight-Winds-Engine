#include "PBRScene.h"

#include <EWEngine/Systems/Rendering/Rigid/RigidRS.h>

namespace EWE {

	PBRScene::PBRScene(EightWindsEngine& ewEngine)
		: ewEngine{ ewEngine },
		menuManager{ ewEngine.menuManager },
		soundEngine{ SoundEngine::GetSoundEngineInstance() },
		windowPtr{ ewEngine.mainWindow.getGLFWwindow() },
		camControl{ windowPtr }
	{}

	PBRScene::~PBRScene() {
#if DECONSTRUCTION_DEBUG
		printf("deconstructing main menu scene \n");
#endif
	}
	void PBRScene::Exit() {
		assert(sphereModel != nullptr);
		Deconstruct(sphereModel);
		sphereModel = nullptr;
	}
	void PBRScene::Load() {
		menuManager.giveMenuFocus();
		assert(sphereModel == nullptr);
		sphereModel = Basic_Model::Sphere(4, 1.f);

		MaterialInfo matInfo;
		matInfo.imageID = IMAGE_INVALID;
		matInfo.materialFlags = Material::no_texture;
		MaterialObjectInfo matObjInfo;
		matObjInfo.drawable = &drawable;
		matObjInfo.meshPtr = sphereModel;
		matObjInfo.ownerTransform = &transforms[0];

		//RigidRenderingSystem::AddMaterialObject(matInfo, matObjInfo);
		RigidRenderingSystem::AddInstancedMaterialObject(matInfo, sphereModel, 16, false);
	}

	void PBRScene::Entry() {
		soundEngine->StopMusic();

		menuManager.ChangeMenuState(menu_main, 0);
		ewEngine.camera.SetPerspectiveProjection(glm::radians(70.0f), ewEngine.eweRenderer.GetAspectRatio(), 0.1f, 1000000.0f);

		ewEngine.camera.UpdateViewData({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f });
	}
	bool PBRScene::Render(double dt) {
		//printf("render main menu scene \n");
		//if (!paused && (glfwGetKey(windowPtr, GLFW_KEY_P) == GLFW_PRESS)) {
		//	paused = true;
		//}
		//if (paused && (glfwGetKey(windowPtr, GLFW_KEY_U) == GLFW_PRESS)) {
		//	paused = false;
		//}
		camControl.Move(camTransform);
		camControl.RotateCam(camTransform);
		camControl.Zoom(camTransform);
		ewEngine.camera.SetViewYXZ(camTransform.translation, camTransform.rotation);

		if (ewEngine.BeginFrame()) {
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