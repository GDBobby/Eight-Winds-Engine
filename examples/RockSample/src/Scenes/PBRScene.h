#pragma once
#include <EWEngine/EightWindsEngine.h>
#include "EWEngine/Scene.h"
#include "EWEngine/Free_Camera_Controller.h"
#include <EWEngine/Systems/Rendering/Rigid/RigidRS.h>
#include <EWEngine/imgui/imGuiHandler.h>

#include "../Pipelines/TerrainPipe.h"
#include "../Pipelines/GeneratedGrassPipe.h"


namespace EWE {
	class PBRScene : public SceneBase {
	public:
		PBRScene(EightWindsEngine& ewEngine);
		~PBRScene();

		void Load() final;
		void Entry() final;
		void Exit() final;
		bool Render(double dt) final;

		EightWindsEngine& ewEngine;
		MenuManager& menuManager;
		std::shared_ptr<SoundEngine> soundEngine;
		GLFWwindow* windowPtr;
		CameraController camControl;
		TransformComponent camTransform{};


		ImGUIHandler imguiHandler;

		void InitSphereMaterialResources();
		std::array<EWEBuffer*, 2> csmEWEBuffer; //csmEWE == controlled sphere material EWE buffer
		MaterialBuffer controlledSphereMB;
		int updatedCMB = MAX_FRAMES_IN_FLIGHT; //CMB == controlled material buffer
		TransformComponent sphereTransform;
		bool sphereDrawable = true;
		MaterialObjectInfo controlledSphere;

		EWEModel* groundModel{ nullptr };
		EWEModel* sphereModel{ nullptr };


		void InitTerrainResources();
		std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> tessBuffer;
		VkDescriptorSet terrainDesc[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };

		//int updatedTBO = 0;
		TessBufferObject tbo;
		bool terrainWire = false;
		bool terrainActive = true;

		void InitGrassResources();
		GrassBufferObject gbo;
		std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> grassBuffer;
		VkDescriptorSet grassDesc[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
		bool grassActive = true;
	

		void InitPerlinNoiseResources();
		VkSampler perlinNoiseSampler{VK_NULL_HANDLE};
		VkDeviceMemory perlinNoiseImageMemory{VK_NULL_HANDLE};
		VkImage perlinNoiseImage{VK_NULL_HANDLE};
		VkImageView perlinNoiseImageView{VK_NULL_HANDLE};
		VkDescriptorImageInfo perlinComputeImgInfo{VK_NULL_HANDLE};
		VkDescriptorImageInfo perlinGraphicsImgInfo{VK_NULL_HANDLE};
		VkDescriptorSet perlinDesc[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
		EWEDescriptorSetLayout* perlinGenDSL{nullptr};

		void RenderLBOControls();
		void RenderCameraData();
		void RenderControlledSphereControls();
		void RenderTerrainControls();
		void RenderGrassControls();

		int updatedLBO = MAX_FRAMES_IN_FLIGHT;
		LightBufferObject lbo;
	};
}

