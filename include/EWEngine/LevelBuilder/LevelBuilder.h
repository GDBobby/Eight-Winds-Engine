/*
#pragma once

#include "EWEngine/Graphics/imGuiHandler.h"
//#include "EWE_model.h"
#include "../freeCameraController.h"
#include "../ObjectManager.h"
#include "EWEngine/Graphics/EWE_camera.h"
#include "EweLevelLoader.h"

#include "EWEngine/Graphics/EWE_frame_info.h"

namespace EWE {
	class BuilderObjects {

	};


	class LevelBuilder {
	public:
		LevelBuilder(ImGUIHandler* ImGuiHandler, GLFWwindow* window, ObjectManager* objMan, EWEGameObject* cameraObj, EWECamera* EWECamera, 
		LightBufferObject *lbo, bool* shouldRenderPoints);

		static void LBMouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void LBKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

		//this needs to be called from the main thread, aka eightwinds::run()
		void setGLFWCallbacks(GLFWwindow* windowPtr) {
			//glfwFocusWindow(windowPtr);
			glfwSetKeyCallback(windowPtr, LBKeyCallback);
			glfwSetMouseButtonCallback(windowPtr, LBMouseCallback);
		}

		void render(float dt);
		bool shouldClose() { return closePlease; }
		bool ReturnToMain() { 
			if (returnToMain) {
				returnToMain = false;
				return true;
			}
			return false;
		}

		void CleanUp();

		void postRender() {
			if (wantsToResetLevel || wantsToLoadLevel) {
				vkDeviceWaitIdle(EWEDevice::GetVkDevice());
			}
			destroyObjects();
			loadLevel();
			resetBuilder();
		}

		bool objectAdded() {
			if (quadWasAdded) {
				quadWasAdded = false;
				return true;
			}
			return false;
		}

	private:
		std::pair<int16_t, int32_t> lastTextureID = { -1,-1 };

		const float oneDegree = 0.0174533f;
		const float tenDegrees = 0.174533f;
		LightBufferObject* lbo;
		bool showLBOControls = false;
		bool* shouldRenderPoints;

		ImGUIHandler* imguiHandler;
		CameraController cameraControl;
		std::shared_ptr<EWEModel> floorGridModel;
		ObjectManager* objectManager;
		

		EWEGameObject* cameraObject;
		EWECamera* camera;

		TransformComponent placeTransform;
		float sunColor[3];

		bool showAddQuad = false;
		bool showObjectList = false;
		bool showObjectControl = false;
		uint32_t selectedObject = 6942069;
		uint32_t spawnObjectID = 6942070;
		uint32_t gridObjectID = 6942071;
		uint32_t targetTimer = 30;
		uint32_t targetStepSmall = 1;
		uint32_t targetStepLarge = 10;

		std::map<uint32_t, std::map<uint32_t, EWEGameObject>*> objectList;
		//void deleteObject();
		//void addObject();

		//std::string targetPath = "";
		//char targetLocation[128] = "";

		bool returnToMain = false;
		bool closePlease = false;
		bool newLevelPrompt = false;
		bool saveLevelPrompt = false;
		bool loadLevelPrompt = false;
		bool targetControlScheme = false;
		bool newUVFocus = true;

		float uvScaleX = 1.f;
		float uvScaleY = 1.f;
		std::vector<bobAVertexNT> changingSimpleVertices;
		std::vector<bobAVertex> changingMateriaEWErtices;
		BuilderModel::VertexType changingVertexType = BuilderModel::vT_NT;

		uint32_t targetCount = 0;
		uint32_t objectCounter = 0;
		char targetLocation[MAX_LB_PATH] = "";
		char saveLocation[MAX_LB_PATH] = "";
		char materialLocation[MAX_LB_PATH] = "blenderObstacle/hazard";
		char genericBuffer[MAX_LB_PATH] = "";

		void addMainControls();

		std::shared_ptr<RigidRenderingSystem> materialHandler;

		//these need to be one outside of the rendering process, aka before beginFrame(), or after endframe()
		bool quadWasAdded = false; 
		bool wantsToLoadLevel = false;
		bool wantsToResetLevel = false;
		bool wantsToDestroy = false;


		void destroyObjects() {
			if (wantsToDestroy) {
				if (((BuilderModel*)objectList[selectedObject]->at(selectedObject).model.get())->ReadyForDeletion()) {
					printf("destroy? \n");
					vkDeviceWaitIdle(EWEDevice::GetVkDevice());
					materialHandler->removeByTransform(objectList[selectedObject]->at(selectedObject).textureID, &objectList[selectedObject]->at(selectedObject).transform);
					objectList[selectedObject]->erase(selectedObject);
					objectList.erase(selectedObject);
					selectedObject = 6942069;
					wantsToDestroy = false;
				}
			}
		}
		void resetBuilder() {
			if (wantsToResetLevel) {
				objectList.clear();
				objectManager->resetBuilders();
				printf("builder reset \n");
				objectCounter = 0;
				wantsToResetLevel = false;
			}
		}

		void loadLevel() {
			if (wantsToLoadLevel) {
				objectList.clear();
				objectManager->resetBuilders();
				printf("Level Loaded \n");
				objectCounter = LevelManager::loadLevelToBuilder(eweDevice, saveLocation, objectManager, objectList, *lbo, LevelExportData::builder, objectManager->spriteBuildObjects[0].transform, targetLocation, targetTimer);
				for (auto iter = objectList.begin(); iter != objectList.end(); iter++) {
					printf("adding object to materialhandler \n");
					materialHandler->addMaterialObject(iter->second->at(iter->first).textureFlags, MOI_none, &iter->second->at(iter->first).transform, iter->second->at(iter->first).model.get(), iter->second->at(iter->first).textureID, &iter->second->at(iter->first).drawable);
				}
				wantsToLoadLevel = false;
				quadWasAdded = objectCounter > 0;
			}
		}
	};
}
*/