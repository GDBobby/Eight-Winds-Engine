#include "PBRScene.h"

#include <numeric>
#include <algorithm>
#include <random>


#include "../Pipelines/PipeEnum.h"

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

		if (tessBuffer[0] != nullptr) {
			Deconstruct(tessBuffer[0]);
			Deconstruct(tessBuffer[1]);
		}
		if (terrainDesc[0] != VK_NULL_HANDLE) {
			auto* dsl = PipelineSystem::At(Pipe::ENGINE_MAX_COUNT)->GetDSL();
			EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, dsl, &terrainDesc[0]);
			EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, dsl, &terrainDesc[1]);
		}

		if(perlinNoiseImage != VK_NULL_HANDLE){
			EWE_VK(vkDestroyImage, VK::Object->vkDevice, perlinNoiseImage, nullptr);
			perlinNoiseImage = VK_NULL_HANDLE;

			Sampler::RemoveSampler(perlinNoiseSampler);
			perlinNoiseSampler = VK_NULL_HANDLE;

			EWE_VK(vkFreeMemory, VK::Object->vkDevice, perlinNoiseImageMemory, nullptr);
			perlinNoiseImageMemory = VK_NULL_HANDLE;

			EWE_VK(vkDestroyImageView, VK::Object->vkDevice, perlinNoiseImageView, nullptr);
			perlinNoiseImageView = VK_NULL_HANDLE;

			EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, perlinGenDSL, &perlinDesc[0]);
			EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, perlinGenDSL, &perlinDesc[1]);
			perlinDesc[0] = VK_NULL_HANDLE;
			perlinDesc[1] = VK_NULL_HANDLE;
			Deconstruct(perlinGenDSL);
			perlinGenDSL = nullptr;
		}
	}

	void PBRScene::InitTerrainResources(){

		for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
			tessBuffer[i] = Construct<EWEBuffer>({sizeof(TessBufferObject), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT});

			tessBuffer[i]->Map();
			tessBuffer[i]->WriteToBuffer(&tbo, sizeof(TessBufferObject));
			tessBuffer[i]->Flush();
			tessBuffer[i]->Unmap();
		}

		for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
			EWEDescriptorWriter descWriter(PipelineSystem::At(Pipe::Terrain)->GetDSL(), DescriptorPool_Global);
			DescriptorHandler::AddGlobalsToDescriptor(descWriter, i);
			descWriter.WriteBuffer(tessBuffer[i]->DescriptorInfo());
			terrainDesc[i] = descWriter.Build();
		}
	}

	void PBRScene::InitGrassResources() {

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			grassBuffer[i] = Construct<EWEBuffer>({ sizeof(GrassBufferObject), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT });


			EWEDescriptorWriter descWriter(PipelineSystem::At(Pipe::GenGrass)->GetDSL(), DescriptorPool_Global);
			DescriptorHandler::AddGlobalsToDescriptor(descWriter, i);
			descWriter.WriteBuffer(grassBuffer[i]->DescriptorInfo());
			grassDesc[i] = descWriter.Build();
		}
	}

	void PBRScene::InitPerlinNoiseResources() {

		const VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
		VkFormatProperties formatProperties;
		// Get device properties for the requested texture format
		EWE_VK(vkGetPhysicalDeviceFormatProperties, VK::Object->physicalDevice, format, &formatProperties);
		// Check if requested image format supports image storage operations required for storing pixel from the compute shader
		assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
		
		const uint32_t perlinExtent = 256;

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;

		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent = { perlinExtent, perlinExtent, 1 };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		imageCreateInfo.flags = 0;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		
		uint32_t queueData[] = { static_cast<uint32_t>(VK::Object->queueIndex[Queue::graphics]), static_cast<uint32_t>(VK::Object->queueIndex[Queue::present])};
		const bool differentFamilies = (queueData[0] != queueData[1]);
		imageCreateInfo.sharingMode = (VkSharingMode)differentFamilies;
		imageCreateInfo.queueFamilyIndexCount = 1 + differentFamilies;
		imageCreateInfo.pQueueFamilyIndices = queueData;
		Image::CreateImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, perlinNoiseImage, perlinNoiseImageMemory);

		SyncHub* syncHub = SyncHub::GetSyncHubInstance();
		//directly to graphics because no data is being uploaded
		CommandBuffer& cmdBuf = syncHub->BeginSingleTimeCommandGraphics();

		VkImageMemoryBarrier imageBarrier = Barrier::TransitionImageLayout(perlinNoiseImage,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
			1, 1
		);
		
		EWE_VK(vkCmdPipelineBarrier, cmdBuf,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, //i get the feeling this is suboptimal, but this is what sascha does and i haven't found an alternative
			0,
			0, nullptr,
			0, nullptr,
			1, &imageBarrier
		);			
		GraphicsCommand gCommand{};
		gCommand.command = &cmdBuf;
		syncHub->EndSingleTimeCommandGraphics(gCommand);


		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		perlinNoiseSampler = Sampler::GetSampler(samplerInfo);


		VkImageViewCreateInfo view{};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.pNext = nullptr;
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.format = format;
		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view.subresourceRange.baseMipLevel = 0;
		view.subresourceRange.levelCount = 1;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount = 1;
		view.image = perlinNoiseImage;
		EWE_VK(vkCreateImageView, VK::Object->vkDevice, &view, nullptr, &perlinNoiseImageView);

		perlinComputeImgInfo.imageView = perlinNoiseImageView;
		perlinGraphicsImgInfo.imageView = perlinNoiseImageView;

		perlinComputeImgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		perlinComputeImgInfo.sampler = perlinNoiseSampler;
		
		perlinGraphicsImgInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		perlinGraphicsImgInfo.sampler = perlinNoiseSampler;

	}

	void PBRScene::InitSphereMaterialResources(){
		assert(sphereModel == nullptr);
		sphereModel = Basic_Model::Sphere(0, 1.f);

		controlledSphere.drawable = &sphereDrawable;
		sphereTransform.translation = glm::vec3(0.f, -2.f, 9.f);
		controlledSphere.ownerTransform = &sphereTransform;
		controlledSphere.meshPtr = sphereModel;

		csmEWEBuffer = {
			Construct<EWEBuffer>({sizeof(MaterialBuffer), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}),
			Construct<EWEBuffer>({sizeof(MaterialBuffer), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT})
		};

		MaterialInfo matInfo;
		matInfo.imageID = IMAGE_INVALID;
		matInfo.materialFlags = Material::Flags::Other::Instanced;
		RigidRenderingSystem::AddInstancedMaterialObject(matInfo, sphereModel, 16, false);
		const std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> materialBuffers = RigidRenderingSystem::GetBothMaterialBuffers(matInfo.materialFlags, sphereModel);
		const std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> transformBuffers = RigidRenderingSystem::GetBothTransformBuffers(matInfo.materialFlags, sphereModel);
#if DEBUGGING_MATERIAL_NORMALS
		matInfo.materialFlags |= Material::Flags::GenerateNormals;
		RigidRenderingSystem::AddInstancedMaterialObject(matInfo, sphereModel, 16, false);
		const std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> gn_transformBuffers = RigidRenderingSystem::GetBothTransformBuffers(matInfo.materialFlags, sphereModel);

		matInfo.materialFlags = Material::Flags::GenerateNormals;
		RigidRenderingSystem::AddMaterialObject(matInfo, controlledSphere, csmEWEBuffer);
#endif
		matInfo.materialFlags = 0;
		groundModel = Basic_Model::Grid3DQuadPrimitive(100);
		//controlledSphere.meshPtr = groundModel;
		RigidRenderingSystem::AddMaterialObject(matInfo, controlledSphere, csmEWEBuffer);

		//RigidRenderingSystem::AddMaterialObject(matInfo, controlledSphere


		transformBuffers[0]->Map();
		transformBuffers[1]->Map();
#if DEBUGGING_MATERIAL_NORMALS
		gn_transformBuffers[0]->Map();
		gn_transformBuffers[1]->Map();
		uint64_t mappedTransformAddr[4] = {reinterpret_cast<uint64_t>(transformBuffers[0]->GetMappedMemory()), reinterpret_cast<uint64_t>(transformBuffers[1]->GetMappedMemory()), reinterpret_cast<uint64_t>(gn_transformBuffers[0]->GetMappedMemory()), reinterpret_cast<uint64_t>(gn_transformBuffers[1]->GetMappedMemory())
		};
#else
		uint64_t mappedTransformAddr[2] = { reinterpret_cast<uint64_t>(transformBuffers[0]->GetMappedMemory()), reinterpret_cast<uint64_t>(transformBuffers[1]->GetMappedMemory()) };
#endif

		TransformComponent transform{};
		const glm::vec3 baseAlbedo{ 0.41f, 0.249f, 0.f };
		std::vector<MaterialBuffer> matData(16);

		glm::mat4 tempMat4;
		for (uint8_t x = 0; x < 4; x++) {
			for (uint8_t y = 0; y < 4; y++) {
				matData[y + x * 4].albedo = baseAlbedo;
				matData[y + x * 4].rough = 1.f - x / 3.f;
				matData[y + x * 4].metal = 1.f - y / 3.f;
				transform.translation.x = -7.f + 4.f * x;
				transform.translation.z = -7.f + 4.f * y;

				tempMat4 = transform.mat4();

				memcpy(reinterpret_cast<void*>(mappedTransformAddr[0] + (sizeof(glm::mat4) * (y + x * 4))), &tempMat4, sizeof(glm::mat4));
				memcpy(reinterpret_cast<void*>(mappedTransformAddr[1] + (sizeof(glm::mat4) * (y + x * 4))), &tempMat4, sizeof(glm::mat4));
#if DEBUGGING_MATERIAL_NORMALS
				memcpy(reinterpret_cast<void*>(mappedTransformAddr[2] + (sizeof(glm::mat4) * (y + x * 4))), &tempMat4, sizeof(glm::mat4));
				memcpy(reinterpret_cast<void*>(mappedTransformAddr[3] + (sizeof(glm::mat4) * (y + x * 4))), &tempMat4, sizeof(glm::mat4));
#endif
			}
		}
		transformBuffers[0]->Flush();
		transformBuffers[0]->Unmap();
		transformBuffers[1]->Flush();
		transformBuffers[1]->Unmap();
#if DEBUGGING_MATERIAL_NORMALS
		gn_transformBuffers[0]->Flush();
		gn_transformBuffers[0]->Unmap();
		gn_transformBuffers[1]->Flush();
		gn_transformBuffers[1]->Unmap();
#endif

		StagingBuffer* stagingBuffer = Construct<StagingBuffer>({16 * sizeof(MaterialBuffer), matData.data()});

		auto& cmdBuf = SyncHub::GetSyncHubInstance()->BeginSingleTimeCommandTransfer();

		VK::CopyBuffer(cmdBuf, stagingBuffer->buffer, materialBuffers[0]->GetBuffer(), 16 * sizeof(MaterialBuffer));
		VK::CopyBuffer(cmdBuf, stagingBuffer->buffer, materialBuffers[1]->GetBuffer(), 16 * sizeof(MaterialBuffer));

		TransferCommand transferCommand{};
		transferCommand.commands.push_back(&cmdBuf);
		transferCommand.stagingBuffers.push_back(stagingBuffer);

		SyncHub::GetSyncHubInstance()->EndSingleTimeCommandTransfer(transferCommand);

		updatedCMB = MAX_FRAMES_IN_FLIGHT;
		controlledSphereMB.albedo = glm::vec3(1.f);
		controlledSphereMB.metal = 0.f;
		controlledSphereMB.rough = 0.f;
	}

	void PBRScene::Load() {
		menuManager.giveMenuFocus();
		InitSphereMaterialResources();
		InitTerrainResources();
		InitGrassResources();
	
		lbo.ambientColor = glm::vec4(0.04f);
		lbo.numLights = 0;
		lbo.sunlightColor = glm::vec4(1.f);
		lbo.sunlightDirection = glm::normalize(glm::vec4(1.f));

		updatedLBO = MAX_FRAMES_IN_FLIGHT;

		camTransform.translation = glm::vec3(-1.5f, -7.5f, 9.f);

		tbo.proj = ewEngine.camera.GetProjection();
		tbo.displacementFactor = 32.f;
		tbo.tessFactor = 0.75f;
		tbo.tessEdgeSize = 20.f;

		gbo.animationScale = 1.f;
		gbo.endDistance = 10000.f;
		gbo.height = 1.f;
		gbo.lengthGroundPosV2 = 1.f;
		gbo.spacing = 0.1f;
		gbo.windDir = 0.f;
		gbo.time = 0.f;

		//updatedTBO = MAX_FRAMES_IN_FLIGHT;
	
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

	void PBRScene::RenderTerrainControls() {
		if (ImGui::Begin("terrain data")) {

			ImGui::Checkbox("active##ter", &terrainActive);
			ImGui::DragFloat("displacement factor", &tbo.displacementFactor, 1.f, 0.f, 1000.f);
			ImGui::DragFloat("tessellation factor", &tbo.tessFactor, 0.01f, 0.f, 10.f);
			ImGui::DragFloat("tessellation edge size", &tbo.tessEdgeSize, 0.1f, 0.1f, 100.f);
			ImGui::SliderInt("octaves", &tbo.octaves, 1, 8);
#if EWE_DEBUG
			ImGui::Checkbox("wireframe", &terrainWire);
#endif
		}
		ImGui::End();
	}
	void PBRScene::RenderGrassControls() {
		if (ImGui::Begin("grass data")) {
			ImGui::DragFloat("animation scale", &gbo.animationScale, 0.1f, -100.f, 100.f);
			ImGui::DragFloat("end distance", &gbo.endDistance, 1.f, 0.f, 10000.f);
			ImGui::DragFloat("height&gr", &gbo.height, 0.01f, 0.f, 100.f);
			ImGui::DragFloat("length ground posv2", &gbo.lengthGroundPosV2, 0.01f, 0.f, 100.f);
			ImGui::DragFloat("spacing", &gbo.spacing, 0.01f, 0.f, 100.f);
			ImGui::DragFloat("wind dir", &gbo.windDir, 0.01f, 0.f, glm::two_pi<float>());
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
		{
			tessBuffer[VK::Object->frameIndex]->Map();
			void* mappedMem = tessBuffer[VK::Object->frameIndex]->GetMappedMemory();
			tbo.view = ewEngine.camera.GetView();
			memcpy(mappedMem, &tbo, sizeof(TessBufferObject));
			tessBuffer[VK::Object->frameIndex]->Flush();
			tessBuffer[VK::Object->frameIndex]->Unmap();
		}
		{
			grassBuffer[VK::Object->frameIndex]->Map();
			void* mappedMem = grassBuffer[VK::Object->frameIndex]->GetMappedMemory();
			gbo.time += static_cast<float>(dt);
			memcpy(mappedMem, &gbo, sizeof(GrassBufferObject));
			grassBuffer[VK::Object->frameIndex]->Flush();
			grassBuffer[VK::Object->frameIndex]->Unmap();
		}

		camControl.Move(camTransform);
		camControl.RotateCam(camTransform);
		camControl.Zoom(camTransform);
		ewEngine.camera.SetViewYXZ(camTransform.translation, camTransform.rotation);

		if (ewEngine.BeginFrame()) {
			ewEngine.camera.BindUBO();
			const auto tempFrustumCopy = ewEngine.camera.GetFrustumPlanes();
			for (uint8_t i = 0; i < 6; i++) {
				tbo.frustumPlanes[i] = tempFrustumCopy[i];
			}
			tbo.viewportDim = glm::vec2{VK::Object->screenWidth, VK::Object->screenHeight};

			ewEngine.BeginRenderX();

			ewEngine.Draw3DObjects(dt);

			if (terrainActive) {
				PipelineSystem* pipe;
				if (terrainWire) {
					pipe = PipelineSystem::At(Pipe::TerrainWM);//terrain pipe. i should just make an enum but if this is the only pipe its not a big deal
				}
				else {
					pipe = PipelineSystem::At(Pipe::Terrain);//terrain pipe. i should just make an enum but if this is the only pipe its not a big deal
				}
				pipe->BindPipeline();
				pipe->BindDescriptor(0, &terrainDesc[VK::Object->frameIndex]);
				pipe->BindModel(groundModel);
				pipe->DrawModel();
			}

			if (VK::CmdDrawMeshTasksEXT != VK_NULL_HANDLE) {
				if (grassActive) {
					PipelineSystem* pipe = PipelineSystem::At(Pipe::GenGrass);
					pipe->BindPipeline();
					pipe->BindDescriptor(0, &grassDesc[VK::Object->frameIndex]);
					//EWE_VK(VK::CmdDrawMeshTasksEXT, VK::Object->GetFrameBuffer(), 1, 0, 0);
					VK::CmdDrawMeshTasksEXT(VK::Object->GetFrameBuffer().cmdBuf, 1, 0, 0);
				}
			}

			ewEngine.Draw2DObjects();
			ewEngine.DrawText(dt);

			imguiHandler.beginRender();
			RenderLBOControls();
			RenderCameraData();
			RenderControlledSphereControls();
			RenderTerrainControls();
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