#pragma once

#include <EWEngine/EightWindsEngine.h>

#include "Scenes/SceneEnum.h"
#include "Scenes/MainMenuScene.h"
#include "Scenes/ShaderGenerationScene.h"
#include "Scenes/OceanScene.h"
#include "Scenes/LevelCreationScene.h"

#include "Pipelines/PipelineHeaderWrapper.h"

//#include "Scenes/FreeCameraScene.h"
#include <functional>

namespace EWE {
	struct LoadingThreadTracker {
		bool soundMapThread = false;
		bool mainSceneThread = false;
		bool shaderGenSceneThread = false;
		bool oceanSceneThread = false;
		bool menuModuleThread = false;
		bool globalObjectThread = false;
		bool levelCreationSceneThread = false;

		bool Finished() const {
			return soundMapThread && mainSceneThread && menuModuleThread && globalObjectThread && levelCreationSceneThread;
		}
	};

	class EWESample {
	public:
		EWESample(EightWindsEngine& ewEngine, LoadingThreadTracker& loadingThreadTracker);
		~EWESample();
		EightWindsEngine& ewEngine;
		GLFWwindow* windowPtr;
		MenuManager& menuManager;

		Scene_Enum lastScene = scene_LevelCreation;
		Scene_Enum currentScene = scene_LevelCreation;
		std::unordered_map<Scene_Enum, Scene*> scenes;
		Scene* currentScenePtr{ nullptr };
		bool swappingScenes = false;
		std::shared_ptr<SoundEngine> soundEngine;

		bool gameRunning = true;
		double renderRefreshRate = 0.0;

		ImageID skyboxImgID{ IMAGE_INVALID };

		void mainThread();
		void addModulesToMenuManager(float screenWidth, float screenHeight);
		
		void loadGlobalObjects();
		bool processClick();

		void SwapScenes();

		void LoadSceneIfMatching(Scene_Enum scene);

		void AddPipelinesToSystem();
	};
}