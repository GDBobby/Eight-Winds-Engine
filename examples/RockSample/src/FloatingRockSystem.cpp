#include "FloatingRockSystem.h"
#include <EWEngine/Systems/Rendering/Pipelines/Pipe_SimpleTextured.h>

namespace EWE {
	FloatingRock::FloatingRock(EWEDevice& device) {
		rockModel = EWEModel::createModelFromFile(device, "Rock1.obj");
		rockTextureID = Texture_Builder::createSimpleTexture("rock.jpg", true, true, VK_SHADER_STAGE_FRAGMENT_BIT);

		//RANDOM NUMBER GENERATOR
		std::random_device r;
		std::default_random_engine randomGen(r());
		std::uniform_int_distribution<int> speedDistribution(4, 8); //inverse speed?
		std::uniform_int_distribution<int> rockDistribution(5, 12);
		std::uniform_int_distribution<int> trackDistribution(12, 20);
		unsigned int trackCount = trackDistribution(randomGen);

		for (uint16_t i = 0; i < trackCount; i++) {
			RockTrack tempTrack;
			tempTrack.trackTilt = glm::two_pi<float>() * i / trackCount; // if its an odd number use two_pi, even numebr use pi (reflected tracks)
			//std::cout << "track tilt on track[" << +i << "] : " << tempTrack.trackTilt << std::endl;
			tempTrack.radius = powf(static_cast<float>(i) + 1.f, 0.6f) + 30.f; //was + 10.f
			tempTrack.speed = speedDistribution(randomGen);
			//std::cout << "speed check : " << tempTrack.speed << std::endl;
			unsigned int rockCount = rockDistribution(randomGen);
			//unsigned int rockCount = i;
			//std::cout << "rock check : " << rockCount << std::endl;

			unsigned int positionAmount = tempTrack.speed * 250;
			for (uint32_t j = 0; j < positionAmount; j++) { //precode positions or na? test both ways
				float horizontalOffset = glm::two_pi<float>() * j / positionAmount;
				glm::vec3 tempPosition;
				if (i % 3 == 0) {
					tempPosition = glm::normalize(glm::vec3{ sin(horizontalOffset) * sin(tempTrack.trackTilt), cos(tempTrack.trackTilt) * cos(horizontalOffset), cos(horizontalOffset) * sin(tempTrack.trackTilt) }) * tempTrack.radius;
				}
				else if (i % 3 == 1) {
					tempPosition = glm::normalize(glm::vec3{ sin(horizontalOffset) * sin(tempTrack.trackTilt), cos(horizontalOffset) * sin(tempTrack.trackTilt), cos(tempTrack.trackTilt) * cos(horizontalOffset) }) * tempTrack.radius;
				}
				else if (i % 3 == 2) {
					tempPosition = glm::normalize(glm::vec3{ cos(tempTrack.trackTilt) * cos(horizontalOffset), sin(horizontalOffset) * sin(tempTrack.trackTilt), cos(horizontalOffset) * sin(tempTrack.trackTilt) }) * tempTrack.radius;
				}
				tempTrack.trackPositions.push_back(tempPosition);
			}

			for (uint16_t j = 0; j < rockCount; j++) {
				unsigned int tempPos = (positionAmount * j / rockCount);
				/*
				if (j > 0) {
					if (tempTrack.currentPosition.back() == tempPos) {
						std::cout << "last pos = current pos ~ trackCount:rockCount  :   " << +i << ":" << +j << std::endl;
					}
				}
				*/
				tempTrack.currentPosition.push_back(tempPos);
				//tempTrack.rocksInTrack.back().rock.transform.translation = tempTrack.trackPositions[tempPos];

			}
			rockField.push_back(tempTrack);
			//std::cout << "pushback " << std::endl;
		}
	}
	void FloatingRock::update() {
		for (auto& rock : rockField) {
			for (int j = 0; j < rock.currentPosition.size(); j++) {
				rock.currentPosition[j]++;
				if (rock.currentPosition[j] >= rock.trackPositions.size()) {
					rock.currentPosition[j] = 0;
				}
			}
		}
	}
	void FloatingRock::render(FrameInfo& frameInfo) {
		//PipelineManager::pipelines.at(Pipe_textured)->bind(frameInfo.cmdIndexPair.first);
		PipelineSystem::setFrameInfo(frameInfo);
		auto pipe = PipelineSystem::at(Pipe_textured);

		pipe->bindPipeline();

		pipe->bindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameInfo.index));
		pipe->bindDescriptor(1, &rockTextureID);

		pipe->bindModel(rockModel.get());
		SimplePushConstantData push{ glm::mat4{1.f}, glm::mat3{1.f} };
		for (int i = 0; i < rockField.size(); i++) {

			for (int j = 0; j < rockField[i].currentPosition.size(); j++) {

				glm::vec3& tempPosition = rockField[i].trackPositions[rockField[i].currentPosition[j]];
				push.modelMatrix[3].x = tempPosition.x;
				push.modelMatrix[3].y = tempPosition.y;
				push.modelMatrix[3].z = tempPosition.z;

				pipe->pushAndDraw(&push);
				//ockCount++;
				//std::cout << "post draw simple" << std::endl;
			}
		}

	}
}