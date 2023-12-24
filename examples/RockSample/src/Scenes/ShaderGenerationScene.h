#pragma once

#include <EWEngine/EightWindsEngine.h>
#include "EWEngine/Scene.h"

namespace EWE {
	class ShaderGenerationScene : public Scene {
		EightWindsEngine& ewEngine;
		MenuManager& menuManager;
		std::shared_ptr<SoundEngine> soundEngine;

	public:
		ShaderGenerationScene(EightWindsEngine& ewEngine);
		~ShaderGenerationScene();

		void load() override;
		void entry() override;
		void exit() override;
		bool render(double dt) override;

	protected:
		double currentTime = 0.0;
		double saveTime = 5.0; //every 5 seconds, overkill?
		FrameInfo frameInfo;

		void SaveShader();
	};
}

