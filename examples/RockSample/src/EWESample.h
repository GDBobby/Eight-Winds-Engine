#pragma once

#include <EWEngine/EightWindsEngine.h>
#include <EWEngine/Systems/SceneManager.h>

#include "Scenes/SceneEnum.h"
#include "Scenes/MainMenuScene.h"
#include "Scenes/ShaderGenerationScene.h"
#include "Scenes/OceanScene.h"
#include "Scenes/PBRScene.h"

//#include "Scenes/FreeCameraScene.h"

namespace EWE {
	struct LoadingThreadTracker {
		bool soundMapThread = false;
		bool mainSceneThread = false;
		bool shaderGenSceneThread = false;
		bool oceanSceneThread = false;
		bool menuModuleThread = false;
		bool globalObjectThread = false;
		bool pbrSceneThread = false;

		bool Finished() const {
			return soundMapThread && mainSceneThread && menuModuleThread && globalObjectThread && pbrSceneThread;
		}
	};

	class EWESample {
	public:
		EWESample(EightWindsEngine& ewEngine, LoadingThreadTracker& loadingThreadTracker);
		~EWESample();
		EightWindsEngine& ewEngine;
		GLFWwindow* windowPtr;
		MenuManager& menuManager;
		std::shared_ptr<SoundEngine> soundEngine;

		Scene_Enum lastScene = scene_PBR;
		Scene_Enum currentScene = scene_PBR;
		std::unordered_map<Scene_Enum, SceneBase*> scenes;
		SceneBase* currentScenePtr{ nullptr };
		bool swappingScenes = false;

		bool gameRunning = true;
		double renderRefreshRate = 0.0;

		ImageID skyboxImgID{ IMAGE_INVALID };

		void mainThread();
		void addModulesToMenuManager();
		
		void loadGlobalObjects();
		//bool processClick();

		void SwapScenes();

		void LoadSceneIfMatching(Scene_Enum scene);
	};
}