
#pragma once

#include "Device.hpp"
#include "Pipeline.h"
#include "Descriptors.h"

#include <glm/glm.hpp>




#include <iostream>

#define ENABLE_VALIDATION false 

#define DEFAULT_WIDTH 1920.f
#define DEFAULT_HEIGHT 1080.f

namespace EWE {

	enum TextAlign { TA_left, TA_center, TA_right };
	struct TextStruct {
		std::string string;
		float x{ 0.f };
		float y{ 0.f };
		uint8_t align{ TA_left };
		float scale{ 1.f };
		TextStruct() {}
		TextStruct(std::string string, float x, float y, uint8_t align, float scale)
			: string{ string }, x{ x }, y{ y }, align{ align }, scale{ scale }
		{}
		TextStruct(std::string string, float x, float y, TextAlign align, float scale) 
			: string{ string }, x{ x }, y{ y }, align{ static_cast<uint8_t>(align) }, scale{ scale }
		{}
		uint16_t GetSelectionIndex(double xpos, float screenWidth);
		float GetWidth(float screenWidth);
	};


	class TextOverlay {
	private:
		static constexpr uint32_t TEXTOVERLAY_MAX_CHAR_COUNT = 65536 / sizeof(glm::vec4);
		static TextOverlay* textOverlayPtr;

		float frameBufferWidth;
		float frameBufferHeight;

		VkSampler sampler;
		VkImage image;
		VkImageView view;
		VkDeviceMemory imageMemory;
		//VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout{};
		VkDescriptorSet descriptorSet[2];
		VkPipelineLayout pipelineLayout;
		VkPipelineCache pipelineCache;
		VkPipeline pipeline;
		//std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		// Pointer to mapped vertex buffer
		glm::vec4* mapped = nullptr;
		EWEBuffer* vertexBuffer[2] = { nullptr, nullptr };

		uint32_t numLetters;
	public:

		bool visible = true;
		float scale;

		std::vector<VkCommandBuffer> cmdBuffers;

		TextOverlay(
			float framebufferwidth,
			float framebufferheight,
			VkPipelineRenderingCreateInfo const& pipelineInfo
		);

		~TextOverlay();
		void PrepareResources();
		void PreparePipeline(VkPipelineRenderingCreateInfo renderingInfo);
		float GetWidth(std::string text, float textScale = 1.f);
		//float addText(std::string text, float x, float y, TextAlign align, float textScale = 1.f);
		void AddText(TextStruct textStruct, const float scaleX = 1.f);

		static void StaticAddText(TextStruct textStruct);

		void Draw(FrameInfo frameInfo);
		void AddDefaultText(double time, double peakTime, double averageTime, double highTime);
		void BeginTextUpdate(uint8_t frameIndex);
		void EndTextUpdate(FrameInfo frameInfo);

		void WindowResize(float newWidth, float newHeight) {
			frameBufferWidth = newWidth;
			frameBufferHeight = newHeight;
			scale = frameBufferWidth / DEFAULT_WIDTH;
		}
	};
}
