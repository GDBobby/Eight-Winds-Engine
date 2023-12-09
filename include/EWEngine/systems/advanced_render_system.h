#pragma once

#include "../Graphics/EWE_Camera.h"
#include "../Graphics/EWE_Pipeline.h"
#include "../Graphics/EWE_Device.hpp"
#include "../Graphics/EWE_FrameInfo.h"

#include "../GUI/UIHandler.h"

#include "../GUI/MenuManager.h"

#include "../ObjectManager.h"

#include <memory>
#include <vector>

#define RENDERING_EIGHT_WINDS false

namespace EWE {
	class AdvancedRenderSystem {
	public:

	//descriptorsetlayouts are in a vector vector
	//the first layert of the vector (vector<vector>>) designates a pipeline
	//the second layer (the vector inside the vector) designates the descriptorsets in that pipeline
		AdvancedRenderSystem(EWEDevice& device, VkPipelineRenderingCreateInfo const& pipeRenderInfo, ObjectManager& objectManager, MenuManager& menuManager);
		~AdvancedRenderSystem();

		AdvancedRenderSystem(const AdvancedRenderSystem&) = delete;
		AdvancedRenderSystem& operator=(const AdvancedRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);
		void render2DGameObjects(FrameInfo2D& frameInfo);

		void renderLoadingScreen(FrameInfoLoading& frameInfo);

		std::shared_ptr<MaterialHandler> materialHandlerInstance;

#if LEVEL_BUILDER
		void renderBuilderObjects(FrameInfo& frameInfo);
#endif
		void updateLoadingPipeline(VkPipelineRenderingCreateInfo const& pipeRenderInfo);
		void updatePipelines(ObjectManager& objectManager, VkPipelineRenderingCreateInfo const& pipeRenderInfo);
		void updateMaterialPipelines(VkPipelineRenderingCreateInfo const& pipeRenderInfo); //only cause this outside of ARS in the level builder, donot call elsewhere, updatepipelines calls this
		void initGlobalPool(unsigned int maxSets);

		std::shared_ptr<EWEDescriptorPool> globalPool{};

		bool shouldRenderPoints = false;
		//uint32_t uiTextureID = 0;
		void takeUIHandlerPtr(UIHandler* uiHandlerPtr) { uiHandler = uiHandlerPtr; }

		ObjectManager& objectManager;
		MenuManager& menuManager;

		std::shared_ptr<EWEModel> get2DModel() {
			return model2D;
		}

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
#if RENDERING_EIGHT_WINDS
		void RenderSpikyball(FrameInfo& frameInfo);
#endif
		void RenderGrass(FrameInfo& frameInfo);

		void RenderDynamicMaterials(FrameInfo& frameInfo);

		EWEDevice& eweDevice;

	};
}