#pragma once

#include "EWEngine/Graphics/Camera.h"
#include "EWEngine/Graphics/Pipeline.h"
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Frame_Info.h"

#include "EWEngine/GUI/UIHandler.h"

#include "EWEngine/GUI/MenuManager.h"

#include "EWEngine/ObjectManager.h"

#include <memory>
#include <vector>

namespace EWE {
	class AdvancedRenderSystem {
	public:

	//descriptorsetlayouts are in a vector vector
	//the first layert of the vector (vector<vector>>) designates a pipeline
	//the second layer (the vector inside the vector) designates the descriptorsets in that pipeline
		AdvancedRenderSystem(EWEDevice& device, VkPipelineRenderingCreateInfo* pipeRenderInfo, ObjectManager& objectManager, MenuManager& menuManager);
		~AdvancedRenderSystem();

		AdvancedRenderSystem(const AdvancedRenderSystem&) = delete;
		AdvancedRenderSystem& operator=(const AdvancedRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);
		void render2DGameObjects(FrameInfo2D& frameInfo);

		//void renderLoadingScreen(FrameInfoLoading& frameInfo);

		std::shared_ptr<MaterialHandler> materialHandlerInstance;

#if LEVEL_BUILDER
		void renderBuilderObjects(FrameInfo& frameInfo);
#endif
		void updateLoadingPipeline();
		void updatePipelines();
		void updateMaterialPipelines(); //only cause this outside of ARS in the level builder, donot call elsewhere, updatepipelines calls this

		bool shouldRenderPoints = false;
		//uint32_t uiTextureID = 0;
		void takeUIHandlerPtr(UIHandler* uiHandlerPtr) { uiHandler = uiHandlerPtr; }

		ObjectManager& objectManager;
		MenuManager& menuManager;

		std::shared_ptr<EWEModel> get2DModel() {
			return model2D;
		}

		bool drawSkybox = true;
	private:
		UIHandler* uiHandler;

		//bool globalDescriptorSetBound = false;

		std::shared_ptr<EWEModel> model2D;

		void renderSkybox(FrameInfo& frameInfo);
		void renderTexturedGameObjects(FrameInfo& frameInfo);
		void renderVisualEffects(FrameInfo& frameInfo);
		
		void renderSprites(FrameInfo& frameInfo);

#if DRAWING_POINTS
		void renderPointLights(FrameInfo& frameInfo);
#endif
		void RenderLightning(FrameInfo& frameInfo);

		void RenderGrass(FrameInfo& frameInfo);

		void RenderDynamicMaterials(FrameInfo& frameInfo);

		EWEDevice& eweDevice;

	};
}