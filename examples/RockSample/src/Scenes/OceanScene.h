#pragma once
#include <EWEngine/EightWindsEngine.h>
#include <EWEngine/Scene.h>
#include "../FloatingRockSystem.h"
#include <EWEngine/Systems/Ocean/Ocean.h>
#include <EWEngine/Free_Camera_Controller.h>

namespace EWE {
	class OceanScene : public Scene {
		EightWindsEngine& ewEngine;
		MenuManager& menuManager;
		std::shared_ptr<SoundEngine> soundEngine;

	public:
		OceanScene(EightWindsEngine& ewEngine, ImageID skyboxImgID);
		~OceanScene();

		void load() override;
		void entry() override;
		void exit() override;
		bool render(double dt) override;

	protected:
		Ocean::Ocean* ocean;
		CameraController cameraControl;
		TransformComponent transform{};
	};
}