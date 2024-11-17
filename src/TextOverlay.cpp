
#include "EWEngine/Graphics/Renderer.h"
#include "EWEngine/Graphics/TextOverlay.h"

#include "EWEngine/Graphics/Texture/Sampler.h"


#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "EWEngine/Fonts/stb_font_consolas_24_latin1.inl"

#include <stdexcept>
//#include <iostream>
#include <sstream>
#include <iomanip>


namespace EWE {

	stb_fontchar stbFontData[STB_FONT_consolas_24_latin1_NUM_CHARS];

	TextOverlay* TextOverlay::textOverlayPtr{ nullptr };


	TextOverlay::TextOverlay(
		float framebufferwidth,
		float framebufferheight,
		VkPipelineRenderingCreateInfo const& pipelineInfo
	) : frameBufferWidth{ framebufferwidth }, frameBufferHeight{ framebufferheight }, scale{ frameBufferWidth / DEFAULT_WIDTH }
	{
		assert(textOverlayPtr == nullptr && "trying to recreate textoverlay??");
		textOverlayPtr = this;

		//printf("text overlay construction \n");

		PrepareResources();
		//printf("after prepare resources \n");
		PreparePipeline(pipelineInfo);
		//printf("afterr prepare pipeline \n");
	}

	TextOverlay::~TextOverlay() {
		// Free up all Vulkan resources requested by the text overlay
#if DECONSTRUCTION_DEBUG
		printf("deconstrructing textoverlay \n");
#endif
		Deconstruct(vertexBuffer[0]);
		Deconstruct(vertexBuffer[1]);

		EWE_VK(vkDestroySampler, VK::Object->vkDevice, sampler, nullptr);
		EWE_VK(vkDestroyImage, VK::Object->vkDevice, image, nullptr);
		EWE_VK(vkDestroyImageView, VK::Object->vkDevice, view, nullptr);
		EWE_VK(vkFreeMemory, VK::Object->vkDevice, imageMemory, nullptr);
		EWE_VK(vkDestroyShaderModule, VK::Object->vkDevice, vertShaderModule, nullptr);
		EWE_VK(vkDestroyShaderModule, VK::Object->vkDevice, fragShaderModule, nullptr);
		EWE_VK(vkDestroyDescriptorSetLayout, VK::Object->vkDevice, descriptorSetLayout, nullptr);
		EWE_VK(vkDestroyPipelineLayout, VK::Object->vkDevice, pipelineLayout, nullptr);
		EWE_VK(vkDestroyPipelineCache, VK::Object->vkDevice, pipelineCache, nullptr);
		EWE_VK(vkDestroyPipeline, VK::Object->vkDevice, pipeline, nullptr);

#if DECONSTRUCTION_DEBUG
		printf("end deconstruction textoverlay \n");
#endif

	}


	uint16_t TextStruct::GetSelectionIndex(double xpos, float screenWidth) {
		const float charW = 1.5f * scale / screenWidth;
		float width = GetWidth(screenWidth);
		float currentPos = x;
		stb_fontchar* charData = &stbFontData[(uint32_t)string.back() - STB_FONT_consolas_24_latin1_FIRST_CHAR];
#if EWE_DEBUG
		printf("xpos get selection index - %.1f \n", xpos);
#endif
		if (align == TA_left) {
			/*
			if (xpos >= (x + width - charData->advance * charW / 2.f)) {
				printf("TA_left, greater than all - %.2f \n", charData->advance);
				return string.length(); 
			}
			*/
		}
		else if (align == TA_center) {
			currentPos -= width / 2.f;
			//if (xpos > x + (width - charData->advance * charW) / 2.f) { return string.length(); }
		}
		else if (align == TA_right) {
			currentPos -= width;
			//if (xpos >= (x - charData->advance * charW / 2.f)) { return string.length(); }
		}

		//float lastPos = currentPos;
		for (uint16_t i = 0; i < string.length(); i++) {
			charData = &stbFontData[static_cast<uint32_t>(string[i]) - STB_FONT_consolas_24_latin1_FIRST_CHAR];
			currentPos += (charData->advance * charW) * screenWidth / 8.f;
#if EWE_DEBUG
			printf("currentPos : %.2f \n", currentPos);
#endif
			if (xpos <= currentPos) { return i; }
			currentPos += (charData->advance * charW) * screenWidth * 3.f / 8.f;
		}
		return static_cast<uint16_t>(string.length());
	}
	float TextStruct::GetWidth(float screenWidth) {
		//std::cout << "yo? : " << frameBufferWidth << std::endl;
		const float charW = 1.5f * scale / screenWidth;
		float textWidth = 0;
		stb_fontchar* charData;
		for (auto const& letter : string) {
			charData = &stbFontData[static_cast<uint32_t>(letter) - STB_FONT_consolas_24_latin1_FIRST_CHAR];
			textWidth += charData->advance * charW;
		}
		//printf("text struct get width : %.5f \n", textWidth);
#if EWE_DEBUG
		if (textWidth < 0.0f) {

			printf("width less than 0, what  was the string? : %s:%.1f \n", string.c_str(), screenWidth);
			assert(false);
		}
#endif
		return textWidth;
	}

