/*
#include "Cloth.h"

#include <iostream>

namespace lve {
	void Cloth::addGraphicsToComputeBarriers(VkCommandBuffer commandBuffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {
		if (specializedComputeQueue) {


			VkBufferMemoryBarrier bufferBarrier{};// = vks::initializers::bufferMemoryBarrier();
			bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			bufferBarrier.srcAccessMask = srcAccessMask;
			bufferBarrier.dstAccessMask = dstAccessMask;
			bufferBarrier.srcQueueFamilyIndex = lveDevice.getGraphicsIndex();
			bufferBarrier.dstQueueFamilyIndex = lveDevice.getComputeIndex();
			bufferBarrier.size = VK_WHOLE_SIZE;

			std::vector<VkBufferMemoryBarrier> bufferBarriers;
			bufferBarrier.buffer = compute.storageBuffers.inputBuffer->getBuffer();
			bufferBarriers.push_back(bufferBarrier);
			bufferBarrier.buffer = compute.storageBuffers.outputBuffer->getBuffer();
			bufferBarriers.push_back(bufferBarrier);
			vkCmdPipelineBarrier(commandBuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				static_cast<uint32_t>(bufferBarriers.size()), bufferBarriers.data(),
				0, nullptr);
		}
	}

	void Cloth::addComputeToComputeBarriers(VkCommandBuffer commandBuffer) {
		VkBufferMemoryBarrier bufferBarrier{};// = vks::initializers::bufferMemoryBarrier();
		bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier.size = VK_WHOLE_SIZE;
		std::vector<VkBufferMemoryBarrier> bufferBarriers;
		bufferBarrier.buffer = compute.storageBuffers.inputBuffer->getBuffer();
		bufferBarriers.push_back(bufferBarrier);
		bufferBarrier.buffer = compute.storageBuffers.outputBuffer->getBuffer();
		//compute.storageBuffers.inputBuffer->
		bufferBarriers.push_back(bufferBarrier);
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0,
			0, nullptr,
			static_cast<uint32_t>(bufferBarriers.size()), bufferBarriers.data(),
			0, nullptr);
	}
	void Cloth::addComputeToGraphicsBarriers(VkCommandBuffer commandBuffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {
		if (specializedComputeQueue) {
			VkBufferMemoryBarrier bufferBarrier{};// = vks::initializers::bufferMemoryBarrier();
			bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			bufferBarrier.srcAccessMask = srcAccessMask;
			bufferBarrier.dstAccessMask = dstAccessMask;
			bufferBarrier.dstQueueFamilyIndex = lveDevice.getComputeIndex();
			bufferBarrier.srcQueueFamilyIndex = lveDevice.getGraphicsIndex();
			bufferBarrier.size = VK_WHOLE_SIZE;
			std::vector<VkBufferMemoryBarrier> bufferBarriers;
			bufferBarrier.buffer = compute.storageBuffers.inputBuffer->getBuffer();
			bufferBarriers.push_back(bufferBarrier);
			bufferBarrier.buffer = compute.storageBuffers.outputBuffer->getBuffer();
			bufferBarriers.push_back(bufferBarrier);
			vkCmdPipelineBarrier(
				commandBuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				static_cast<uint32_t>(bufferBarriers.size()), bufferBarriers.data(),
				0, nullptr);
		}
	}
	void Cloth::buildCommandBuffers(VkCommandBuffer drawCmdBuffers) {
		//std::cout << "beginning of build command buffers" << std::endl;
		VkCommandBufferBeginInfo cmdBufInfo{};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// Set target frame buffer
		//renderPassBeginInfo.framebuffer = frameBuffers[i];/

		// Acquire storage buffers from compute queue
		addComputeToGraphicsBarriers(drawCmdBuffers, 0, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);

		// Draw the particle system using the update vertex buffer

		//vkCmdBeginRenderPass(drawCmdBuffers, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkDeviceSize offsets[1] = { 0 };

		// Render sphere


		// Render cloth
		//vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.clothPipeline);
		graphics.clothPipeline->bind(drawCmdBuffers);
		vkCmdBindDescriptorSets(drawCmdBuffers, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, &graphics.descriptorSet, 0, NULL);
		vkCmdBindIndexBuffer(drawCmdBuffers, graphics.indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		VkBuffer tempBuffer = compute.storageBuffers.outputBuffer->getBuffer();
		vkCmdBindVertexBuffers(drawCmdBuffers, 0, 1, &tempBuffer, offsets);
		vkCmdDrawIndexed(drawCmdBuffers, indexCount, 1, 0, 0, 0);

		//drawUI(drawCmdBuffers[i]);

		//vkCmdEndRenderPass(drawCmdBuffers[i]);
		

		// release the storage buffers to the compute queue
		addGraphicsToComputeBarriers(drawCmdBuffers, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, 0, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		//vkEndCommandBuffer(drawCmdBuffers);
		//VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers));

		//std::cout << "end of this 69" << std::endl;
	}

	void Cloth::buildComputeCommandBuffer(VkCommandBuffer cmdBuffer) {
		//std::cout << "beginning of build compute command buffer" << std::endl;
		VkCommandBufferBeginInfo cmdBufInfo{}; //= vks::initializers::commandBufferBeginInfo();
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		//VkCommandBuffer tempCmdBuff;
			//compute.commandBuffers[i].
			
		//tempCmdBuff = compute.commandBuffers[i];
		
		if (!prepared) {
			if (vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to create pipeline layout");
			}
		}
			
		// Acquire the storage buffers from the graphics queue
		addGraphicsToComputeBarriers(cmdBuffer, 0, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipeline);

		uint32_t calculateNormals = 0;
		vkCmdPushConstants(cmdBuffer, compute.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &calculateNormals);

		// Dispatch the compute job
		const uint32_t iterations = 64;
		for (uint32_t j = 0; j < iterations; j++) {
			readSet = 1 - readSet;
			vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout, 0, 1, &compute.descriptorSets[readSet], 0, 0);

			if (j == iterations - 1) {
				calculateNormals = 1;
				vkCmdPushConstants(cmdBuffer, compute.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &calculateNormals);
			}

			vkCmdDispatch(cmdBuffer, clothStruct.gridsize.x / 10, clothStruct.gridsize.y / 10, 1);

			// Don't add a barrier on the last iteration of the loop, since we'll have an explicit release to the graphics queue
			if (j != iterations - 1) {
				addComputeToComputeBarriers(cmdBuffer);
			}

		}

		// release the storage buffers back to the graphics queue
		addComputeToGraphicsBarriers(cmdBuffer, VK_ACCESS_SHADER_WRITE_BIT, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		
		//vkEndCommandBuffer(cmdBuffer); ~?
		
		//std::cout << "end of this 2" << std::endl;
	}

	void Cloth::prepareStorageBuffers() {
		std::vector<Particle> particleBuffer(clothStruct.gridsize.x * clothStruct.gridsize.y);

		float dx = clothStruct.size.x / (clothStruct.gridsize.x - 1);
		float dy = clothStruct.size.y / (clothStruct.gridsize.y - 1);
		float du = 1.0f / (clothStruct.gridsize.x - 1);
		float dv = 1.0f / (clothStruct.gridsize.y - 1);

		switch (sceneSetup) {
		case 0:
		{
			// Horz. cloth falls onto sphere
			glm::mat4 transM = glm::translate(glm::mat4(1.0f), glm::vec3(-clothStruct.size.x / 2.0f, -2.0f, -clothStruct.size.y / 2.0f));
			for (uint32_t i = 0; i < clothStruct.gridsize.y; i++) {
				for (uint32_t j = 0; j < clothStruct.gridsize.x; j++) {
					particleBuffer[i + j * clothStruct.gridsize.y].pos = transM * glm::vec4(dx * j, 0.0f, dy * i, 1.0f);
					particleBuffer[i + j * clothStruct.gridsize.y].vel = glm::vec4(0.0f);
					particleBuffer[i + j * clothStruct.gridsize.y].uv = glm::vec4(1.0f - du * i, dv * j, 0.0f, 0.0f);
				}
			}
			break;
		}
		case 1:
		{
			// Vert. Pinned cloth
			glm::mat4 transM = glm::translate(glm::mat4(1.0f), glm::vec3(-clothStruct.size.x / 2.0f, -clothStruct.size.y / 2.0f, 0.0f));
			for (uint32_t i = 0; i < clothStruct.gridsize.y; i++) {
				for (uint32_t j = 0; j < clothStruct.gridsize.x; j++) {
					particleBuffer[i + j * clothStruct.gridsize.y].pos = transM * glm::vec4(dx * j, dy * i, 0.0f, 1.0f);
					particleBuffer[i + j * clothStruct.gridsize.y].vel = glm::vec4(0.0f);
					particleBuffer[i + j * clothStruct.gridsize.y].uv = glm::vec4(du * j, dv * i, 0.0f, 0.0f);
					// Pin some particles
					particleBuffer[i + j * clothStruct.gridsize.y].pinned = (i == 0) && ((j == 0) || (j == clothStruct.gridsize.x / 3) || (j == clothStruct.gridsize.x - clothStruct.gridsize.x / 3) || (j == clothStruct.gridsize.x - 1));
					// Remove sphere
					compute.ubo.spherePos.z = -10.0f;
				}
			}
			break;
		}
		}

		VkDeviceSize storageBufferSize = particleBuffer.size() * sizeof(Particle);

		// Staging
		// SSBO won't be changed on the host after upload so copy to device local memory

		LveBuffer stagingBuffer(lveDevice, storageBufferSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.map();
		stagingBuffer.writeToBuffer(particleBuffer.data());
		//std::cout << "1" << std::endl;
		//stagingBuffer.


		compute.storageBuffers.inputBuffer = std::make_unique<LveBuffer>(lveDevice, storageBufferSize, 1, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		compute.storageBuffers.inputBuffer->map();
		//std::cout << "2" << std::endl;
		compute.storageBuffers.outputBuffer = std::make_unique<LveBuffer>(lveDevice, storageBufferSize, 1, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		compute.storageBuffers.outputBuffer->map();
		//std::cout << "3" << std::endl;

		// Copy from staging buffer

		VkCommandBufferAllocateInfo cmdBuffAllocateInfo;
		cmdBuffAllocateInfo.pNext = nullptr;
		cmdBuffAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBuffAllocateInfo.commandPool = lveDevice.getCommandPool();
		cmdBuffAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBuffAllocateInfo.commandBufferCount = 1;

		VkCommandBuffer copyCmd{};// = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		vkAllocateCommandBuffers(lveDevice.device(), &cmdBuffAllocateInfo, &copyCmd);
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(copyCmd, &beginInfo);

		//std::cout << "4" << std::endl;
		VkBufferCopy copyRegion = {};
		copyRegion.size = storageBufferSize;
		//lveDevice.copyBuffer()
		vkCmdCopyBuffer(copyCmd, stagingBuffer.getBuffer(), compute.storageBuffers.inputBuffer->getBuffer(), 1, &copyRegion);
		//std::cout << "5" << std::endl;
		vkCmdCopyBuffer(copyCmd, stagingBuffer.getBuffer(), compute.storageBuffers.outputBuffer->getBuffer(), 1, &copyRegion);
		//std::cout << "6" << std::endl;
		// Add an initial release barrier to the graphics queue,
		// so that when the compute command buffer executes for the first time
		// it doesn't complain about a lack of a corresponding "release" to its "acquire"
		addGraphicsToComputeBarriers(copyCmd, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, 0, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		//vulkanDevice->flushCommandBuffer(copyCmd, queue, true);
		vkEndCommandBuffer(copyCmd);

		VkSubmitInfo submitInfo{};// = vks::initializers::submitInfo();
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &copyCmd;
		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceInfo{};// = vks::initializers::fenceCreateInfo(VK_FLAGS_NONE);
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;
		VkFence fence;
		if (vkCreateFence(lveDevice.device(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
			throw std::runtime_error("failed to create fence");
		}
		// Submit to the queue
		if (vkQueueSubmit(lveDevice.computeQueue(), 1, &submitInfo, fence) != VK_SUCCESS) {
			throw std::runtime_error("failed to queue submit");
		}
		// Wait for the fence to signal that command buffer has finished executing
		if (vkWaitForFences(lveDevice.device(), 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT) != VK_SUCCESS) {
			throw std::runtime_error("failed to create fence");
		}
		vkDestroyFence(lveDevice.device(), fence, nullptr);
		if (free)
		{
			vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), 1, &copyCmd);
		}

		//stagingBuffer.~LveBuffer(); ~ this bugs the app

		// Indices
		std::vector<uint32_t> indices;
		for (uint32_t y = 0; y < clothStruct.gridsize.y - 1; y++) {
			for (uint32_t x = 0; x < clothStruct.gridsize.x; x++) {
				indices.push_back((y + 1) * clothStruct.gridsize.x + x);
				indices.push_back((y)*clothStruct.gridsize.x + x);
			}
			// Primitive restart (signaled by special value 0xFFFFFFFF)
			indices.push_back(0xFFFFFFFF);
		}
		uint32_t indexBufferSize = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);
		indexCount = static_cast<uint32_t>(indices.size());


		LveBuffer stagingBuffer2(lveDevice, indexBufferSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//std::cout << "7" << std::endl;
		stagingBuffer2.map();
		//std::cout << "8" << std::endl;
		stagingBuffer2.writeToBuffer(indices.data());
		//std::cout << "9" << std::endl;
		graphics.indexBuffer = std::make_unique<LveBuffer>(lveDevice, indexBufferSize, 1, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		//std::cout << "10" << std::endl;
		graphics.indexBuffer->map();
		graphics.indexBuffer->writeToBuffer(indices.data());
		//std::cout << "12" << std::endl;

		// Copy from staging buffer
		//copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		vkAllocateCommandBuffers(lveDevice.device(), &cmdBuffAllocateInfo, &copyCmd);
		vkBeginCommandBuffer(copyCmd, &beginInfo); //the beginInfo above is reusable?
		copyRegion = {};
		copyRegion.size = indexBufferSize;
		vkCmdCopyBuffer(copyCmd, stagingBuffer2.getBuffer(), graphics.indexBuffer->getBuffer(), 1, &copyRegion);
		//lveDevice.copyBuffer()
		
		//vulkanDevice->flushCommandBuffer(copyCmd, queue, true);
		//std::cout << "13" << std::endl;
		vkEndCommandBuffer(copyCmd);

		VkSubmitInfo submitInfoFlush{};// = vks::initializers::submitInfo();
		submitInfoFlush.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfoFlush.commandBufferCount = 1;
		submitInfoFlush.pCommandBuffers = &copyCmd;
		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceInfoFlush{};// = vks::initializers::fenceCreateInfo(VK_FLAGS_NONE);
		fenceInfoFlush.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfoFlush.flags = 0;
		VkFence fenceFlush;
		if (vkCreateFence(lveDevice.device(), &fenceInfoFlush, nullptr, &fenceFlush) != VK_SUCCESS) {
			throw std::runtime_error("failed to create fence");
		}
		// Submit to the queue
		if (vkQueueSubmit(lveDevice.computeQueue(), 1, &submitInfoFlush, fenceFlush) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit queue");
		}
		// Wait for the fence to signal that command buffer has finished executing
		if (vkWaitForFences(lveDevice.device(), 1, &fenceFlush, VK_TRUE, DEFAULT_FENCE_TIMEOUT) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module");
		}
		vkDestroyFence(lveDevice.device(), fenceFlush, nullptr);
		if (free)
		{
			vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), 1, &copyCmd);
		}
		//std::cout << "14" << std::endl;
		
		//stagingBuffer.destroy();
	}

	void Cloth::setupLayoutsAndDescriptors() {
		//std::cout << "beginning of layouts and descriptors" << std::endl;
		// Set layout

		graphics.descriptorSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		VkDescriptorSetLayout tempSetLayout = graphics.descriptorSetLayout->getDescriptorSetLayout();
		pipelineLayoutInfo.pSetLayouts = &tempSetLayout;

		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &graphics.pipelineLayout) != VK_SUCCESS) {
			//std::cout << "failed to create pipeline layout" << std::endl;
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void Cloth::preparePipelines(VkPipelineRenderingCreateInfo const& pipeRenderInfo) {
		//std::cout << "beginning of prepare pipelines" << std::endl;
		PipelineConfigInfo lveConfig{};
		LvePipeline::defaultPipelineConfigInfo(lveConfig);
		lveConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		lveConfig.inputAssemblyInfo.primitiveRestartEnable = VK_TRUE;

		lveConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // experiement with this later

		lveConfig.colorBlendAttachment.colorWriteMask = 0xf; // experiment with this too

		lveConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		std::vector<VkVertexInputBindingDescription> inputBindings = { {0, sizeof(Particle), VK_VERTEX_INPUT_RATE_VERTEX} };

		lveConfig.bindingDescriptions = inputBindings;
		std::vector<VkVertexInputAttributeDescription> inputAttributes = {
			{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Particle, pos)},
			{1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Particle, uv)},
			{2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Particle, normal)}
		};
		lveConfig.attributeDescriptions = inputAttributes;
		//lveConfig.attributeDescriptions.

		VkPipelineLayoutCreateInfo graphicsLayoutInfo{};
		graphicsLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		lveConfig.pipelineLayout = graphics.pipelineLayout;
		lveConfig.pipelineRenderingInfo = pipeRenderInfo;
		
		
		graphics.clothPipeline = std::make_unique<LvePipeline>(lveDevice, "shaders/cloth.vert.spv", "shaders/cloth.frag.spv", lveConfig);
		//std::cout << "right after cloth pipeline" << std::endl;
		// Rendering pipeline


		//VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(graphics.pipelineLayout, renderPass);
	}

	void Cloth::prepareCompute() {
		//std::cout << "beginning of prepare compute" << std::endl;
		// Create a compute capable device queue
		compute.queue = lveDevice.computeQueue();

		// Create compute pipeline

		compute.descriptorSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();
		//std::cout << "after descriptor set layout" << std::endl;
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};// = vks::initializers::pipelineLayoutCreateInfo(&compute.descriptorSetLayout, 1)


		// Push constants used to pass some parameters
		VkPushConstantRange pushConstantRange{};
		//vks::initializers::pushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t), 0);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(uint32_t);

		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
		VkDescriptorSetLayout tempDescriptorSetLayout = compute.descriptorSetLayout->getDescriptorSetLayout(); //why do i need to do this?
		pipelineLayoutCreateInfo.pSetLayouts = &tempDescriptorSetLayout;
		pipelineLayoutCreateInfo.setLayoutCount = 1;

		//std::cout << "before create pipeline layout" << std::endl;
		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutCreateInfo, nullptr, &compute.pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
		//std::cout << "before descriptor sets" << std::endl;
		compute.descriptorSets.push_back(VkDescriptorSet{});
		LveDescriptorWriter(*compute.descriptorSetLayout, *globalPool)
			.writeBuffer(0, &compute.storageBuffers.inputBuffer->descriptorInfo())
			.writeBuffer(1, &compute.storageBuffers.outputBuffer->descriptorInfo())
			.writeBuffer(2, &compute.uniformBuffer->descriptorInfo())
			.build(compute.descriptorSets.back());
		//std::cout << "after descriptor sets 1" << std::endl;
		compute.descriptorSets.push_back(VkDescriptorSet{});
		LveDescriptorWriter(*compute.descriptorSetLayout, *globalPool)
			.writeBuffer(0, &compute.storageBuffers.outputBuffer->descriptorInfo())
			.writeBuffer(1, &compute.storageBuffers.inputBuffer->descriptorInfo())
			.writeBuffer(2, &compute.uniformBuffer->descriptorInfo())
			.build(compute.descriptorSets.back());
		//std::cout << "after descriptor sets 2" << std::endl;
		// Create pipeline
		VkComputePipelineCreateInfo computePipelineCreateInfo{};// = vks::initializers::computePipelineCreateInfo(compute.pipelineLayout, 0);
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.layout = compute.pipelineLayout;
		computePipelineCreateInfo.flags = 0;

		auto computeCode = LvePipeline::readFile("shaders/cloth.comp.spv");
		//std::cout << "after shader" << std::endl;

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.codeSize = computeCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(computeCode.data());
		createInfo.flags = 0;
		if (vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr, &computeShaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module");
		}

		VkPipelineShaderStageCreateInfo shaderStage;
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStage.module = computeShaderModule;
		shaderStage.pName = "main";
		shaderStage.flags = 0;
		shaderStage.pNext = nullptr;
		shaderStage.pSpecializationInfo = nullptr;

		computePipelineCreateInfo.stage = shaderStage;
		if (vkCreateComputePipelines(lveDevice.device(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &compute.pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline");
		}
		//std::cout << "after pipeline" << std::endl;

		// Separate command pool as queue family for compute may be different than graphics
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = lveDevice.getComputeIndex();
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		//std::cout << "before command pool" << std::endl;
		if (vkCreateCommandPool(lveDevice.device(), &cmdPoolInfo, nullptr, &compute.commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool");
		}
		//std::cout << "after command pool" << std::endl;
		// Create a command buffer for compute operations
		VkCommandBufferAllocateInfo cmdBufAllocateInfo{};// = vks::initializers::commandBufferAllocateInfo(compute.commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 2);
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.commandPool = compute.commandPool;
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufAllocateInfo.commandBufferCount = 2;
		//std::cout << " allocation command" << std::endl;
		if (vkAllocateCommandBuffers(lveDevice.device(), &cmdBufAllocateInfo, &compute.commandBuffers[0]) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}
		//std::cout << "after allocating" << std::endl;
		// Semaphores for graphics / compute synchronization
		VkSemaphoreCreateInfo semaphoreCreateInfo{};// = vks::initializers::semaphoreCreateInfo();
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if (vkCreateSemaphore(lveDevice.device(), &semaphoreCreateInfo, nullptr, &compute.semaphores.ready) != VK_SUCCESS) {
			throw std::runtime_error("failed to create semaphore 1");
		}
		//std::cout << "after creating 1 semaphore" << std::endl;
		if (vkCreateSemaphore(lveDevice.device(), &semaphoreCreateInfo, nullptr, &compute.semaphores.complete) != VK_SUCCESS) {
			throw std::runtime_error("failed to create semaphore 2");
		}
		//std::cout << "after creating semaphorew 2" << std::endl;
		//std::cout << "end of this 1" << std::endl;
		// Build a single command buffer containing the compute dispatch commands
		buildComputeCommandBuffer(compute.commandBuffers[0]);
		buildComputeCommandBuffer(compute.commandBuffers[1]);
	}

	void Cloth::prepareUniformBuffers(float dt) {
		//std::cout << "beginning gof prepare uniform buffers" << std::endl;
		// Compute shader uniform buffer block
		//graphics.indexBuffer->writeToBuffer(indices.data());
		compute.uniformBuffer = std::make_unique<LveBuffer>(lveDevice, sizeof(compute.ubo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		compute.uniformBuffer->map();


		// Initial values
		float dx = clothStruct.size.x / (clothStruct.gridsize.x - 1);
		float dy = clothStruct.size.y / (clothStruct.gridsize.y - 1);

		compute.ubo.restDistH = dx;
		compute.ubo.restDistV = dy;
		compute.ubo.restDistD = sqrtf(dx * dx + dy * dy);
		compute.ubo.particleCount = clothStruct.gridsize;
		//std::cout << "15" << std::endl;
		updateComputeUBO(dt);
		//std::cout << "16" << std::endl;
		// Vertex shader uniform buffer block
		graphics.uniformBuffer = std::make_unique<LveBuffer>(lveDevice, sizeof(graphics.ubo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		graphics.uniformBuffer->map();
		//std::cout << "17" << std::endl;
		//updateGraphicsUBO();
	}

	void Cloth::updateComputeUBO(float dt) {
		if (!paused) {
			//compute.ubo.deltaT = 0.000005f;
			// todo: base on frametime
			// SRS - Clamp frameTimer to max 20ms refresh period (e.g. if blocked on resize), otherwise image breakup can occur
			compute.ubo.deltaT = fmin(dt, 0.02) * 0.005f;

			if (simulateWind) {
				timer += dt;
				std::default_random_engine rndEngine((unsigned)time(nullptr));
				std::uniform_real_distribution<float> rd(1.0f, 12.0f);
				compute.ubo.gravity.x = cos(glm::radians(-timer * 360.0f)) * (rd(rndEngine) - rd(rndEngine));
				compute.ubo.gravity.z = sin(glm::radians(timer * 360.0f)) * (rd(rndEngine) - rd(rndEngine));
			}
			else {
				compute.ubo.gravity.x = 0.0f;
				compute.ubo.gravity.z = 0.0f;
			}
		}
		else {
			compute.ubo.deltaT = 0.0f;
		}

		//memcpy(compute.uniformBuffer.mapped, &compute.ubo, sizeof(compute.ubo));
		compute.uniformBuffer->writeToBuffer(&compute.ubo, sizeof(compute.ubo));
	}
	void Cloth::predraw() {
		static bool firstDraw = true;
		VkSubmitInfo computeSubmitInfo{};// = vks::initializers::submitInfo();
		computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		// FIXME find a better way to do this (without using fences, which is much slower)
		VkPipelineStageFlags computeWaitDstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if (!firstDraw) {
			computeSubmitInfo.waitSemaphoreCount = 1;
			computeSubmitInfo.pWaitSemaphores = &compute.semaphores.ready;
			computeSubmitInfo.pWaitDstStageMask = &computeWaitDstStageMask;
		}
		else {
			firstDraw = false;
		}
		computeSubmitInfo.signalSemaphoreCount = 1;
		computeSubmitInfo.pSignalSemaphores = &compute.semaphores.complete;
		computeSubmitInfo.commandBufferCount = 1;
		computeSubmitInfo.pCommandBuffers = &compute.commandBuffers[readSet];
		//std::cout << "before queue submit" << std::endl;
		if (vkQueueSubmit(compute.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {

			throw std::runtime_error("failed to submit queue");
		}
		//std::cout << "after queue submit" << std::endl;
	}
	void Cloth::draw(VkCommandBuffer drawCmdBuffer) {
		// probably skip this whole function, do it in advancedrendersystem



		// Submit graphics commands
		//VulkanExampleBase::prepareFrame();

		VkPipelineStageFlags waitDstStageMask[2] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
		};
		VkSemaphore waitSemaphores[2] = {
			semaphorePresentComplete, compute.semaphores.complete
		};
		VkSemaphore signalSemaphores[2] = {
			semaphoreRenderComplete, compute.semaphores.ready
		};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 2;
		submitInfo.pWaitDstStageMask = waitDstStageMask;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.signalSemaphoreCount = 2;
		submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &drawCmdBuffer;
		if (vkQueueSubmit(lveDevice.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {   // which queue?
			throw std::runtime_error("failed to submit queue");
		}

		//VulkanExampleBase::submitFrame(); //this is just swapchain.queuepresent
	}
}
*/