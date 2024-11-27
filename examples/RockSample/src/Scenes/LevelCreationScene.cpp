#include "LevelCreationScene.h"

#include "../pipelines/PipelineEnum.h"

namespace EWE {
	LevelCreationScene* LevelCreationScene::lcPtr{nullptr};

	LevelCreationScene::LevelCreationScene(EightWindsEngine& ewEngine)
		: ewEngine{ ewEngine },
		menuManager{ ewEngine.menuManager },
		soundEngine{ SoundEngine::GetSoundEngineInstance() },
		levelCreationIMGUI{MenuModule::clickReturns, menuManager.screenWidth, menuManager.screenHeight}
	{
		lcPtr = this;
	}
	LevelCreationScene::~LevelCreationScene() {
		printf("deconstructing main menu \n");
		lcPtr = nullptr;
	}

	//									   GLFWwindow* window, int button, int action, int mods
	void LevelCreationScene::mouseCallback(GLFWwindow* window, int button, int action, int mods) {


		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse) {
			lcPtr->levelCreationIMGUI.mouseCallback(button, action);
			//printf("imgui wants mouse capture \n");
			return;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				if (lcPtr->levelCreationIMGUI.selectedTool == LevelCreationIMGUI::Tool_colorSelection) {
					lcPtr->tileMapD->clearSelection();
				}

				glfwGetCursorPos(window, &lcPtr->currentMouseXPos, &lcPtr->currentMouseYPos);
				glfwSetCursorPosCallback(lcPtr->ewEngine.mainWindow.getGLFWwindow(), LevelCreationScene::rightClickDragCallback);
			}
			else if (action == GLFW_RELEASE) {
				glfwSetCursorPosCallback(lcPtr->ewEngine.mainWindow.getGLFWwindow(), ImGui_ImplGlfw_CursorPosCallback);
			}
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS) {
				if (lcPtr->tileMapD.get() != nullptr) {
					glfwGetCursorPos(window, &lcPtr->currentMouseXPos, &lcPtr->currentMouseYPos);
					printf("cursor pos : %d:%d \n", static_cast<int>(lcPtr->currentMouseXPos), static_cast<int>(lcPtr->currentMouseYPos));
					//lcPtr->tileMapD->getScreenCoordinates(lcPtr->menuManager.screenWidth, lcPtr->menuManager.screenHeight);
					lcPtr->lastTileDragPos = lcPtr->tileMapD->getClickedTile(static_cast<int>(lcPtr->currentMouseXPos), static_cast<int>(lcPtr->currentMouseYPos), lcPtr->menuManager.screenWidth, lcPtr->menuManager.screenHeight);
					if (lcPtr->lastTileDragPos >= 0) {
						lcPtr->levelCreationIMGUI.toolLeft(static_cast<uint32_t>(lcPtr->lastTileDragPos), glfwGetKey(window, GLFW_KEY_LEFT_SHIFT), glfwGetKey(window, GLFW_KEY_LEFT_CONTROL));
						
					}
				}
				glfwSetCursorPosCallback(lcPtr->ewEngine.mainWindow.getGLFWwindow(), LevelCreationScene::leftClickDragCallback);
			}
			else if (action == GLFW_RELEASE) {
				glfwSetCursorPosCallback(lcPtr->ewEngine.mainWindow.getGLFWwindow(), ImGui_ImplGlfw_CursorPosCallback);
			}
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			if (action == GLFW_PRESS) {
				if (lcPtr->tileMapD.get() != nullptr) {
					printf("fitting to screen \n");
					//lcPtr->tileMapD->getScreenCoordinates(lcPtr->menuManager.screenWidth, lcPtr->menuManager.screenHeight);
					lcPtr->fitToScreen();
					//lcPtr->tileMapD->getScreenCoordinates(lcPtr->menuManager.screenWidth, lcPtr->menuManager.screenHeight);
				}
			}
		}
	}
	void LevelCreationScene::leftClickDragCallback(GLFWwindow* window, double xpos, double ypos) {
		int64_t currentPos = lcPtr->tileMapD->getClickedTile(static_cast<int>(xpos), static_cast<int>(ypos), lcPtr->menuManager.screenWidth, lcPtr->menuManager.screenHeight);
		if (currentPos >= 0 && currentPos != lcPtr->lastTileDragPos) {
			lcPtr->lastTileDragPos = currentPos;
			lcPtr->levelCreationIMGUI.toolLeft(static_cast<uint32_t>(lcPtr->lastTileDragPos), glfwGetKey(window, GLFW_KEY_LEFT_SHIFT), glfwGetKey(window, GLFW_KEY_LEFT_CONTROL));
		}
	}

	void LevelCreationScene::rightClickDragCallback(GLFWwindow* window, double xpos, double ypos) {
		ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse) {
			return;
		}
		lcPtr->pushTrans.x += static_cast<float>(lcPtr->currentMouseXPos - xpos) / (100.f * lcPtr->pushScale.y);
		lcPtr->pushTrans.y += static_cast<float>(lcPtr->currentMouseYPos - ypos) / (100.f * lcPtr->pushScale.y);
		lcPtr->currentMouseXPos = xpos;
		lcPtr->currentMouseYPos = ypos;
		lcPtr->tileMapD->refreshedMap = true;
	}

	void LevelCreationScene::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse) {
			lcPtr->levelCreationIMGUI.scrollCallback(yoffset);
			return;
		}
		//printf("scroll call back - %.5f:%.5f \n", xoffset, yoffset);
		float scaling = 1 + .25f * yoffset;
		lcPtr->pushScale *= scaling;

		lcPtr->tileMapD->pushTile.scale.x *= scaling;
		lcPtr->tileMapD->pushTile.scale.y *= scaling;
		//lcPtr->tileMapD->pushTile.scale.x *= scaling;

		lcPtr->tileMapD->refreshedMap = true;
	}
	void LevelCreationScene::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
		ImGuiIO& io = ImGui::GetIO();

		if (io.WantCaptureKeyboard) {
			//printf("imgui wants mouse capture \n");
			return;
		}
	}


	void LevelCreationScene::Load() {
		menuManager.giveMenuFocus();

		printf("after updating pipelines load menu objects, returning \n");
	}
	void LevelCreationScene::Entry() {
		soundEngine->StopMusic();

		menuManager.changeMenuState(menu_main, 0);
		menuManager.closeMenu();

		//old method
		/*
		ewEngine.camera.setViewTarget({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f }, glm::vec3(0.f, 1.f, 0.f));
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			ewEngine.camera.bindUBO(i);
		}
		*/
		//new camera method
		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			ewEngine.camera.UpdateViewData({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f }, glm::vec3(0.f, 1.f, 0.f));
		}
		imguiHandler = std::make_unique<ImGUIHandler>(ewEngine.mainWindow.getGLFWwindow(), ewEngine.eweDevice, MAX_FRAMES_IN_FLIGHT);
		//handle threads in this scene, or a game specific class
		ewEngine.advancedRS.drawSkybox = false;
		levelCreationIMGUI.gridZoom = &pushScale;
		levelCreationIMGUI.gridTrans = &pushTrans;
		levelCreationIMGUI.gridScale = &pushGridScale;
		pushScale.x = 1.f;//menuManager.screenWidth / menuManager.screenHeight;
		pushScale.y = 1.f;
		pushTrans.x = 0.f;
		pushTrans.y = 0.f;

		levelCreationIMGUI.loadTextures(ewEngine.eweDevice);

		GLFWwindow* windowPtr = ewEngine.mainWindow.getGLFWwindow();
		ImGui_ImplGlfw_RestoreCallbacks(windowPtr);
		glfwSetMouseButtonCallback(windowPtr, LevelCreationScene::mouseCallback);
		glfwSetKeyCallback(windowPtr, LevelCreationScene::keyCallback);
		glfwSetScrollCallback(windowPtr, LevelCreationScene::scrollCallback);
		printf("tileSetID : %d \n", levelCreationIMGUI.tileSetID);
		levelCreationIMGUI.createButtonPtr = createLevel;

		gridModel = Basic_Model::Generate2DGrid(ewEngine.eweDevice);

		tileMapD = std::make_unique<TileMapDevelopment>(ewEngine.eweDevice, 1, 1);
		levelCreationIMGUI.tileMapD = tileMapD.get();

		
	}
	void LevelCreationScene::Exit() {
		ewEngine.objectManager.eweObjects.clear();
		imguiHandler.reset();
		ewEngine.advancedRS.drawSkybox = true;
		glfwSetMouseButtonCallback(ewEngine.mainWindow.getGLFWwindow(), mouseReturnFunction);
		glfwSetKeyCallback(ewEngine.mainWindow.getGLFWwindow(), keyReturnFunction);
		gridModel.reset();
		//i need to give callback control back to whatever needs it after exiting the scene, something is missing
	}
	bool LevelCreationScene::Render(double dt) {
		//printf("render main menu scene \n");



		if (ewEngine.BeginRender()) {

			imguiHandler->beginRender();

			levelCreationIMGUI.render();

			tileMapD->pushTile.translation.x = 2.f * pushTrans.x;
			tileMapD->pushTile.translation.y = 2.f * pushTrans.y;

			tileMapD->renderTiles();
			renderBackgroundGrid();
			//ewEngine.drawObjects(cmdBufFrameIndex, dt);
			ewEngine.DrawText(dt);
			//printf("after displaying render info \n");

			ImGui::ShowDemoWindow(&show_demo_window);

			imguiHandler->endRender();
			ewEngine.EndRender();
			//std::cout << "after ending render \n";
			return false;
		}
		return true;
	}

	void LevelCreationScene::renderBackgroundGrid() {

		PipelineSystem::At(Pipe_Grid2d)->BindPipeline();
		Grid2DPushConstantData push;
		push.scaleOffset = { pushScale, pushTrans };
		push.color.r = 0.f;
		push.color.g = 125.6f / 255.f;
		push.color.b = 90.f / 255.f;
		push.gridScale = pushGridScale;

		PipelineSystem::At(Pipe_Grid2d)->BindModel(gridModel.get());
		PipelineSystem::At(Pipe_Grid2d)->PushAndDraw(&push);
	}

	void LevelCreationScene::createLevel(uint16_t width, uint16_t height) {
		printf("creating level, waiting for idle \n");

		vkQueueWaitIdle(VK::Object->queues[Queue::graphics]);
		lcPtr->tileMapD->refreshMap(width, height);
		lcPtr->fitToScreen();
		//lcPtr->gridModel.reset();
		//lcPtr->gridModel = Basic_Model::generate2DGrid(lcPtr->ewEngine.eweDevice);
	}
	void LevelCreationScene::fitToScreen() {
		tileMapD->fitToScreen();
		pushScale.x = 1.f;
		pushScale.y = 1.f;
		pushTrans.x = 0.f;
		pushTrans.y = 0.f;
	}
}