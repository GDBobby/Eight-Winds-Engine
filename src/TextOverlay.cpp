
#include "EWGraphics/Vulkan/Renderer.h"
#include "EWEngine/Graphics/TextOverlay.h"

#include "EWGraphics/Texture/Sampler.h"


#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "EWEngine/Fonts/stb_font_consolas_24_latin1.inl"

#include <stdexcept>
//#include <iostream>
#include <sstream>
#include <iomanip>


namespace EWE {

	TextOverlay* textOverlayPtr{ nullptr };


	TextOverlay::TextOverlay(float framebufferwidth, float framebufferheight, VkPipelineRenderingCreateInfo* pipelineInfo) 
		: frameBufferWidth{ framebufferwidth }, 
		frameBufferHeight{ framebufferheight }, 
		scale{ frameBufferWidth / DEFAULT_WIDTH }
	{
		assert(textOverlayPtr == nullptr && "trying to recreate textoverlay??");
		textOverlayPtr = this;

		//printf("text overlay construction \n");

		PrepareResources();
		//printf("after prepare resources \n");
		PreparePipelineLayout();
		PreparePipeline(*pipelineInfo);

		LoadConsolas24();
		//printf("afterr prepare pipeline \n");
	}

	Font::~Font() {

		Sampler::RemoveSampler(sampler);
		EWE_VK(vkDestroyImage, VK::Object->vkDevice, image, nullptr);
		EWE_VK(vkDestroyImageView, VK::Object->vkDevice, view, nullptr);
		EWE_VK(vkFreeMemory, VK::Object->vkDevice, imageMemory, nullptr);
	}

	Font::Font(std::vector<CharacterData>& vertData, std::vector<float>& advanceData, std::size_t width, std::size_t height, uint8_t firstChar, void* imgdata) 
		: vertData{ std::move(vertData) }, 
		advanceData{ std::move(advanceData) }, 
		width{ width }, 
		height{ height }, 
		firstChar{ firstChar } 
	{
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = VK_FORMAT_R8_UNORM;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.flags = 0; //optional????


		EWE_VK(vkCreateImage, VK::Object->vkDevice, &imageCreateInfo, nullptr, &image);
#if DEBUG_NAMING
		DebugNaming::SetObjectName(image, VK_OBJECT_TYPE_IMAGE, "textoverlay image");
#endif

		VkMemoryRequirements memRequirements;
		EWE_VK(vkGetImageMemoryRequirements, VK::Object->vkDevice, image, &memRequirements);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		EWE_VK(vkAllocateMemory, VK::Object->vkDevice, &allocInfo, nullptr, &imageMemory);

		EWE_VK(vkBindImageMemory, VK::Object->vkDevice, image, imageMemory, 0);


		// Staging
		StagingBuffer* stagingBuffer = Construct<StagingBuffer>({ allocInfo.allocationSize, imgdata });
		// Copy to image

		SyncHub* syncHub = SyncHub::GetSyncHubInstance();
		const bool inMainThread = VK::Object->CheckMainThread();

		CommandBuffer& cmdBuf = syncHub->BeginSingleTimeCommand();
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;
		{   //initialize image

			VkImageMemoryBarrier imageBarrier = Barrier::ChangeImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
			EWE_VK(vkCmdPipelineBarrier, cmdBuf,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageBarrier
			);
		}

		{ //transfer data to image
			VkBufferImageCopy bufferCopyRegion{};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = 0;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = width;
			bufferCopyRegion.imageExtent.height = height;
			bufferCopyRegion.imageExtent.depth = 1;

			EWE_VK(vkCmdCopyBufferToImage,
				cmdBuf,
				stagingBuffer->buffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&bufferCopyRegion
			);
		}
		{//transition image to a read state, and from transfer queue to graphics queue (in one barrier?)
			VkImageMemoryBarrier imageBarrier = Barrier::ChangeImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);
			if (inMainThread) {
				GraphicsCommand gCommand{};
				gCommand.command = &cmdBuf;
				gCommand.stagingBuffer = stagingBuffer;
				EWE_VK(vkCmdPipelineBarrier, cmdBuf,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &imageBarrier
				);
				syncHub->EndSingleTimeCommandGraphics(gCommand);
			}
			else {
				if (VK::Object->queueEnabled[Queue::transfer]) {
					imageBarrier.srcQueueFamilyIndex = VK::Object->queueIndex[Queue::transfer];
					imageBarrier.dstQueueFamilyIndex = VK::Object->queueIndex[Queue::graphics];
					PipelineBarrier pipeBarrier{};
					pipeBarrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
					pipeBarrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
					pipeBarrier.AddBarrier(imageBarrier);
					pipeBarrier.dependencyFlags = 0;
					pipeBarrier.Submit(cmdBuf);

					TransferCommand command{};
					command.commands.push_back(&cmdBuf);
					command.stagingBuffers.push_back(stagingBuffer);
					command.pipeBarriers.push_back(std::move(pipeBarrier));
					syncHub->EndSingleTimeCommandTransfer(command);
				}
				else {
					imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					PipelineBarrier pipeBarrier{};
					pipeBarrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
					pipeBarrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
					pipeBarrier.AddBarrier(imageBarrier);
					pipeBarrier.dependencyFlags = 0;
					pipeBarrier.Submit(cmdBuf);
				}
			}

		}

		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.pNext = nullptr;
		imageViewInfo.image = image;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = imageCreateInfo.format;
		imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		EWE_VK(vkCreateImageView, VK::Object->vkDevice, &imageViewInfo, nullptr, &view);

		// Sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext = nullptr;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		sampler = Sampler::GetSampler(samplerInfo);

		VkDescriptorImageInfo descImgInfo;
		descImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descImgInfo.imageView = view;
		descImgInfo.sampler = sampler;



		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			buffers[i] = Construct<EWEBuffer>({sizeof(Font::CharacterData::Vert) * 4, TextOverlay::TEXTOVERLAY_MAX_CHAR_COUNT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT});

			EWEDescriptorWriter writer{ textOverlayPtr->eDSL, DescriptorPool_Global };
			writer.WriteBuffer(buffers[i]->DescriptorInfo());
			writer.WriteImage(&descImgInfo);
			descriptorSets[i] = writer.Build();
		}
	}

	TextOverlay::~TextOverlay() {
		// Free up all Vulkan resources requested by the text overlay
#if DECONSTRUCTION_DEBUG
		printf("deconstrructing textoverlay \n");
#endif
		Deconstruct(vertexBuffer[0]);
		Deconstruct(vertexBuffer[1]);
		Deconstruct(eDSL);

		EWE_VK(vkDestroyShaderModule, VK::Object->vkDevice, vertShaderModule, nullptr);
		EWE_VK(vkDestroyShaderModule, VK::Object->vkDevice, fragShaderModule, nullptr);
		EWE_VK(vkDestroyPipelineLayout, VK::Object->vkDevice, pipelineLayout, nullptr);
		EWE_VK(vkDestroyPipelineCache, VK::Object->vkDevice, pipelineCache, nullptr);
		EWE_VK(vkDestroyPipeline, VK::Object->vkDevice, pipeline, nullptr);

#if DECONSTRUCTION_DEBUG
		printf("end deconstruction textoverlay \n");
#endif

	}


	uint16_t TextStruct::GetSelectionIndex(double xpos) {
		const float charW = 1.5f * scale / VK::Object->screenWidth;
		const float width = GetWidth();
		float currentPos = x;

		const Font* font = &textOverlayPtr->fonts[textOverlayPtr->currentFont];
#if EWE_DEBUG
		printf("xpos get selection index - %.1f \n", xpos);
#endif
		switch (align) {
			case TA_left:break;
			case TA_center: currentPos -= width / 2.f; break;
			case TA_right: currentPos -= width; break;
		}

		//float lastPos = currentPos;
		for (uint16_t i = 0; i < string.length(); i++) {
			currentPos += font->GetCharWidth(string[i], charW) * VK::Object->screenWidth / 8.f;
#if EWE_DEBUG
			printf("currentPos : %.2f \n", currentPos);
#endif
			if (xpos <= currentPos) { return i; }
			currentPos += font->GetCharWidth(string[i], charW) * VK::Object->screenWidth * 3.f / 8.f;
		}
		return static_cast<uint16_t>(string.length());
	}

	float Font::GetCharWidth(const char c, const float charW) const {
		return advanceData[static_cast<uint32_t>(c) - firstChar] * charW;
	}
	float Font::GetStringWidth(std::string const& str, const float charW) const {
		float ret = 0.f;
		for (auto const& letter : str) {
			ret += advanceData[static_cast<uint32_t>(letter) - firstChar] * charW;
		}
		return ret;
	}
	Font::CharacterData::Vert const* Font::GetVertData(const char c) const {
		return vertData[static_cast<uint32_t>(c) - firstChar].vertices;
	}

	float TextStruct::GetWidth() {
		//std::cout << "yo? : " << frameBufferWidth << std::endl;
		const float charW = 1.5f * scale / VK::Object->screenWidth;
		//printf("text struct get width : %.5f \n", textWidth);
#if EWE_DEBUG
		const float textWidth = textOverlayPtr->fonts[textOverlayPtr->currentFont].GetStringWidth(string, charW);
		if (textWidth < 0.0f) {

			printf("width less than 0, what  was the string? : %s:%.1f \n", string.c_str(), VK::Object->screenWidth);
			assert(false);
		}
		return textWidth;
#else
		return textOverlayPtr->fonts[textOverlayPtr->currentFont].GetStringWidth(string, charW);
#endif
	}

	void TextOverlay::PreparePipelineLayout() {

	}

	void TextOverlay::LoadConsolas24() {

		unsigned char** font24pixels = new unsigned char* [STB_FONT_consolas_24_latin1_BITMAP_WIDTH];
		for (std::size_t i = 0; i < STB_FONT_consolas_24_latin1_BITMAP_WIDTH; ++i) {
			font24pixels[i] = new unsigned char[STB_FONT_consolas_24_latin1_BITMAP_WIDTH];
		}

		std::vector<stb_fontchar> stbData{};
		stb_font_consolas_24_latin1(stbData.data(), font24pixels, STB_FONT_consolas_24_latin1_BITMAP_WIDTH);

		// Font texture
	}

	void TextOverlay::PrepareResources() {

		eDSL = EWEDescriptorSetLayout::Builder()
			.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = eDSL->GetDescriptorSetLayout();

		//std::cout << "pipelineinfo 3" << std::endl;

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout);
#if DEBUG_NAMING
		DebugNaming::SetObjectName(pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "textoverlay pipe layout");
