#include "EWESample.h"

#include "GUI/MainMenuMM.h"
#include "GUI/MenuEnums.h"
#include "Pipelines/TerrainPipe.h"
//#include "GUI/ShaderGenerationMM.h"
//#include "GUI/ControlsMM.h"
#include <EWEngine/Systems/Rendering/Stationary/StatRS.h>
#include <EWEngine/Graphics/Texture/Cube_Texture.h>
#include <EWEngine/Systems/PipelineSystem.h>

#include <EWEngine/Systems/ThreadPool.h>

#include <chrono>

namespace EWE {
	EWESample::EWESample(EightWindsEngine& ewEngine, LoadingThreadTracker& loadingThreadTracker) :
		ewEngine{ ewEngine },
		windowPtr{ ewEngine.mainWindow.getGLFWwindow() },
		menuManager{ ewEngine.menuManager },
		soundEngine{SoundEngine::GetSoundEngineInstance()}
 {


		{
			auto loadFunc = [&]() {
				printf("loading sound map : %u\n", std::this_thread::get_id());
				std::unordered_map<uint16_t, std::string> effectsMap{};
				effectsMap.emplace(0, "sounds/effects/click.mp3");
				soundEngine->LoadSoundMap(effectsMap, SoundEngine::SoundType::Effect);
				loadingThreadTracker.soundMapThread = true;
			};
			ThreadPool::EnqueueVoidFunction("load sound map thread", loadFunc);
		}
		{
			auto loadFunc = [&]() {
				printf("adding modules to menu manager : %u\n", std::this_thread::get_id());

				addModulesToMenuManager();
				loadingThreadTracker.menuModuleThread = true;
			};
			ThreadPool::EnqueueVoidFunction("load menu modules thread", loadFunc);
		}
		{
			auto loadFunc = [&]() {
				printf("loading global objects : %u\n", std::this_thread::get_id());
				loadGlobalObjects();
				loadingThreadTracker.globalObjectThread = true;
			};
			ThreadPool::EnqueueVoidFunction("load global objects thread", loadFunc);
		}

		scenes.emplace(scene_mainmenu, nullptr);
		scenes.emplace(scene_shaderGen, nullptr);
		scenes.emplace(scene_ocean, nullptr);
		scenes.emplace(scene_LevelCreation, nullptr);
		scenes.emplace(scene_PBR, nullptr);
		auto sceneLoadFunc = [&]() {
			printf("loading main menu scene : %u\n", std::this_thread::get_id());

			scenes.at(scene_mainmenu) = Construct<MainMenuScene>({ ewEngine});
			LoadSceneIfMatching(scene_mainmenu);
			loadingThreadTracker.mainSceneThread = true;

		};
		ThreadPool::EnqueueVoidFunction("load main scene thread", sceneLoadFunc);
		
		auto sceneLoadFunc2 = [&]() {
			//SetThreadName("load shader gen thread");
			printf("loading shader gen scene : %u\n", std::this_thread::get_id());
			scenes.at(scene_shaderGen) = Construct<ShaderGenerationScene>({ ewEngine });
			LoadSceneIfMatching(scene_shaderGen);
			loadingThreadTracker.shaderGenSceneThread = true;
		};
		auto sceneLoadFunc3 = [&]() {
			//SetThreadName("load ocean scene thread");
			printf("loading ocean scene : %u\n", std::this_thread::get_id());
			scenes.at(scene_ocean) = Construct<OceanScene>({ ewEngine, skyboxImgID });
			LoadSceneIfMatching(scene_ocean);
			loadingThreadTracker.oceanSceneThread = true;
		};
		auto sceneLoadFunc4 = [&] {
			scenes.at(scene_PBR) = Construct<PBRScene>({ ewEngine });
			LoadSceneIfMatching(scene_PBR);
			loadingThreadTracker.pbrSceneThread = true;
		};
		ThreadPool::EnqueueVoidFunction("load pbr scene thread", sceneLoadFunc4);
		//ThreadPool::EnqueueVoidFunction(sceneLoadFunc2);
		//ThreadPool::EnqueueVoidFunction(sceneLoadFunc3);

		//scenes.emplace(scene_)

		//StaticRenderSystem::initStaticRS(1, 1);

		//StaticRenderSystem::destructStaticRS();

	}
	EWESample::~EWESample() {
		for (auto& scene : scenes) {
			delete scene.second;
		}
		//explicitly deconstruct static objects that go into vulkan
	}
	void EWESample::mainThread() {

		auto mainThreadCurrentTime = std::chrono::high_resolution_clock::now();
		renderRefreshRate = static_cast<double>(SettingsJSON::settingsData.FPS);
		std::chrono::high_resolution_clock::time_point newTime;
		double mainThreadTimeTracker = 0.0;
		if (SettingsJSON::settingsData.FPS == 0) {
			//small value, for effectively uncapped frame rate
			//given recent games frying GPUs while frame rate is unlimited, users have to go into their settings file and change their frame rate to 0
			//idk if GPU frying will be an issue or not, but this is intentionally obscure
			renderRefreshRate = 0.00001;
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
				printf("swapping scenes beginning \n");
				//stop loading screen here
			}
			else if (mainThreadTimeTracker >= renderRefreshRate) {
				//if (processClick()) { printf("continuing on process clikc \n"); swappingScenes = true; continue; }


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

	void EWESample::loadGlobalObjects() {
		std::string skyboxLoc = "nasa/";
		skyboxImgID = Cube_Texture::CreateCubeImage(skyboxLoc);

		//i dont even know if the engine will work if this isnt constructed
		ewEngine.advancedRS.skyboxModel = Basic_Model::SkyBox(10000.f);
		ewEngine.advancedRS.CreateSkyboxDescriptor(skyboxImgID);

		//point lights are off by default
		std::vector<glm::vec3> lightColors{
			{1.f,.1f,.1f},
			{.1f,.1f,1.f},
			{.1f,1.0f,.1f},
			{1.f,1.f,.1f},
			{.1f,1.f,1.f},
			{1.f,1.f,1.f},
		};
		//NEED LESS THAN 10 POINT LIGHTS
		//shaders will only support 10
		/*
		for (int i = 0; i < lightColors.size(); i++) {
			ewEngine.objectManager.pointLights.push_back(PointLight::makePointLight(5.0f, 0.1f, lightColors[i]));
			auto rotateLight = glm::rotate(
				glm::mat4(2.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, 1.f, 0.f }
			);
			ewEngine.objectManager.pointLights[i].transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, 0.75f, -1.f, 1.f));
			ewEngine.objectManager.pointLights[i].transform.translation.x *= 5.f;
			ewEngine.objectManager.pointLights[i].transform.translation.z *= 5.f;
			ewEngine.objectManager.pointLights[i].transform.translation.y += 1.f;
		}
		*/

		PipelineSystem::Emplace(Pipe::ENGINE_MAX_COUNT, Construct<TerrainPipe>({}));
	}
	void EWESample::addModulesToMenuManager() {
		auto& mm = menuManager.menuModules.emplace(menu_main, std::make_unique<MainMenuMM>()).first->second;
		mm->labels[1].string = "2.0.0";
		mm->callbacks.push_back([&](){
				currentScene = scene_exitting;
				swappingScenes = true;

			}
		);

		//need to add the callbacks for the graphics and audio settings MMs

		
		//menuManager.menuModules.emplace(menu_ShaderGen, std::make_unique<ShaderGenerationMM>(windowPtr, screenWidth, screenHeight));
		//Shader::InputBox::giveGLFWCallbacks(MenuManager::staticMouseCallback, MenuManager::staticKeyCallback);
	}

	void EWESample::SwapScenes() {

		//loading entry?
		EWE_VK(vkDeviceWaitIdle, VK::Object->vkDevice);
		currentScenePtr->Exit();
		//loading entry?
		if (currentScene != scene_exitting) {
			currentScenePtr = scenes.at(currentScene);
			currentScenePtr->Load();
			currentScenePtr->Entry();
			lastScene = currentScene;
			//printf("swapping scenes end \n");
			swappingScenes = false;
		}
		else {
			gameRunning = false;
		}
	}
	void EWESample::LoadSceneIfMatching(Scene_Enum scene) {
		if (scene == currentScene) {
			currentScenePtr = scenes.at(currentScene);

			currentScenePtr->Load();
		}
	}
}