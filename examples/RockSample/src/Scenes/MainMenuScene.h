#pragma once
#include <EWEngine/EightWindsEngine.h>
#include <EWEngine/Scene.h>
#include "../FloatingRockSystem.h"

namespace EWE {
	class MainMenuScene : public Scene {
		EightWindsEngine& ewEngine;
		MenuManager& menuManager;
		std::shared_ptr<SoundEngine> soundEngine;

	public:
		MainMenuScene(EightWindsEngine& ewEngine);
		~MainMenuScene();

		void load() override;
		void entry() override;
		void exit() override;
		bool render(double dt) override;

	protected:
		FloatingRock rockSystem; //this should be loaded in entry() and deleted on exit() but it requires a decent amount of computation so im just gonna keep it alive with the scene
	};
}