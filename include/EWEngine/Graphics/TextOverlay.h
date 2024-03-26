
#pragma once

#include "Device.hpp"
#include "Pipeline.h"
#include "Descriptors.h"

#include <glm/glm.hpp>

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "EWEngine/Fonts/stb_font_consolas_24_latin1.inl"



#include <iostream>

#define ENABLE_VALIDATION false

#define TEXTOVERLAY_MAX_CHAR_COUNT 65536

#define DEFAULT_WIDTH 1920.f
#define DEFAULT_HEIGHT 1080.f

namespace EWE {

	enum TextAlign { TA_left, TA_center, TA_right };
	struct TextStruct {
		std::string string;
		float x{ 0.f };
		float y{ 0.f };
		unsigned char align{ TA_left };
		float scale{ 1.f };
		TextStruct() {}
		TextStruct(std::string string, float x, float y, unsigned char align, float scale)
			: string{ string }, x{ x }, y{ y }, align{ align }, scale{ scale }
		{}
		TextStruct(std::string string, float x, float y, TextAlign align, float scale) 
			: string{ string }, x{ x }, y{ y }, align{ (unsigned char)align }, scale{ scale }
		{}
		uint16_t getSelectionIndex(double xpos, float screenWidth);
		float getWidth(float screenWidth);
	};


	class TextOverlay {
	private:
		static TextOverlay* textOverlayPtr;

		float frameBufferWidth;
		float frameBufferHeight;

		VkSampler sampler;
		VkImage image;
		VkImageView view;
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDeviceMemory imageMemory;
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout{};
		VkDescriptorSet descriptorSet;
		VkPipelineLayout pipelineLayout;
		VkPipelineCache pipelineCache;
		VkPipeline pipeline;
		//std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		// Pointer to mapped vertex buffer
		glm::vec4* mapped = nullptr;

		uint32_t numLetters;
	public:

		static stb_fontchar stbFontData[STB_FONT_consolas_24_latin1_NUM_CHARS];

		bool visible = true;
		float scale;

		std::vector<VkCommandBuffer> cmdBuffers;

		TextOverlay(
			float framebufferwidth,
			float framebufferheight,
			VkPipelineRenderingCreateInfo const& pipelineInfo
		);

		~TextOverlay();
		void prepareResources();
		void preparePipeline(VkPipelineRenderingCreateInfo renderingInfo);
		float getWidth(std::string text, float textScale = 1.f);
		//float addText(std::string text, float x, float y, TextAlign align, float textScale = 1.f);
		void addText(TextStruct textStruct);
		void addTextEx(TextStruct textStruct, float scaleX);

		static void staticAddText(TextStruct textStruct);

		void draw(VkCommandBuffer commandBuffer);
		void addDefaultText(double time, double peakTime, double averageTime, double highTime);
		VkCommandBuffer beginBuffer(int bufferIndex);
		void beginTextUpdate();
		void endTextUpdate();

		void windowResize(float newWidth, float newHeight) {
			frameBufferWidth = newWidth;
			frameBufferHeight = newHeight;
			scale = frameBufferWidth / DEFAULT_WIDTH;
		}
	};
}
