#pragma once

#include "EWGraphics/Vulkan/Device.hpp"
#include "EWGraphics/Vulkan/Pipeline.h"
#include "EWGraphics/Vulkan/Descriptors.h"
#include "EWGraphics/Vulkan/Device_Buffer.h"

#include <LAB/Vector.h>

#include <iostream>

#define ENABLE_VALIDATION false 

#define DEFAULT_WIDTH 1920.f
#define DEFAULT_HEIGHT 1080.f

namespace EWE {

	enum TextAlign : uint8_t { TA_left, TA_center, TA_right };
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
		uint16_t GetSelectionIndex(double xpos);
		float GetWidth();
	};

	struct Font {
		~Font();
		struct CharacterData {
			struct Vert {
				float x;
				float y;
				float u;
				float v;
			};
			//top left and bottom right, can be mixed for top right and bottom left
			const Vert vertices[2];
		};
		Font(std::vector<CharacterData>& vertData, std::vector<float>& advanceData, std::size_t width, std::size_t height, uint8_t firstChar, void* imgdata);

		float GetCharWidth(char c, const float charW) const;
		CharacterData::Vert const* GetVertData(const char c) const;
		float GetStringWidth(std::string const& str, const float charW) const;

		const uint16_t firstChar;
		const uint16_t width;
		const uint16_t height;

		const std::string name;

		std::size_t drawnLetterCount = 0;
		const std::vector<CharacterData> vertData{};
		const std::vector<float> advanceData;

		VkSampler sampler;
		VkImage image;
		VkImageView view;
		VkDeviceMemory imageMemory;

		std::array<EWEBuffer*, 2> buffers{};
		std::array<VkDescriptorSet, 2> descriptorSets;
	};


	class TextOverlay {
	private:
		static constexpr uint32_t TEXTOVERLAY_MAX_CHAR_COUNT = 65536 / sizeof(lab::vec4);

		float frameBufferWidth;
		float frameBufferHeight;

		//VkDescriptorPool descriptorPool;
		VkPipelineLayout pipelineLayout;
		VkPipelineCache pipelineCache;
		VkPipeline pipeline;
		//std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		// Pointer to mapped vertex buffer
		Font::CharacterData::Vert* mapped = nullptr;
		EWEBuffer* vertexBuffer[2] = { nullptr, nullptr };
		EWEDescriptorSetLayout* eDSL;

		uint32_t numLetters;

		std::vector<Font> fonts{};
		int16_t currentFont;

		friend struct TextStruct;
		friend struct Font;

	public:

		bool visible = true;
		float scale;

		TextOverlay(float framebufferwidth, float framebufferheight, VkPipelineRenderingCreateInfo* pipelineInfo);

		~TextOverlay();
		void PrepareResources();
		void PreparePipelineLayout();
		void PreparePipeline(VkPipelineRenderingCreateInfo renderingInfo);
		void LoadConsolas24();
		float GetWidth(std::string const& text, float textScale = 1.f);
		//float addText(std::string text, float x, float y, TextAlign align, float textScale = 1.f);
		void AddText(TextStruct const& textStruct, const float scaleX = 1.f);

		static void StaticAddText(TextStruct textStruct);

		void Draw();
		void AddDefaultText(double time, double peakTime, double averageTime, double highTime);
		void BeginTextUpdate();
		void EndTextUpdate();

		bool SetCurrentFont(uint16_t);
		int16_t GetCurrentFont();
		std::string const& GetCurrentFontName();

		void AddFont(Font const& font);
		bool RemoveFont(uint16_t fontIndex);


		void WindowResize() {
			frameBufferWidth = VK::Object->screenWidth;
			frameBufferHeight = VK::Object->screenHeight;
			scale = frameBufferWidth / DEFAULT_WIDTH;
		}
	};
}
