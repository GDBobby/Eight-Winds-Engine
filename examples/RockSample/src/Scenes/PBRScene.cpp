#include "PBRScene.h"


namespace EWE {

	PBRScene::PBRScene(EightWindsEngine& ewEngine)
		: ewEngine{ ewEngine },
		menuManager{ ewEngine.menuManager },
		soundEngine{ SoundEngine::GetSoundEngineInstance() },
		windowPtr{ ewEngine.mainWindow.getGLFWwindow() },
		camControl{ ewEngine.mainWindow.getGLFWwindow() },
		imguiHandler{ ewEngine.mainWindow.getGLFWwindow(), MAX_FRAMES_IN_FLIGHT }
	{}

	PBRScene::~PBRScene() {
#if DECONSTRUCTION_DEBUG
		printf("deconstructing main menu scene \n");
#endif
	}
	void PBRScene::Exit() {
		assert(sphereModel != nullptr);
		Deconstruct(sphereModel);
		sphereModel = nullptr;
	}
	void PBRScene::Load() {
		menuManager.giveMenuFocus();
		assert(sphereModel == nullptr);

		sphereModel = Basic_Model::Sphere(1, 1.f);

		MaterialInfo matInfo;
		matInfo.imageID = IMAGE_INVALID;
		matInfo.materialFlags = Material::Flags::Other::Instanced;
		controlledSphere.drawable = &sphereDrawable;
		sphereTransform.translation = glm::vec3(0.f, -2.f, 9.f);
		controlledSphere.ownerTransform = &sphereTransform;
		controlledSphere.meshPtr = sphereModel;

		csmEWEBuffer = {
			Construct<EWEBuffer>({sizeof(MaterialBuffer), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}),
			Construct<EWEBuffer>({sizeof(MaterialBuffer), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT})
		};

		//RigidRenderingSystem::AddMaterialObject(matInfo, matObjInfo);
		RigidRenderingSystem::AddInstancedMaterialObject(matInfo, sphereModel, 16, false);
		matInfo.materialFlags = Material::Flags::GenerateNormals;
		RigidRenderingSystem::AddMaterialObject(matInfo, controlledSphere, csmEWEBuffer);
		matInfo.materialFlags = 0;
		RigidRenderingSystem::AddMaterialObject(matInfo, controlledSphere, csmEWEBuffer);

		const std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> materialBuffers = RigidRenderingSystem::GetBothMaterialBuffers(sphereModel);
		const std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> transformBuffers = RigidRenderingSystem::GetBothTransformBuffers(sphereModel);

		transformBuffers[0]->Map();
		transformBuffers[1]->Map();
		void* mappedTransforms[2] = { transformBuffers[0]->GetMappedMemory(), transformBuffers[1]->GetMappedMemory() };
		uint64_t mappedTransformAddr[2] = {reinterpret_cast<uint64_t>(mappedTransforms[0]), reinterpret_cast<uint64_t>(mappedTransforms[1])};

		TransformComponent transform{};
		const glm::vec3 baseAlbedo{ 0.41f, 0.249f, 0.f };
		std::vector<MaterialBuffer> matData(16);

		glm::mat4 tempMat4;
		for (uint8_t x = 0; x < 4; x++) {
			for (uint8_t y = 0; y < 4; y++) {
				matData[y + x * 4].albedo = baseAlbedo;
				matData[y + x * 4].rough = x / 3.f;
				matData[y + x * 4].metal = y / 3.f;
				transform.translation.x = -7.f + 4.f * x;
				transform.translation.z = -7.f + 4.f * y;

				tempMat4 = transform.mat4();

				memcpy(reinterpret_cast<void*>(mappedTransformAddr[0] + (sizeof(glm::mat4) * (y + x * 4))), &tempMat4, sizeof(glm::mat4));
				memcpy(reinterpret_cast<void*>(mappedTransformAddr[1] + (sizeof(glm::mat4) * (y + x * 4))), &tempMat4, sizeof(glm::mat4));
			}
		}
		transformBuffers[0]->Flush();
		transformBuffers[0]->Unmap();
		transformBuffers[1]->Flush();
		transformBuffers[1]->Unmap();

		StagingBuffer* stagingBuffer = Construct<StagingBuffer>({16 * sizeof(MaterialBuffer), matData.data()});

		auto& cmdBuf = SyncHub::GetSyncHubInstance()->BeginSingleTimeCommandTransfer();

		auto matBuffers = RigidRenderingSystem::GetBothMaterialBuffers(sphereModel);

		VK::CopyBuffer(cmdBuf, stagingBuffer->buffer, matBuffers[0]->GetBuffer(), 16 * sizeof(MaterialBuffer));
		VK::CopyBuffer(cmdBuf, stagingBuffer->buffer, matBuffers[1]->GetBuffer(), 16 * sizeof(MaterialBuffer));

		TransferCommand transferCommand{};
		transferCommand.commands.push_back(&cmdBuf);
		transferCommand.stagingBuffers.push_back(stagingBuffer);

		SyncHub::GetSyncHubInstance()->EndSingleTimeCommandTransfer(transferCommand);
	
		lbo.ambientColor = glm::vec4(0.04f);
		lbo.numLights = 0;
		lbo.sunlightColor = glm::vec4(1.f);
		lbo.sunlightDirection = glm::normalize(glm::vec4(1.f));

		updatedLBO = MAX_FRAMES_IN_FLIGHT;

		updatedCMB = MAX_FRAMES_IN_FLIGHT;
		controlledSphereMB.albedo = glm::vec3(1.f);
		controlledSphereMB.metal = 0.f;
		controlledSphereMB.rough = 0.f;

		camTransform.translation = glm::vec3(-1.5f, -7.5f, 9.f);
	}

