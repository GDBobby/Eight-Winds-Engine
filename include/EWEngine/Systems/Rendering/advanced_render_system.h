#pragma once

#include "EWEngine/Graphics/Camera.h"
#include "EWEngine/Graphics/Pipeline.h"
#include "EWEngine/Graphics/Device.hpp"

#include "EWEngine/GUI/UIHandler.h"

#include "EWEngine/GUI/MenuManager.h"

#include "EWEngine/Graphics/PointLight.h"

#include <memory>
#include <vector>

namespace EWE {
	class AdvancedRenderSystem {
	public:

	//descriptorsetlayouts are in a vector vector
	//the first layert of the vector (vector<vector>>) designates a pipeline
	//the second layer (the vector inside the vector) designates the descriptorsets in that pipeline
		AdvancedRenderSystem(MenuManager& menuManager);
		~AdvancedRenderSystem();

		AdvancedRenderSystem(const AdvancedRenderSystem&) = delete;
		AdvancedRenderSystem& operator=(const AdvancedRenderSystem&) = delete;

		void renderGameObjects(float time);
		void render2DGameObjects(bool menuActive);


#if LEVEL_BUILDER
		void renderBuilderObjects();
#endif

		bool shouldRenderPoints = false;
		//uint32_t uiTextureID = 0;
		void takeUIHandlerPtr(UIHandler* uiHandlerPtr) { uiHandler = uiHandlerPtr; }

		MenuManager& menuManager;

		EWEModel* get2DModel() {
			return model2D;
		}

		bool drawSkybox = true;
		EWEModel* skyboxModel{ nullptr };

		void CreateSkyboxDescriptor(ImageID skyboxImgID);

		std::vector<PointLight> pointLights{};
	private:
		UIHandler* uiHandler;

		//bool globalDescriptorSetBound = false;

		EWEModel* model2D;

		void renderSkybox();


#if DRAWING_POINTS
		void renderPointLights();
#endif
		void RenderLightning();


		EWEDescriptorSetLayout* skyboxEDSL{ nullptr };
		std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> skyboxDescriptors{ VK_NULL_HANDLE, VK_NULL_HANDLE };
	};
}