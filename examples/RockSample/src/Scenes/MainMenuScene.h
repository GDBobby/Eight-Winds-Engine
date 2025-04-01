#pragma once
#include "../FloatingRockSystem.h"

#include <EWEngine/EightWindsEngine.h>
#include <EWEngine/Scene.h>
#include "EWEngine/Free_Camera_Controller.h"

namespace EWE {
	class MainMenuScene : public SceneBase {
		EightWindsEngine& ewEngine;
		MenuManager& menuManager;
		std::shared_ptr<SoundEngine> soundEngine;

	public:
		MainMenuScene(EightWindsEngine& ewEngine);
		~MainMenuScene();

		void Load() final;
		void Entry() final;
		void Exit() final;
		bool Render(double dt) final;

	protected:
		FloatingRock rockSystem; //this should be loaded in entry() and deleted on exit() but it requires a decent amount of computation so im just gonna keep it alive with the scene
		GLFWwindow* windowPtr;
		CameraController camControl;
		bool paused = false;
		TransformComponent camTransform{};
		
	};
}