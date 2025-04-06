#pragma once
#include <EWEngine/EightWindsEngine.h>
#include "EWEngine/Scene.h"
#include "EWEngine/Free_Camera_Controller.h"
#include <EWEngine/Systems/Rendering/Rigid/RigidRS.h>
#include <EWEngine/imgui/imGuiHandler.h>

#include "../Pipelines/TerrainPipe.h"


namespace EWE {
	class PBRScene : public SceneBase {
	public:
		PBRScene(EightWindsEngine& ewEngine);
		~PBRScene();

		void Load() final;
		void Entry() final;
		void Exit() final;
		bool Render(double dt) final;

		EightWindsEngine& ewEngine;
		MenuManager& menuManager;
		std::shared_ptr<SoundEngine> soundEngine;
		GLFWwindow* windowPtr;
		CameraController camControl;
		TransformComponent camTransform{};

		TransformComponent sphereTransform;
		bool sphereDrawable = true;
		MaterialObjectInfo controlledSphere;

		ImGUIHandler imguiHandler;

		std::array<EWEBuffer*, 2> csmEWEBuffer; //csmEWE == controlled sphere material EWE buffer
		MaterialBuffer controlledSphereMB;
		int updatedCMB = 0; //CMB == controlled material buffer

		EWEModel* groundModel{ nullptr };
		EWEModel* sphereModel{ nullptr };

		VkDescriptorSet terrainDesc[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };

		void RenderLBOControls();
		void RenderCameraData();
		void RenderControlledSphereControls();

		int updatedLBO = 0;
		LightBufferObject lbo;
		TessBufferObject tbo;
	};
}

