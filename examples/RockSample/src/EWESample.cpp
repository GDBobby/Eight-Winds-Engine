#include "EWESample.h"

#include "GUI/MainMenuMM.h"
//#include "GUI/ShaderGenerationMM.h"
//#include "GUI/ControlsMM.h"
#include <EWEngine/Systems/Rendering/Stationary/StatRS.h>
#include <EWEngine/Graphics/Texture/Cube_Texture.h>

#include "GUI/MenuEnums.h"


#include <chrono>

namespace EWE {
	EWESample::EWESample(EightWindsEngine& ewEngine) :
		ewEngine{ ewEngine },
		menuManager{ ewEngine.menuManager },
		soundEngine{SoundEngine::getSoundEngineInstance()},
		windowPtr{ewEngine.mainWindow.getGLFWwindow()}
 {
		float screenWidth = ewEngine.uiHandler.getScreenWidth();
		float screenHeight = ewEngine.uiHandler.getScreenHeight();

		std::unordered_map<uint16_t, std::string> effectsMap{};
		effectsMap.emplace(0, "sounds/effects/click.mp3");
		printf("loading effects \n");
		soundEngine->loadSoundMap(effectsMap, SoundEngine::SoundType::Effect);

		addModulesToMenuManager(screenWidth, screenHeight);
		loadGlobalObjects();
		currentScene = scene_mainmenu;
		scenes.emplace(scene_mainmenu, std::make_unique<MainMenuScene>(ewEngine, skyboxInfo));
		scenes.emplace(scene_shaderGen, std::make_unique<ShaderGenerationScene>(ewEngine));
		//scenes.emplace(scene_)
		currentScenePtr = scenes.at(currentScene).get();

		currentScenePtr->load();


		//StaticRenderSystem::initStaticRS(1, 1);

		//StaticRenderSystem::destructStaticRS();

		ewEngine.EndEngineLoadScreen();
	}
	EWESample::~EWESample() {
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
			//idk if that will be an issue or not, but this is intentionally obscure
			renderRefreshRate = 0.00001;
		}
		else {
			renderRefreshRate = 1.0 / renderRefreshRate;
		}
		do { //having a simple while() may cause a race condition
			vkDeviceWaitIdle(ewEngine.eweDevice.device());
		} while (ewEngine.GetLoadingScreenProgress());
		currentScenePtr->entry();

