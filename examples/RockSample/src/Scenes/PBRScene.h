#pragma once
#include <EWEngine/EightWindsEngine.h>
#include "EWEngine/Scene.h"
#include "EWEngine/Free_Camera_Controller.h"


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
		CameraController camControl;
		GLFWwindow* windowPtr;
		MenuManager& menuManager;
		std::shared_ptr<SoundEngine> soundEngine;
		TransformComponent camTransform{};

		bool drawable{false};
		std::vector<TransformComponent> transforms{};
		EWEModel* sphereModel{ nullptr };
	};
}

