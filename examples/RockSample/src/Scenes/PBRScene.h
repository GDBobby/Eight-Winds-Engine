#pragma once

#include "../Pipelines/TerrainPipe.h"
#include "../Pipelines/GeneratedGrassPipe.h"

#include <EWEngine/EightWindsEngine.h>
#include "EWEngine/Scene.h"
#include "EWEngine/Free_Camera_Controller.h"
#include <EWEngine/Systems/Rendering/Rigid/RigidRS.h>
#include <EWGraphics/imgui/imGuiHandler.h>
#include <EWGraphics/Vulkan/ComputePipeline.h>

#include <EWEngine/Sound_Engine.h>


#include <LAB/CameraCSRuntime.h>


enum RenderStrat { 
	RS_Tess, 
	RS_Simple, 
	RS_COUNT 
};

namespace EWE {
	class PBRScene : public SceneBase {
	public:
		PBRScene(EightWindsEngine& ewEngine, ImageID skyboxImgID);
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
		lab::Transform<float, 3> camTransform{};


		ImGUIHandler imguiHandler;

		void InitSphereMaterialResources();
		std::array<EWEBuffer*, 2> csmEWEBuffer; //csmEWE == controlled sphere material EWE buffer
		MaterialBuffer controlledSphereMB;
		int updatedCMB = MAX_FRAMES_IN_FLIGHT; //CMB == controlled material buffer
		lab::Transform3 sphereTransform;
		bool sphereDrawable = true;
		MaterialObjectInfo controlledSphere;

		bool materialsActive = false;
		EWEModel* sphereModel{ nullptr };

		EWECamera fakeCameraForCullingDemo;
		bool fakeCameraBool = false;
		bool conservativeFrustum = false;


		void InitTerrainResources();
		std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> tessBuffer;
		VkDescriptorSet terrainDesc[RS_COUNT][MAX_FRAMES_IN_FLIGHT];

		//int updatedTBO = 0;
		TessBufferObject tbo;
		EWEModel* terrainQuadModel{ nullptr };
		EWEModel* terrainTriModel{ nullptr };
		MaterialInfo dirtMatInfo;
		bool terrainActive = true;
		int renderStrat = 1;

		void InitGrassResources();
		GrassBufferObject gbo;
		std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> grassBuffer;
		//std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> ttmGrassBuffer;
		VkDescriptorSet grassDesc[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
		bool grassActive = false;
		lab::ivec3 grassGroup{ 256, 1, 256 };
		bool displayGrassLOD = true;


		void RenderLBOControls();
		void RenderCameraData();
		void RenderControlledSphereControls();
		void RenderTerrainControls();
		void RenderGrassControls();
		//void RenderOceanControls();

		int updatedLBO = MAX_FRAMES_IN_FLIGHT;
		LightBufferObject lbo;


		//Ocean::Ocean* ocean{ nullptr };
		//bool oceanEnabled = true;
		//bool oceanActive = oceanEnabled;
		//int oceanRenderParamsUpdated = 0;

		float fov_degrees = 70.f;
		bool updated_cam_data = false;
		lab::Perspective::API cam_perspective = lab::Perspective::Vulkan;

		lab::Runtime::CoordinateSystem runtimeCS{};

		
	};
}