	void PBRScene::Entry() {
		soundEngine->StopMusic();

		menuManager.ChangeMenuState(menu_main, 0);
		ewEngine.camera.SetPerspectiveProjection(glm::radians(70.0f), ewEngine.eweRenderer.GetAspectRatio(), 0.1f, 1000000.0f);

		ewEngine.camera.UpdateViewData({ 40.f, 0.f, 40.0f }, { 0.f, 0.f, 0.f });

		glfwSetMouseButtonCallback(windowPtr, ImGui_ImplGlfw_MouseButtonCallback);
		glfwSetKeyCallback(windowPtr, ImGui_ImplGlfw_KeyCallback);
	}

	void PBRScene::RenderLBOControls(){
		
		if (ImGui::Begin("light control")) {
			bool lboChanged = false;
			lboChanged |= ImGui::ColorPicker4("ambient", reinterpret_cast<float*>(&lbo.ambientColor), ImGuiColorEditFlags_Float);
			lboChanged |= ImGui::DragFloat3("sunlight direction", reinterpret_cast<float*>(&lbo.sunlightDirection), 0.01f, -1.f, 1.f);
			lboChanged |= ImGui::DragFloat("sun power", &lbo.sunlightDirection.w, 0.01f, 0.f, 100.f);
			lboChanged |= ImGui::ColorPicker4("sunlight color", reinterpret_cast<float*>(&lbo.sunlightColor), ImGuiColorEditFlags_Float);
			lboChanged |= ImGui::DragInt("num lights (not implemented)", reinterpret_cast<int*>(&lbo.numLights), 1.f, 0, 10);

			if (lboChanged) {
				updatedLBO = MAX_FRAMES_IN_FLIGHT;
				glm::vec3 sunDir;
				sunDir.x = lbo.sunlightDirection.x;
				sunDir.y = lbo.sunlightDirection.y;
				sunDir.z = lbo.sunlightDirection.z;
				sunDir = glm::normalize(sunDir);

				lbo.sunlightDirection.x = sunDir.x;
				lbo.sunlightDirection.y = sunDir.y;
				lbo.sunlightDirection.z = sunDir.z;
			}
		}
		ImGui::End();
	}

	void PBRScene::RenderCameraData() {
		if (ImGui::Begin("camera data")) {
			ImGui::Text("camera translation - %.2f:%.2f:%.2f\n", camTransform.translation.x, camTransform.translation.y, camTransform.translation.z);
			ImGui::Text("camera rotation - %.2f:%.2f:%.2f\n", camTransform.rotation.x, camTransform.rotation.y, camTransform.rotation.z);
		}
		ImGui::End();
	}
	void PBRScene::RenderControlledSphereControls() {
		if (ImGui::Begin("controlled sphere material attributes")) {
			ImGui::DragFloat3("translation##cmb", reinterpret_cast<float*>(&sphereTransform.translation), 0.1f);
			ImGui::DragFloat3("rotation##cmb", reinterpret_cast<float*>(&sphereTransform.rotation), 0.01f);
			ImGui::DragFloat3("scale##cmb", reinterpret_cast<float*>(&sphereTransform.scale), 0.1f);

			bool cmbChange = false;
			cmbChange |= ImGui::DragFloat("metal##cmb", &controlledSphereMB.metal, 0.01f, 0.f, 1.f);
			cmbChange |= ImGui::DragFloat("rough##cmb", &controlledSphereMB.rough, 0.01f, 0.f, 1.f);
			cmbChange |= ImGui::ColorPicker3("albedo##cmb", reinterpret_cast<float*>(&controlledSphereMB.albedo), ImGuiColorEditFlags_Float);

			if (cmbChange) {
				updatedCMB = MAX_FRAMES_IN_FLIGHT;
			}

		}
		ImGui::End();
	}

	bool PBRScene::Render(double dt) {
		//printf("render main menu scene \n");
		//if (!paused && (glfwGetKey(windowPtr, GLFW_KEY_P) == GLFW_PRESS)) {
		//	paused = true;
		//}
		//if (paused && (glfwGetKey(windowPtr, GLFW_KEY_U) == GLFW_PRESS)) {
		//	paused = false;
		//}
		if (updatedLBO > 0) {
			DescriptorHandler::WriteToLightBuffer(lbo);
			updatedLBO--;
		}
		if (updatedCMB) {
			csmEWEBuffer[VK::Object->frameIndex]->Map();
			void* mappedMem = csmEWEBuffer[VK::Object->frameIndex]->GetMappedMemory();
			memcpy(mappedMem, &controlledSphereMB, sizeof(MaterialBuffer));

			csmEWEBuffer[VK::Object->frameIndex]->Flush();
			csmEWEBuffer[VK::Object->frameIndex]->Unmap();

			--updatedCMB;
		}

		camControl.Move(camTransform);
		camControl.RotateCam(camTransform);
		camControl.Zoom(camTransform);
		ewEngine.camera.SetViewYXZ(camTransform.translation, camTransform.rotation);

		if (ewEngine.BeginFrame()) {
			ewEngine.camera.BindUBO();
			ewEngine.BeginRenderX();
			ewEngine.DrawObjects(dt);

			imguiHandler.beginRender();
			RenderLBOControls();
			RenderCameraData();
			RenderControlledSphereControls();
			imguiHandler.endRender();

			//rockSystem.Render();
			//printf("after displaying render info \n");
			ewEngine.EndRender();
			ewEngine.EndFrame();
			//std::cout << "after ending render \n";
			return false;
		}
		return true;
	}
}