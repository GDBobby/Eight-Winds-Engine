#pragma once

#include <EWEngine/EightWindsEngine.h>
#include "EWEngine/Scene.h"

namespace EWE {
	class ShaderGenerationScene : public SceneBase {
		EightWindsEngine& ewEngine;
		MenuManager& menuManager;
		std::shared_ptr<SoundEngine> soundEngine;

	public:
		ShaderGenerationScene(EightWindsEngine& ewEngine);
		~ShaderGenerationScene();

		void Load() final;
		void Entry() final;
		void Exit() final;
		bool Render(double dt) final;

	protected:
		double currentTime = 0.0;
		double saveTime = 5.0; //every 5 seconds, overkill?

		void SaveShader();
	};
}