	void TextOverlay::PrepareResources() {

		const uint32_t fontWidth = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;
		const uint32_t fontHeight = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;

		static uint8_t font24pixels[fontWidth][fontHeight];
		stb_font_consolas_24_latin1(stbFontData, font24pixels, fontHeight);

		vertexBuffer[0] = Construct<EWEBuffer>({ TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT });
		vertexBuffer[1] = Construct<EWEBuffer>({ TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT });

		// Font texture
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = VK_FORMAT_R8_UNORM;
		imageCreateInfo.extent.width = fontWidth;
		imageCreateInfo.extent.height = fontHeight;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.flags = 0; //optional????

		//not using this function because i need the allocInfo
		//eweDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);


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
#if USING_VMA
		StagingBuffer stagingBuffer{allocInfo.allocationSize, EWEDevice::GetAllocator(), &font24pixels[0][0] };
#else
		StagingBuffer* stagingBuffer = Construct<StagingBuffer>({ allocInfo.allocationSize, &font24pixels[0][0] });
#endif
		// Copy to image

		SyncHub* syncHub = SyncHub::GetSyncHubInstance();
		CommandBuffer& cmdBuf = syncHub->BeginSingleTimeCommand(Queue::transfer);
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
			bufferCopyRegion.imageExtent.width = fontWidth;
			bufferCopyRegion.imageExtent.height = fontHeight;
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
			imageBarrier.srcQueueFamilyIndex = VK::Object->queueIndex[Queue::transfer];
			imageBarrier.dstQueueFamilyIndex = VK::Object->queueIndex[Queue::graphics];

			PipelineBarrier pipeBarrier{};
			pipeBarrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			pipeBarrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			pipeBarrier.AddBarrier(imageBarrier);
			pipeBarrier.dependencyFlags = 0;
			pipeBarrier.Submit(cmdBuf);

			TransferCommandManager::AddCommand(cmdBuf);
			TransferCommandManager::AddPropertyToCommand(stagingBuffer);
			TransferCommandManager::AddPropertyToCommand(pipeBarrier);
			syncHub->EndSingleTimeCommandTransfer();
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

		// Descriptor
		// Font uses a separate descriptor pool
		//std::vector<VkDescriptorPoolSize> poolSizes(1);
		//poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//poolSizes[0].descriptorCount = 1;


		//VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		//descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		//descriptorPoolInfo.pNext = nullptr;
		//descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		//descriptorPoolInfo.pPoolSizes = poolSizes.data();
		//descriptorPoolInfo.maxSets = 1;

		//EWE_VK(vkCreateDescriptorPool, eweDevice->Device(), &descriptorPoolInfo, nullptr, &descriptorPool);

		// Descriptor set layout
		std::array<VkDescriptorSetLayoutBinding, 2> setLayoutBindings{};
		setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		setLayoutBindings[0].binding = 0;
		setLayoutBindings[0].descriptorCount = 1;

		setLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		setLayoutBindings[1].binding = 1;
		setLayoutBindings[1].descriptorCount = 1;
		setLayoutBindings[1].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.pNext = nullptr;
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
		descriptorSetLayoutInfo.bindingCount = setLayoutBindings.size();

		//std::cout << "vkcreatedescriptorsetlayout return pre " << std::endl;

		EWE_VK(vkCreateDescriptorSetLayout, VK::Object->vkDevice, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout);
		//std::cout << "vkcreatedescriptorsetlayout return : " << printInt << std::endl;

		// Pipeline layout
		//std::cout << "pipeline info1??" << std::endl;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		//std::cout << "pipelineinfo 3" << std::endl;

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout);
#if DEBUG_NAMING
		DebugNaming::SetObjectName(pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "textoverlay pipe layout");
#endif
		//std::cout << "pipeline info2??" << std::endl;

		// Descriptor set
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
		descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.pNext = nullptr;
		descriptorSetAllocInfo.descriptorPool = EWEDescriptorPool::GetPool(DescriptorPool_Global);
		descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
		descriptorSetAllocInfo.descriptorSetCount = 1;

		//std::cout << "check 2" << std::endl;