		while (gameRunning) {
			glfwPollEvents();
			newTime = std::chrono::high_resolution_clock::now();
			mainThreadTimeTracker += std::chrono::duration<double, std::chrono::seconds::period>(newTime - mainThreadCurrentTime).count();
			mainThreadCurrentTime = newTime;

			if (swappingScenes) {
				printf("swapping scenes beginning \n");

				//loading entry?
				vkDeviceWaitIdle(ewEngine.eweDevice.device());
				currentScenePtr->exit();
				ewEngine.objectManager.clearSceneObjects();
				Texture_Manager::GetTextureManagerPtr()->ClearSceneTextures();
				//loading entry?
				if (currentScene != scene_exitting) {
					currentScenePtr = scenes.at(currentScene).get();
					currentScenePtr->load();
					currentScenePtr->entry();
					lastScene = currentScene;
					//printf("swapping scenes end \n");
					swappingScenes = false;
					mainThreadTimeTracker = renderRefreshRate;
				}
				else {
					gameRunning = false;
				}
				//stop loading screen here
			}
			else if (mainThreadTimeTracker >= renderRefreshRate) {
				if (processClick()) { printf("continuing on process clikc \n"); swappingScenes = true; continue; }

				//std::cout << "currentScene at render : " << currentScene << std::endl;
				if (currentScenePtr->render(mainThreadTimeTracker)) {
					//resize functions here
				}

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
		TextureDesc skyboxID = Cube_Texture::createCubeTexture("nasa/");
		skyboxInfo = Texture_Manager::GetDescriptorImageInfo(skyboxLoc);

		//i dont even know if the engine will work if this isnt constructed
		ewEngine.objectManager.skybox = { Basic_Model::createSkyBox(100.f), skyboxID };

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

		for (int i = 0; i < lightColors.size(); i++) {
			ewEngine.objectManager.pointLights.push_back(PointLight::makePointLight(5.0f, 0.1f, lightColors[i]));
			//pointLight.color = lightColors[i];
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
	}
	void EWESample::addModulesToMenuManager(float screenWidth, float screenHeight) {
		menuManager.menuModules.emplace(menu_main, std::make_unique<MainMenuMM>(screenWidth, screenHeight));
		menuManager.menuModules.at(menu_main)->labels[1].string = "1.0.0";
		//menuManager.menuModules.emplace(menu_ShaderGen, std::make_unique<ShaderGenerationMM>(windowPtr, screenWidth, screenHeight));
		//Shader::InputBox::giveGLFWCallbacks(MenuManager::staticMouseCallback, MenuManager::staticKeyCallback);
	}

	bool EWESample::processClick() {

		bool wantsToChangeScene = false;
		auto& clickReturns = MenuModule::clickReturns;
		//while (MenuModule::clickReturns.size() > 0) {
		if (clickReturns.size() == 0) {
			return false;
		}
		soundEngine->playEffect(0);
		uint16_t processMCR = clickReturns.front();
		while (clickReturns.size() > 0) {
			clickReturns.pop();
		}

		switch (processMCR) {
			case MCR_ExitProgram: {
				currentScene = scene_exitting;
				wantsToChangeScene = true;
				break;
			}
			case MCR_DiscardReturn: {
				printf("Discard returning \n");
				if (currentScene == scene_mainmenu) {
					printf("pre main menu change menu state \n");
					menuManager.changeMenuState(menu_main);
				}
				else {
					printf("game state in discard returrn? : %d \n", currentScene);
				}
				break;
			}
			case MCR_SaveReturn: {
				printf("save returning \n");
				//MenuStates currentMenuState = uiHandler.getCurrentState();
				if (menuManager.currentMenuState == menu_graphics_settings) {
					if (SettingsJSON::tempSettings.screenDimensions != SettingsJSON::settingsData.screenDimensions) {
						std::cout << "window is resizing : " << SettingsJSON::tempSettings.screenDimensions << " : " << SettingsJSON::settingsData.screenDimensions << std::endl;
						menuManager.windowResize(SettingsJSON::tempSettings.getDimensions());
					}
					SettingsJSON::settingsData = SettingsJSON::tempSettings;
					SettingsJSON::saveToJsonFile();

					if (SettingsJSON::settingsData.FPS == 0) {
						renderRefreshRate = .00000001;
					}
					else {
						renderRefreshRate = 1.0 / static_cast<double>(SettingsJSON::settingsData.FPS);
					}
					ewEngine.pointLightsEnabled = SettingsJSON::settingsData.pointLights;
					if (!ewEngine.pointLightsEnabled) {
						ewEngine.lbo.numLights = 0;
					}
					ewEngine.displayingRenderInfo = SettingsJSON::settingsData.renderInfo;
					ewEngine.mainWindow.updateSettings();
					printf("new camera perspective on graphics save \n");
				}

				if (currentScene == scene_mainmenu) {
					menuManager.changeMenuState(menu_main);
				}
	#if 0
				else if (currentScene == scene_other) {
					//if you'd like multiple return destionations from the graphics menu, set them up here
				}
	#endif
				else {
					printf("game state on save return? : %d \n", currentScene);
				}
				break;
			}
			case MCR_swapToShaderGen: {
				currentScene = scene_shaderGen;
				wantsToChangeScene = true;
				break;
			}
			case MCR_swapToMainMenu: {
				currentScene = scene_mainmenu;
				wantsToChangeScene = true;
				break;
			}
			case MCR_none: {
				printf("returned MCR_Return \n");
				throw std::runtime_error("this should nto be returned");
				break;
			}
			default: {
				//this should be natrual? idk
				//printf("why default click : %d \n", processMCR);
			}
		}
		return wantsToChangeScene;
	}
}