#endif

		// Pipeline cache
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheCreateInfo.pNext = nullptr;
		pipelineCacheCreateInfo.initialDataSize = 0;
		pipelineCacheCreateInfo.pInitialData = nullptr;
		pipelineCacheCreateInfo.flags = 0;
		EWE_VK(vkCreatePipelineCache, VK::Object->vkDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache);


		//std::cout << "end of function" << std::endl;
	}
	void TextOverlay::PreparePipeline(VkPipelineRenderingCreateInfo renderingInfo) {
		//printf("preparing pipeline \n");
		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		inputAssemblyState.flags = 0;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;
		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.flags = 0;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;
		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;
		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.flags = 0;
		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.flags = 0;
		VkDynamicState dynamicStateEnables[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables;
		dynamicState.dynamicStateCount = 2;
		dynamicState.flags = 0;
		/*
		* 
		* if using indices to character data instead of uploading the character data, use this pipeline vertex data
		* 
		VkVertexInputBindingDescription vertexInputBindings;
		vertexInputBindings.binding = 0;
		vertexInputBindings.stride = sizeof(lab::vec4);
		vertexInputBindings.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription vertexInputAttributes;
		vertexInputAttributes.location = 0;
		vertexInputAttributes.binding = 0;
		vertexInputAttributes.format = VK_FORMAT_R16_UINT;
		vertexInputAttributes.offset = 0;
		*/

		VkPipelineVertexInputStateCreateInfo vertexInputState{};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		
		vertexInputState.vertexBindingDescriptionCount = 0;
		vertexInputState.pVertexBindingDescriptions = nullptr;// &vertexInputBindings;
		vertexInputState.vertexAttributeDescriptionCount = 0;
		vertexInputState.pVertexAttributeDescriptions = nullptr;// &vertexInputAttributes;
		
		vertexInputState.vertexBindingDescriptionCount = 0;
		vertexInputState.pVertexBindingDescriptions = nullptr;
		vertexInputState.vertexAttributeDescriptionCount = 0;
		vertexInputState.pVertexAttributeDescriptions = nullptr;


		//printf("after vertex input state \n");
		auto vertCode = Pipeline_Helper_Functions::ReadFile("shaders/textoverlay.vert.spv");
		//printf("after vert code read file \n");
		auto fragCode = Pipeline_Helper_Functions::ReadFile("shaders/textoverlay.frag.spv");
		//printf("after frag code read file \n");
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.codeSize = vertCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(vertCode.data());
		createInfo.flags = 0;
		//printf("after shader module create info \n");
		EWE_VK(vkCreateShaderModule, VK::Object->vkDevice, &createInfo, nullptr, &vertShaderModule);
		//printf("after successfully creating shader module \n");

		createInfo.codeSize = fragCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(fragCode.data());
		//printf("setting shader module create info to frag \n");
		EWE_VK(vkCreateShaderModule, VK::Object->vkDevice, &createInfo, nullptr, &fragShaderModule);
		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertShaderModule;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;
		//printf("shader state [0] \n");

		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragShaderModule;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;
		//printf("shader state [1] \n");

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.pNext = &renderingInfo;
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = pipelineLayout;
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.basePipelineIndex = -1;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = 2;
		pipelineCreateInfo.pStages = shaderStages;
		//printf("after pipeline create info \n");

		EWE_VK(vkCreateGraphicsPipelines, VK::Object->vkDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline);
#if DEBUG_NAMING
		DebugNaming::SetObjectName(pipeline, VK_OBJECT_TYPE_PIPELINE, "textoverlay pipeline");
#endif
		//printf("successfully created textoverlay graphics pipeline \n");
		//printf("end of text overlay constructor \n");
	}

	void TextOverlay::AddDefaultText(double time, double peakTime, double averageTime, double highTime) {
		int16_t previousFont = currentFont;
		SetCurrentFont(0);
		AddText(TextStruct{ EWEDevice::GetEWEDevice()->deviceName, 0, frameBufferHeight - (20.f * scale), TA_left, 1.f});
		//printf("frameBuffer : %d : %d \n", frameBufferWidth, frameBufferHeight);
		int lastFPS = static_cast<int>(1 / time);
		int averageFPS = static_cast<int>(1 / averageTime);
		std::string buffer_string = std::format("frame time: {:.2f} ms ({} fps)", time * 1000.0f, lastFPS);
		AddText(TextStruct{ buffer_string, 0.f, frameBufferHeight - (40.f * scale), TA_left, 1.f });
		buffer_string = std::format("average FPS: {}", averageFPS);
		AddText(TextStruct{ buffer_string, 0.f, frameBufferHeight - (60.f * scale), TA_left, 1.f });
		buffer_string = std::format("peak: {:.2f} ms ~ average: {:.2f} ms ~ high: {:.2f} ms", peakTime * 1000, averageTime * 1000, highTime * 1000);
		AddText(TextStruct{ buffer_string, 0.f, frameBufferHeight - (80.f * scale), TA_left, 1.f });
		SetCurrentFont(previousFont);
	}

	float TextOverlay::GetWidth(std::string const& text, float textScale) {
		//std::cout << "yo? : " << frameBufferWidth << std::endl;
		const float charW = 1.5f * scale * textScale / frameBufferWidth;
		float textWidth = 0;
		for (auto const& letter : text) {
			textWidth += fonts[currentFont].GetCharWidth(letter, charW);
		}
		return textWidth;
	}
	void TextOverlay::StaticAddText(TextStruct textStruct) {
		textOverlayPtr->AddText(textStruct);
	}

	void TextOverlay::AddText(TextStruct const& textStruct, const float scaleX) {

		assert(mapped != nullptr);
		const float charW = 1.5f * scale * scaleX * textStruct.scale / frameBufferWidth;
		const float charH = 1.5f * scale * textStruct.scale / frameBufferHeight;

		float xPos = (textStruct.x / frameBufferWidth * 2.0f) - 1.0f;
		const float yPos = (textStruct.y / frameBufferHeight * 2.0f) - 1.0f;

		switch (textStruct.align) {
			case TA_right:
				for (auto const& letter : textStruct.string) {
					xPos -= fonts[currentFont].GetCharWidth(letter, charW);
				}
				break;
			case TA_center:
				for (auto const& letter : textStruct.string) {
					xPos -= fonts[currentFont].GetCharWidth(letter, charW) / 2.f;
				}
				break;
			case TA_left:
				break;
		}

		mapped = reinterpret_cast<Font::CharacterData::Vert*>(fonts[currentFont].buffers[VK::Object->frameIndex]->GetMappedMemory());

		for (auto const& letter : textStruct.string) {
			if (fonts[currentFont].drawnLetterCount >= TEXTOVERLAY_MAX_CHAR_COUNT) {
				printf("trying to add more letters than allowed in textoverlay. consider expanding the TEXTOVERLAY_MAX_CHAR_COUNT constant - (drawn/max) (%d/%d) \n", TEXTOVERLAY_MAX_CHAR_COUNT);
				fonts[currentFont].drawnLetterCount++;
				break;
			}
			Font::CharacterData::Vert const* verts = fonts[currentFont].GetVertData(letter);

			mapped[0].x = (xPos + verts[0].x * charW);
			mapped[0].y = (yPos + verts[0].y * charH);
			mapped[0].u = verts[0].u;
			mapped[0].v = verts[0].v;

			mapped[1].x = (xPos + verts[1].x * charW);
			mapped[1].y = (yPos + verts[0].y * charH);
			mapped[1].u = verts[1].u;
			mapped[1].v = verts[0].v;

			mapped[2].x = (xPos + verts[0].x * charW);
			mapped[2].y = (yPos + verts[1].y * charH);
			mapped[2].u = verts[0].u;
			mapped[2].v = verts[1].v;

			mapped[3].x = (xPos + verts[1].x * charW);
			mapped[3].y = (yPos + verts[1].y * charH);
			mapped[3].u = verts[1].u;
			mapped[3].v = verts[1].v;

			mapped = reinterpret_cast<Font::CharacterData::Vert*>(reinterpret_cast<std::size_t>(mapped) + fonts[currentFont].buffers[0]->GetAlignment());

			xPos += fonts[currentFont].GetCharWidth(letter, charW);

			fonts[currentFont].drawnLetterCount++;

			numLetters++;
		}
	}

	void TextOverlay::Draw() {
			EWERenderer::BindGraphicsPipeline(pipeline);
			for (auto& font : fonts) {
				if (font.drawnLetterCount > 0) {
					EWE_VK(vkCmdBindDescriptorSets, VK::Object->GetFrameBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &font.descriptorSets[VK::Object->frameIndex], 0, nullptr);

					EWE_VK(vkCmdDraw, VK::Object->GetFrameBuffer(), 4, font.drawnLetterCount, 0, 0);
					font.drawnLetterCount = 0; // reset drawn letter count after binding
				}

			}
	}

	void TextOverlay::BeginTextUpdate() {
		vertexBuffer[VK::Object->frameIndex]->Map();
		mapped = reinterpret_cast<Font::CharacterData::Vert*>(vertexBuffer[VK::Object->frameIndex]->GetMappedMemory());
		numLetters = 0;
	}

	void TextOverlay::EndTextUpdate() {
		vertexBuffer[VK::Object->frameIndex]->Flush();
		vertexBuffer[VK::Object->frameIndex]->Unmap();
		mapped = nullptr;
		Draw();
	}

	bool TextOverlay::RemoveFont(uint16_t fontIndex) {
		if (fontIndex < fonts.size()) {
			fonts.erase(fonts.begin() + fontIndex);
			return true;
		}
		return false;
	}
	std::string const& TextOverlay::GetCurrentFontName() {
		if (currentFont >= 0 && currentFont < fonts.size()) { return fonts[currentFont].name; }
		else { return "no font currently selected"; }
	}
}
