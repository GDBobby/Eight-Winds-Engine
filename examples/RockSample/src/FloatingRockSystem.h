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

		FloatingRock(EWEDevice& device);

		FloatingRock(const FloatingRock&) = delete;
		FloatingRock& operator=(const FloatingRock&) = delete;
		FloatingRock(FloatingRock&&) = default;
		FloatingRock& operator=(FloatingRock&&) = default;

		void update();

		void render(FrameInfo& frameInfo);
	private:
		std::unique_ptr<EWEModel> rockModel;
		TextureDesc rockTextureID{ 0 };

		struct RockTrack {
			std::vector<unsigned int> currentPosition;
			int speed = 1;
			float radius = 1.0f;
			float trackTilt = 1.0f;
			//float trackDriftSpeed = 0.0f; //circular drift, not sure how to handle this yet
			std::vector<glm::vec3> trackPositions;
			//glm::vec3 trackOffset{0.f}; //not currently included

			//cos 0 = 1
			//sin 0 = 0
		};
		std::vector<RockTrack> rockField;
	};
}