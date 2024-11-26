#pragma once

#include <EWEngine/Graphics/PushConstants.h>
#include <EWEngine/Graphics/Model/Model.h>
#include <EWEngine/Graphics/Pipeline.h>
#include <EWEngine/Systems/Rendering/Skin/SkinRS.h>

//#include <glm/glm.hpp>

#include <memory>
#include <cmath>
#include <iostream>
#include <random>


namespace EWE {

	class FloatingRock {
	public:

		FloatingRock();
		~FloatingRock();

		FloatingRock(const FloatingRock&) = delete;
		FloatingRock& operator=(const FloatingRock&) = delete;
		FloatingRock(FloatingRock&&) = default;
		FloatingRock& operator=(FloatingRock&&) = default;

		void Dispatch(float dt);
	private:

		EWEModel* rockModel;
		MaterialInfo rockMaterial;

		struct RockTrack {
			std::vector<uint32_t> currentPosition{};
			//std::vector<TransformComponent> transforms{};

			int speed = 1;
			float radius = 1.0f;
			float trackTilt = 1.0f;
			//float trackDriftSpeed = 0.0f; //circular drift, not sure how to handle this yet
			std::vector<glm::vec3> trackPositions{};
			bool drawable = true;
			//glm::vec3 trackOffset{0.f}; //not currently included

			//cos 0 = 1
			//sin 0 = 0
		};
		std::vector<RockTrack> rockField;

		//compute data
		VkPipeline compPipeline{ VK_NULL_HANDLE };
		VkPipelineLayout compPipeLayout{ VK_NULL_HANDLE };
		std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> compDescriptorSet = { VK_NULL_HANDLE, VK_NULL_HANDLE };
		VkShaderModule compShaderModule{ VK_NULL_HANDLE };
		struct RockCompPushData {
			float secondsSinceBeginning{0.f};
		};
		RockCompPushData compPushData{};
		EWEDescriptorSetLayout* compDSL{ nullptr };

		EWEBuffer* rockBuffer{ nullptr };

		VkBufferMemoryBarrier bufferBarrier[MAX_FRAMES_IN_FLIGHT * 2];
		bool previouslySubmitted = false;

		void InitComputeData();
	};
}