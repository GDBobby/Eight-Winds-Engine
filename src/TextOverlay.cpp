
#include "EWEngine/Graphics/Renderer.h"
#include "EWEngine/Graphics/TextOverlay.h"

#include <stdexcept>
//#include <iostream>
#include <sstream>
#include <iomanip>


namespace EWE {

	stb_fontchar TextOverlay::stbFontData[STB_FONT_consolas_24_latin1_NUM_CHARS];

	TextOverlay* TextOverlay::textOverlayPtr{ nullptr };


	TextOverlay::TextOverlay(
		float framebufferwidth,
		float framebufferheight,
		VkPipelineRenderingCreateInfo const& pipelineInfo
	) : frameBufferWidth{ framebufferwidth }, frameBufferHeight{ framebufferheight }, scale{ frameBufferWidth / DEFAULT_WIDTH }
	{
		if (textOverlayPtr == nullptr) {
			textOverlayPtr = this;
		}
		else {
			printf("double initializing textoverlay \n");
			throw std::runtime_error("double init on textoverlay");
		}
		//printf("text overlay construction \n");

		cmdBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		prepareResources();
		//printf("after prepare resources \n");
		preparePipeline(pipelineInfo);
		//printf("afterr prepare pipeline \n");
	}

	TextOverlay::~TextOverlay() {
		// Free up all Vulkan resources requested by the text overlay
#if DECONSTRUCTION_DEBUG
		printf("deconstrructing textoverlay \n");
#endif
		VkDevice const& vkDevice = EWEDevice::GetVkDevice();
		vkDestroySampler(vkDevice, sampler, nullptr);
		vkDestroyImage(vkDevice, image, nullptr);
		vkDestroyImageView(vkDevice, view, nullptr);
		vkDestroyBuffer(vkDevice, buffer, nullptr);
		vkFreeMemory(vkDevice, memory, nullptr);
		vkFreeMemory(vkDevice, imageMemory, nullptr);
		vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);
		vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);
		vkDestroyDescriptorSetLayout(vkDevice, descriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(vkDevice, descriptorPool, nullptr);
		vkDestroyPipelineLayout(vkDevice, pipelineLayout, nullptr);
		vkDestroyPipelineCache(vkDevice, pipelineCache, nullptr);
		vkDestroyPipeline(vkDevice, pipeline, nullptr);

#if DECONSTRUCTION_DEBUG
		printf("end deconstruction textoverlay \n");
#endif

	}


	uint16_t TextStruct::getSelectionIndex(double xpos, float screenWidth) {
		const float charW = 1.5f * scale / screenWidth;
		float width = getWidth(screenWidth);
		float currentPos = x;
		stb_fontchar* charData = &TextOverlay::stbFontData[(uint32_t)string.back() - STB_FONT_consolas_24_latin1_FIRST_CHAR];
		printf("xpos get selection index - %.1f \n", xpos);
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
			charData = &TextOverlay::stbFontData[(uint32_t)string[i] - STB_FONT_consolas_24_latin1_FIRST_CHAR];
			currentPos += (charData->advance * charW) * screenWidth / 8.f;
			printf("currentPos : %.2f \n", currentPos);
			if (xpos <= currentPos) { return i; }
			currentPos += (charData->advance * charW) * screenWidth * 3.f / 8.f;
		}
		return static_cast<uint16_t>(string.length());
	}
	float TextStruct::getWidth(float screenWidth) {
		//std::cout << "yo? : " << frameBufferWidth << std::endl;
		const float charW = 1.5f * scale / screenWidth;
		float textWidth = 0;
		stb_fontchar* charData;
		for (auto letter : string) {
			charData = &TextOverlay::stbFontData[(uint32_t)letter - STB_FONT_consolas_24_latin1_FIRST_CHAR];
			textWidth += charData->advance * charW;
		}
		//printf("text struct get width : %.5f \n", textWidth);
		if (textWidth < 0.0f) {
			printf("width less than 0, what  was the string? : %s:%.1f \n", string.c_str(), screenWidth);
		}
		return textWidth;
	}

	void TextOverlay::prepareResources() {
		const uint32_t fontWidth = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;
		const uint32_t fontHeight = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;

		static unsigned char font24pixels[fontWidth][fontHeight];
		stb_font_consolas_24_latin1(stbFontData, font24pixels, fontHeight);

		// Vertex buffer
		VkDeviceSize bufferSize = TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4);

		VkCommandBufferAllocateInfo cmdBuffAllocateInfo;
		cmdBuffAllocateInfo.pNext = nullptr;
		cmdBuffAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBuffAllocateInfo.commandPool = EWEDevice::GetEWEDevice()->getTransferCommandPool();
		cmdBuffAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBuffAllocateInfo.commandBufferCount = (uint32_t)cmdBuffers.size();

		if (vkAllocateCommandBuffers(EWEDevice::GetVkDevice(), &cmdBuffAllocateInfo, cmdBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to alocate cmd bfuer!");
		}

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.size = bufferSize;
		if (vkCreateBuffer(EWEDevice::GetVkDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create cmd bfuer!");
		}

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(EWEDevice::GetVkDevice(), buffer, &memReqs);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = eweDevice.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//allocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(EWEDevice::GetVkDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS) {
			throw std::runtime_error("failed to alocate!");
		}
		vkBindBufferMemory(EWEDevice::GetVkDevice(), buffer, memory, 0);
		//std::cout << "bind buffer memory result : " << printInt << std::endl;
		/*
		if(vkBindBufferMemory(eweDevice.device(), buffer, memory, 0) != VK_SUCCESS) {
			throw std::runtime_error("failed to bind memory!");
		}
		*/

		// Font texture
		VkImageCreateInfo imageInfo;
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = nullptr;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8_UNORM;
		imageInfo.extent.width = fontWidth;
		imageInfo.extent.height = fontHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.flags = 0; //optional????

		//not using this function because i need the allocInfo
		//eweDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);


		if (vkCreateImage(eweDevice.device(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(eweDevice.device(), image, &memRequirements);
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = eweDevice.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(eweDevice.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		if (vkBindImageMemory(eweDevice.device(), image, imageMemory, 0) != VK_SUCCESS) {
			throw std::runtime_error("failed to bind image memory!");
		}


		// Staging
		struct {
			VkDeviceMemory memory;
			VkBuffer buffer;
		} stagingBuffer;

		eweDevice.createBuffer(allocInfo.allocationSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer.buffer,
			stagingBuffer.memory
		);


		uint8_t* data;
		if (vkMapMemory(eweDevice.device(), stagingBuffer.memory, 0, allocInfo.allocationSize, 0, (void**)&data) != VK_SUCCESS) {
			throw std::runtime_error("failed to map memory!");
		}
		// Size of the font texture is WIDTH * HEIGHT * 1 byte (only one channel)
		memcpy(data, &font24pixels[0][0], fontWidth * fontHeight);
		vkUnmapMemory(eweDevice.device(), stagingBuffer.memory);

		// Copy to image

		//this next little segement is similar to beginsingletimecommands
		VkCommandBuffer copyCmd;


		cmdBuffAllocateInfo.commandBufferCount = 1;
		if (vkAllocateCommandBuffers(eweDevice.device(), &cmdBuffAllocateInfo, &copyCmd) != VK_SUCCESS){
			throw std::runtime_error("failed to create command bfufer!");
		}

		VkCommandBufferBeginInfo cmdBufInfo{};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(copyCmd, &cmdBufInfo);
		//std::cout << "begin command bfufer result : " << printInt << std::endl;
		/*
		if ((vkBeginCommandBuffer(copyCmd, &cmdBufInfo)) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin??? command bfufer!");
		}
		*/

		// Prepare for transfer
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		eweDevice.setImageLayout(
			copyCmd,
			image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange
		);

		VkBufferImageCopy bufferCopyRegion{};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = fontWidth;
		bufferCopyRegion.imageExtent.height = fontHeight;
		bufferCopyRegion.imageExtent.depth = 1;

		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer.buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferCopyRegion
		);

		// Prepare for shader read <-- this part right here is why i cant use eweDevice.copyBufferToImage

		//im just winging it with these pipelinestageflags
		
		eweDevice.setImageLayout(
			copyCmd,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			subresourceRange
		);

		eweDevice.endSingleTimeCommands(copyCmd);

		vkFreeMemory(eweDevice.device(), stagingBuffer.memory, nullptr);
		vkDestroyBuffer(eweDevice.device(), stagingBuffer.buffer, nullptr);

		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = image;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = imageInfo.format;
		imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,	VK_COMPONENT_SWIZZLE_A };
		imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		if ((vkCreateImageView(eweDevice.device(), &imageViewInfo, nullptr, &view)) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image view!");
		}

		// Sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
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
		if ((vkCreateSampler(eweDevice.device(), &samplerInfo, nullptr, &sampler)) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image sampler!");
		}

		// Descriptor
		// Font uses a separate descriptor pool
		std::vector<VkDescriptorPoolSize> poolSizes(1);
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.pNext = nullptr;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 1;

		if ((vkCreateDescriptorPool(eweDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool)) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}

		// Descriptor set layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(1);
		setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		setLayoutBindings[0].binding = 0;
		setLayoutBindings[0].descriptorCount = 1;

		

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.pNext = nullptr;
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
		descriptorSetLayoutInfo.bindingCount = 1;

		//std::cout << "vkcreatedescriptorsetlayout return pre " << std::endl;

		vkCreateDescriptorSetLayout(eweDevice.device(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout);
		//std::cout << "vkcreatedescriptorsetlayout return : " << printInt << std::endl;

		/*
		if ((vkCreateDescriptorSetLayout(eweDevice.device(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout)) != VK_SUCCESS){
			//throw std::runtime_error("failed to create descriptor set layout!");
			
		}
		*/

		// Pipeline layout
		//std::cout << "pipeline info1??" << std::endl;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		//std::cout << "pipelineinfo 3" << std::endl;

		if ((vkCreatePipelineLayout(eweDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout)) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
		//std::cout << "pipeline info2??" << std::endl;


		// Descriptor set
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
		descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.descriptorPool = descriptorPool;
		descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
		descriptorSetAllocInfo.descriptorSetCount = 1;

		//std::cout << "check 2" << std::endl;

		if ((vkAllocateDescriptorSets(eweDevice.device(), &descriptorSetAllocInfo, &descriptorSet)) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		//std::cout << "check3" << std::endl;

		VkDescriptorImageInfo texDescriptor{};
			texDescriptor.sampler = sampler;
			texDescriptor.imageView = view;
			texDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::vector<VkWriteDescriptorSet> writeDescriptorSets(1);
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = descriptorSet;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].pImageInfo = &texDescriptor;
		writeDescriptorSets[0].descriptorCount = 1;

		//std::cout << "check4 " << std::endl;

		vkUpdateDescriptorSets(eweDevice.device(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);

		//std::cout << "check5" << std::endl;
		// Pipeline cache
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		if ((vkCreatePipelineCache(eweDevice.device(), &pipelineCacheCreateInfo, nullptr, &pipelineCache)) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline cache!");
		}


		//std::cout << "end of function" << std::endl;
	}
	void TextOverlay::preparePipeline(VkPipelineRenderingCreateInfo renderingInfo)
	{
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
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		dynamicState.flags = 0;
		//printf("after dynamic state enables \n");
		std::vector<VkVertexInputBindingDescription> vertexInputBindings(2);
		vertexInputBindings[0].binding = 0;
		vertexInputBindings[0].stride = sizeof(glm::vec4);
		vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputBindings[1].binding = 1;
		vertexInputBindings[1].stride = sizeof(glm::vec4);
		vertexInputBindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		//printf("after vertex input binding \n");
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes(2);
		vertexInputAttributes[0].location = 0;
		vertexInputAttributes[0].binding = 0;
		vertexInputAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributes[0].offset = 0;
		vertexInputAttributes[1].location = 1;
		vertexInputAttributes[1].binding = 1;
		vertexInputAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributes[1].offset = sizeof(glm::vec2);
		//printf("after vertex input attributes \n");
		VkPipelineVertexInputStateCreateInfo vertexInputState{};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
		vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();
		//printf("after vertex input state \n");
		auto vertCode = Pipeline_Helper_Functions::readFile("textoverlay.vert.spv");
		//printf("after vert code read file \n");
		auto fragCode = Pipeline_Helper_Functions::readFile("textoverlay.frag.spv");
		//printf("after frag code read file \n");
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.codeSize = vertCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(vertCode.data());
		createInfo.flags = 0;
		//printf("after shader module create info \n");
		if (vkCreateShaderModule(EWEDevice::GetVkDevice(), &createInfo, nullptr, &vertShaderModule) != VK_SUCCESS) {
			printf("vert vk create  shader module failed \n");
			throw std::runtime_error("failed to create shader module");
		}
		//printf("after successfully creating shader module \n");

		createInfo.codeSize = fragCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(fragCode.data());
		//printf("setting shader module create info to frag \n");
		if (vkCreateShaderModule(EWEDevice::GetVkDevice(), &createInfo, nullptr, &fragShaderModule) != VK_SUCCESS) {
			printf("frag vk create shader module failed \n");
			throw std::runtime_error("failed to create shader module");
		}
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

		if (vkCreateGraphicsPipelines(EWEDevice::GetVkDevice(), pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
			printf("failed to create graphics pipeline in textoverlay \n");
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		//printf("successfully created textoverlay graphics pipeline \n");
		//printf("end of text overlay constructor \n");
	}

	void TextOverlay::addDefaultText(double time, double peakTime, double averageTime, double highTime) {
		addText(TextStruct{ EWEDevice::GetEWEDevice()->deviceName, 0, frameBufferHeight - (20.f * scale), TA_left, 1.f});
		//printf("frameBuffer : %d : %d \n", frameBufferWidth, frameBufferHeight);
		int lastFPS = static_cast<int>(1 / time);
		int averageFPS = static_cast<int>(1 / averageTime);
		std::string buffer_string = std::format("frame time: {:.2f} ms ({} fps)", time * 1000.0f, lastFPS);
		addText(TextStruct{ buffer_string, 0.f, frameBufferHeight - (40.f * scale), TA_left, 1.f });
		buffer_string = std::format("average FPS: {}", averageFPS);
		addText(TextStruct{ buffer_string, 0.f, frameBufferHeight - (60.f * scale), TA_left, 1.f });
		buffer_string = std::format("peak: {:.2f} ms ~ average: {:.2f} ms ~ high: {:.2f} ms", peakTime * 1000, averageTime * 1000, highTime * 1000);
		addText(TextStruct{ buffer_string, 0.f, frameBufferHeight - (80.f * scale), TA_left, 1.f });
	}

	float TextOverlay::getWidth(std::string text, float textScale) {
		const uint32_t firstChar = STB_FONT_consolas_24_latin1_FIRST_CHAR;
		//std::cout << "yo? : " << frameBufferWidth << std::endl;
		const float charW = 1.5f * scale * textScale / frameBufferWidth;
		float textWidth = 0;
		for (auto letter : text)
		{
			stb_fontchar* charData = &stbFontData[(uint32_t)letter - firstChar];
			textWidth += charData->advance * charW;
		}
		return textWidth;
	}
	void TextOverlay::staticAddText(TextStruct textStruct) {
		textOverlayPtr->addText(textStruct);
	}

	void TextOverlay::addText(TextStruct textStruct) {
		const uint32_t firstChar = STB_FONT_consolas_24_latin1_FIRST_CHAR;

		assert(mapped != nullptr);
		//std::cout << "frameBufferHeight : " << frameBufferHeight << std::endl;
		const float charW = 1.5f * scale * textStruct.scale / frameBufferWidth;
		const float charH = 1.5f * scale * textStruct.scale / frameBufferHeight;

		float fbW = frameBufferWidth;
		float fbH = frameBufferHeight;
		textStruct.x = (textStruct.x / fbW * 2.0f) - 1.0f;
		textStruct.y = (textStruct.y / fbH * 2.0f) - 1.0f;

		// Calculate text width
		float textWidth = 0;
		for (auto letter : textStruct.string)
		{
			stb_fontchar* charData = &stbFontData[(uint32_t)letter - firstChar];
			textWidth += charData->advance * charW;
		}

		switch (textStruct.align)
		{
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
		for (auto letter : textStruct.string)
		{
			stb_fontchar* charData = &stbFontData[(uint32_t)letter - firstChar];

			mapped->x = (textStruct.x + (float)charData->x0 * charW);
			mapped->y = (textStruct.y + (float)charData->y0 * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t0;
			mapped++;

			mapped->x = (textStruct.x + (float)charData->x1 * charW);
			mapped->y = (textStruct.y + (float)charData->y0 * charH);
			mapped->z = charData->s1;
			mapped->w = charData->t0;
			mapped++;

			mapped->x = (textStruct.x + (float)charData->x0 * charW);
			mapped->y = (textStruct.y + (float)charData->y1 * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t1;
			mapped++;

			mapped->x = (textStruct.x + (float)charData->x1 * charW);
			mapped->y = (textStruct.y + (float)charData->y1 * charH);
			mapped->z = charData->s1;
			mapped->w = charData->t1;
			mapped++;

			textStruct.x += charData->advance * charW;

			numLetters++;
		}

		//return textWidth;
	}
	
	void TextOverlay::addTextEx(TextStruct textStruct, float scaleX) {
		const uint32_t firstChar = STB_FONT_consolas_24_latin1_FIRST_CHAR;

		assert(mapped != nullptr);
		//std::cout << "frameBufferHeight : " << frameBufferHeight << std::endl;
		const float charW = 1.5f * scale * scaleX / frameBufferWidth;
		const float charH = 1.5f * scale * textStruct.scale / frameBufferHeight;

		float fbW = frameBufferWidth;
		float fbH = frameBufferHeight;
		textStruct.x = (textStruct.x / fbW * 2.0f) - 1.0f;
		textStruct.y = (textStruct.y / fbH * 2.0f) - 1.0f;

		// Calculate text width
		float textWidth = 0;
		for (auto letter : textStruct.string) {
			stb_fontchar* charData = &stbFontData[(uint32_t)letter - firstChar];
			textWidth += charData->advance * charW;
		}

		switch (textStruct.align)
		{
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
		for (auto letter : textStruct.string) {
			stb_fontchar* charData = &stbFontData[(uint32_t)letter - firstChar];

			mapped->x = (textStruct.x + (float)charData->x0 * charW);
			mapped->y = (textStruct.y + (float)charData->y0 * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t0;
			mapped++;

			mapped->x = (textStruct.x + (float)charData->x1 * charW);
			mapped->y = (textStruct.y + (float)charData->y0 * charH);
			mapped->z = charData->s1;
			mapped->w = charData->t0;
			mapped++;

			mapped->x = (textStruct.x + (float)charData->x0 * charW);
			mapped->y = (textStruct.y + (float)charData->y1 * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t1;
			mapped++;

			mapped->x = (textStruct.x + (float)charData->x1 * charW);
			mapped->y = (textStruct.y + (float)charData->y1 * charH);
			mapped->z = charData->s1;
			mapped->w = charData->t1;
			mapped++;

			textStruct.x += charData->advance * charW;

			numLetters++;
		}

		//return textWidth;
	}


	VkCommandBuffer TextOverlay::beginBuffer(int bufferIndex) {
		VkCommandBufferBeginInfo cmdBufInfo{};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(cmdBuffers[bufferIndex], &cmdBufInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin command buffer!");
		}

		return cmdBuffers[bufferIndex];
	}

	void TextOverlay::draw(VkCommandBuffer commandBuffer) {
			EWERenderer::bindGraphicsPipeline(commandBuffer, pipeline);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

			VkDeviceSize offsets = 0;
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offsets);
			vkCmdBindVertexBuffers(commandBuffer, 1, 1, &buffer, &offsets);
			for (uint32_t j = 0; j < numLetters; j++) {
				vkCmdDraw(commandBuffer, 4, 1, j * 4, 0);
			}

			/*
			vkCmdEndRenderPass(cmdBuffers[bufferIndex]);

			if (vkEndCommandBuffer(cmdBuffers[bufferIndex]) != VK_SUCCESS) {
				throw std::runtime_error("failed to end command buffer!");
			}
			*/
	}

	void TextOverlay::beginTextUpdate() {
		if (vkMapMemory(EWEDevice::GetVkDevice(), memory, 0, VK_WHOLE_SIZE, 0, (void**)&mapped) != VK_SUCCESS) {
			throw std::runtime_error("failed to map memory!");
		};
		numLetters = 0;
	}

	void TextOverlay::endTextUpdate() {
		vkUnmapMemory(EWEDevice::GetVkDevice(), memory);
		mapped = nullptr;
	}
}
