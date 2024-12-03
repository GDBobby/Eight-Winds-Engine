#include "EWEngine/Systems/SceneManager.h"
#include "EWEngine/Systems/ThreadPool.h"


namespace EWE {
	SceneManager::SceneManager(EightWindsEngine& ewEngine) : ewEngine{ ewEngine } {

		//this prevents realignment
		scenes.reserve(255);
	}
	SceneManager::~SceneManager() {
		for (auto& scene : scenes) {
			Deconstruct(scene.second);
		}
	}

	void SceneManager::RunSceneLoop() {
		auto mainThreadCurrentTime = std::chrono::high_resolution_clock::now();
		renderRefreshRate = static_cast<double>(SettingsJSON::settingsData.FPS);
		std::chrono::high_resolution_clock::time_point newTime;
		double mainThreadTimeTracker = 0.0;
		if (SettingsJSON::settingsData.FPS == 0) {
			//small value, for effectively uncapped frame rate
			//given recent games frying GPUs while frame rate is unlimited, users have to go into their settings file and change their frame rate to 0
			//idk if GPU frying will be an issue or not, but this is intentionally obscure
			renderRefreshRate = 1.0 / 1000.0;
		}
		else {
			renderRefreshRate = 1.0 / renderRefreshRate;
		}
		do { //having a simple while() may cause a race condition
			EWE_VK(vkDeviceWaitIdle, VK::Object->vkDevice);
		} while (ewEngine.GetLoadingScreenProgress());

		currentScenePtr->Entry();

		while (gameRunning) {
			glfwPollEvents();
			newTime = std::chrono::high_resolution_clock::now();
			mainThreadTimeTracker += std::chrono::duration<double, std::chrono::seconds::period>(newTime - mainThreadCurrentTime).count();
			mainThreadCurrentTime = newTime;

			if (swappingScenes) [[unlikely]] {
				SwapScenes();
				mainThreadTimeTracker = renderRefreshRate;
#if EWE_DEBUG
				printf("swapping scenes beginning \n");
#endif
				//stop loading screen here
			}
			else if (mainThreadTimeTracker >= renderRefreshRate) {


				//std::cout << "currentScene at render : " << currentScene << std::endl;
				//ewEngine.camera.PrintCameraPos();
				currentScenePtr->Render(mainThreadTimeTracker);

				if (mainThreadTimeTracker > ewEngine.peakRenderTime) {
					printf("peak render time : % .5f \n", mainThreadTimeTracker);
					ewEngine.peakRenderTime = mainThreadTimeTracker;
				}
				ewEngine.renderFramesCounted++;
				ewEngine.totalRenderTime += mainThreadTimeTracker;
				if ((ewEngine.renderFramesCounted % 144) == 0) {
					ewEngine.averageRenderTime = ewEngine.totalRenderTime / 144;
					//std::cout << "peak Render Time : " << peakRenderTime << std::endl;
					//std::cout << "average Render Time (every 1000th frame) : " << averageRenderTime << std::endl;
					//std::cout << "min Render Time : " << minRenderTime << std::endl;
					//ewEngine.peakRenderTime = 0.0f;
					ewEngine.totalRenderTime = 0.0f;
					ewEngine.renderFramesCounted = 0;
				}
				mainThreadTimeTracker = 0.0;
			}
		}
	}
	void SceneManager::ChangeScene(SceneKey sceneKey) {
#if EWE_DEBUG
		assert(lastScene == currentScene && "changing scenes too quickly");
#endif
		currentScene = sceneKey;
		swappingScenes = true;
	}
	
	uint8_t SceneManager::AddScene(Scene* scene) {
#if EWE_DEBUG
		assert(scene != nullptr);
		assert((scenes.size() + 1) < scene_exit);
#endif
		
		SceneKey currentSize = scenes.size() + 1;
		scenes.emplace(currentSize, scene);

		return currentSize;
	}

	void SceneManager::SetStartupScene(SceneKey sceneKey) {
		currentScene = sceneKey;
		currentScenePtr = scenes.at(sceneKey);
	}

	void SceneManager::SwapScenes() {
		EWE_VK(vkDeviceWaitIdle, VK::Object->vkDevice);
		currentScenePtr->Exit();
		ewEngine.objectManager.ClearSceneObjects();
		if (currentScene != scene_exit) {
			currentScenePtr = scenes.at(currentScene);
			currentScenePtr->Load();
			currentScenePtr->Entry();
			lastScene = currentScene;
			swappingScenes = false;
		}
		else {
			gameRunning = false;
		}
	}

}//namespace EWE
