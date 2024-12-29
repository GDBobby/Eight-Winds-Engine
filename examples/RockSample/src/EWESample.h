#pragma once

#include <EWEngine/EightWindsEngine.h>

#include "Scenes/SceneEnum.h"
#include "Scenes/MainMenuScene.h"
#include "Scenes/ShaderGenerationScene.h"
#include "Scenes/OceanScene.h"

//#include "Scenes/FreeCameraScene.h"

namespace EWE {
	struct LoadingThreadTracker {
		bool soundMapThread = false;
		bool mainSceneThread = false;
		bool shaderGenSceneThread = false;
		bool oceanSceneThread = false;
		bool menuModuleThread = false;
		bool globalObjectThread = false;

		bool Finished() const {
			return soundMapThread && mainSceneThread && menuModuleThread && globalObjectThread;
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

		Scene_Enum lastScene = scene_mainmenu;
		Scene_Enum currentScene = scene_mainmenu;
		std::unordered_map<Scene_Enum, Scene*> scenes;
		Scene* currentScenePtr{ nullptr };
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