		EWE_VK(vkAllocateDescriptorSets, VK::Object->vkDevice, &descriptorSetAllocInfo, &descriptorSet[0]);
		EWE_VK(vkAllocateDescriptorSets, VK::Object->vkDevice, &descriptorSetAllocInfo, &descriptorSet[1]);

		VkWriteDescriptorSet writeDescriptorSets[4];
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].pNext = nullptr;
		writeDescriptorSets[0].dstSet = descriptorSet[0];
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].pBufferInfo = vertexBuffer[0]->DescriptorInfo();
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[0].dstArrayElement = 0;

		VkDescriptorImageInfo texDescriptorInfo{};
		texDescriptorInfo.sampler = sampler;
		texDescriptorInfo.imageView = view;
		texDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[1].pNext = nullptr;
		writeDescriptorSets[1].dstSet = descriptorSet[0];
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[1].dstBinding = 1;
		writeDescriptorSets[1].pImageInfo = &texDescriptorInfo;
		writeDescriptorSets[1].descriptorCount = 1;
		writeDescriptorSets[1].dstArrayElement = 0;

		writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[2].pNext = nullptr;
		writeDescriptorSets[2].dstSet = descriptorSet[1];
		writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[2].dstBinding = 0;
		writeDescriptorSets[2].pBufferInfo = vertexBuffer[1]->DescriptorInfo();
		writeDescriptorSets[2].descriptorCount = 1;
		writeDescriptorSets[2].dstArrayElement = 0;

		writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[3].pNext = nullptr;
		writeDescriptorSets[3].dstSet = descriptorSet[1];
		writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[3].dstBinding = 1;
		writeDescriptorSets[3].pImageInfo = &texDescriptorInfo;
		writeDescriptorSets[3].descriptorCount = 1;
		writeDescriptorSets[3].dstArrayElement = 0;

		//std::cout << "check4 " << std::endl;

		EWE_VK(vkUpdateDescriptorSets, VK::Object->vkDevice, 4, writeDescriptorSets, 0, nullptr);

		//std::cout << "check5" << std::endl;
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
		//printf("after blend attachment state \n");

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		inputAssemblyState.flags = 0;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;
		//printf("after input assembly state \n");
		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.flags = 0;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;
		//printf("after rrasterization state \n");
		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;
		//printf("after color blend state \n");
		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		//printf("after depth stencil state \n");
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.flags = 0;
		//printf("after viewport state \n");
		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.flags = 0;
		//printf("after multisample state \n");
		VkDynamicState dynamicStateEnables[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables;
		dynamicState.dynamicStateCount = 2;
		dynamicState.flags = 0;
		//printf("after dynamic state enables \n");
		VkVertexInputBindingDescription vertexInputBindings;
		vertexInputBindings.binding = 0;
		vertexInputBindings.stride = sizeof(glm::vec4);
		vertexInputBindings.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		//printf("after vertex input binding \n");
		VkVertexInputAttributeDescription vertexInputAttributes;
		vertexInputAttributes.location = 0;
		vertexInputAttributes.binding = 0;
		vertexInputAttributes.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		vertexInputAttributes.offset = 0;
		//printf("after vertex input attributes \n");

		VkPipelineVertexInputStateCreateInfo vertexInputState{};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &vertexInputBindings;
		vertexInputState.vertexAttributeDescriptionCount = 1;
		vertexInputState.pVertexAttributeDescriptions = &vertexInputAttributes;

		//printf("after vertex input state \n");
		auto vertCode = Pipeline_Helper_Functions::ReadFile("textoverlay.vert.spv");
		//printf("after vert code read file \n");
		auto fragCode = Pipeline_Helper_Functions::ReadFile("textoverlay.frag.spv");
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
		//printf("after successfully creating another shader module \n");
		//EWEPipeline::createShaderModule(vertCode, &vertShaderModule);
		//EWEPipeline::createShaderModule(fragCode, &fragShaderModule);
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

		//std::make_unique<EWEPipeline>(eweDevice, "texture_shader.vert.spv", "texture_shader.frag.spv", pipelineConfig);

		EWE_VK(vkCreateGraphicsPipelines, VK::Object->vkDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline);
#if DEBUG_NAMING
		DebugNaming::SetObjectName(pipeline, VK_OBJECT_TYPE_PIPELINE, "textoverlay pipeline");
#endif
		//printf("successfully created textoverlay graphics pipeline \n");
		//printf("end of text overlay constructor \n");
	}

	void TextOverlay::AddDefaultText(double time, double peakTime, double averageTime, double highTime) {
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
	}

	float TextOverlay::GetWidth(std::string text, float textScale) {
		const uint32_t firstChar = STB_FONT_consolas_24_latin1_FIRST_CHAR;
		//std::cout << "yo? : " << frameBufferWidth << std::endl;
		const float charW = 1.5f * scale * textScale / frameBufferWidth;
		float textWidth = 0;
		for (auto const& letter : text)
		{
			stb_fontchar* charData = &stbFontData[static_cast<uint32_t>(letter) - firstChar];
			textWidth += charData->advance * charW;
		}
		return textWidth;
	}
	void TextOverlay::StaticAddText(TextStruct textStruct) {
		textOverlayPtr->AddText(textStruct);
	}

	void TextOverlay::AddText(TextStruct textStruct, const float scaleX) {
		const uint32_t firstChar = STB_FONT_consolas_24_latin1_FIRST_CHAR;

		assert(mapped != nullptr);
		//std::cout << "frameBufferHeight : " << frameBufferHeight << std::endl;
		const float charW = 1.5f * scale * scaleX * textStruct.scale / frameBufferWidth;
		const float charH = 1.5f * scale * textStruct.scale / frameBufferHeight;

		textStruct.x = (textStruct.x / frameBufferWidth * 2.0f) - 1.0f;
		textStruct.y = (textStruct.y / frameBufferHeight * 2.0f) - 1.0f;

		// Calculate text width
		float textWidth = 0.f;
		for (auto const& letter : textStruct.string) {
			stb_fontchar* charData = &stbFontData[static_cast<uint32_t>(letter) - firstChar];
			textWidth += charData->advance * charW;
		}

		switch (textStruct.align) {
			case TA_right:
				textStruct.x -= textWidth;
				break;
			case TA_center:
				textStruct.x -= textWidth / 2.0f;
				break;
			case TA_left:
				break;
		}



		// Generate a uv mapped quad per char in the new text
		for (auto const& letter : textStruct.string) {
			stb_fontchar* charData = &stbFontData[static_cast<uint32_t>(letter) - firstChar];

			mapped->x = (textStruct.x + static_cast<float>(charData->x0) * charW);
			mapped->y = (textStruct.y + static_cast<float>(charData->y0) * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t0;
			mapped++;

			mapped->x = (textStruct.x + static_cast<float>(charData->x1) * charW);
			mapped->y = (textStruct.y + static_cast<float>(charData->y0) * charH);
			mapped->z = charData->s1;
			mapped->w = charData->t0;
			mapped++;

			mapped->x = (textStruct.x + static_cast<float>(charData->x0) * charW);
			mapped->y = (textStruct.y + static_cast<float>(charData->y1) * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t1;
			mapped++;

			mapped->x = (textStruct.x + static_cast<float>(charData->x1) * charW);
			mapped->y = (textStruct.y + static_cast<float>(charData->y1) * charH);
			mapped->z = charData->s1;
			mapped->w = charData->t1;
			mapped++;

			textStruct.x += charData->advance * charW;

			numLetters++;
		}

		//return textWidth;
	}

	void TextOverlay::Draw() {
			EWERenderer::BindGraphicsPipeline(pipeline);

			EWE_VK(vkCmdBindDescriptorSets, VK::Object->GetFrameBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet[VK::Object->frameIndex], 0, nullptr);

			//EWE_VK(vkCmdBindVertexBuffers, commandBuffer, 0, 1, &vertexBuffer, &offsets);
			//EWE_VK(vkCmdBindVertexBuffers, commandBuffer, 1, 1, &vertexBuffer, &offsets);
			EWE_VK(vkCmdDraw, VK::Object->GetFrameBuffer(), 4, numLetters, 0, 0);
			//for (uint32_t j = 0; j < numLetters; j++) {
			//	EWE_VK(vkCmdDraw, commandBuffer, 4, 1, j * 4, 0);
			//}

			/*
			vkCmdEndRenderPass(cmdBuffers[bufferIndex]);

			if (vkEndCommandBuffer(cmdBuffers[bufferIndex]) != VK_SUCCESS) {
				throw std::runtime_error("failed to end command buffer!");
			}
			*/
	}

	void TextOverlay::BeginTextUpdate() {
		vertexBuffer[VK::Object->frameIndex]->Map();
		mapped = reinterpret_cast<glm::vec4*>(vertexBuffer[VK::Object->frameIndex]->GetMappedMemory());
		numLetters = 0;
	}

	void TextOverlay::EndTextUpdate() {
		vertexBuffer[VK::Object->frameIndex]->Flush();
		vertexBuffer[VK::Object->frameIndex]->Unmap();
		mapped = nullptr;
		Draw();
	}
}
