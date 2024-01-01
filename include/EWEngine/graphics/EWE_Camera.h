#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "EWE_Buffer.h"

namespace EWE {
	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		//glm::mat4 inverseView{ 1.f };
		glm::vec4 cameraPos{ 1.f }; //4 just for alignment

		//alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, 3.f, -1.f });
		//glm::vec4 ambientLightColor{ 1.f, 0.7f, 0.7f, .02f };  // w is intensity
	};
	class EWECamera {
	public:

		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setPerspectiveProjection(float fovy, float aspect, float near, float far);

		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f,1.0f, 0.0f});
		//void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.0f,1.0f, 0.0f });
		void newViewTarget(glm::vec3 const& position, glm::vec3 const& target, glm::vec3 const& cameraUp);
		void ViewTargetDirect(uint8_t currentFrame);
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4& getProjection() const { return ubo.projection; }
		const glm::mat4& getView() const { return ubo.view; }
		//const glm::mat4& getInverseView() const { return inverseViewMatrix; }

		void bindUBO(uint8_t frameIndex) {
			//printf("camera set ubo \n");
			uniformBuffers->at(frameIndex)->writeToBuffer(&ubo);
			uniformBuffers->at(frameIndex)->flush();
		}

		void setBuffers(std::vector<std::unique_ptr<EWEBuffer>>* buffers) {
			uniformBuffers = buffers;
			assert(uniformBuffers->size() > 0);
		}
		void updateViewData(glm::vec3 const& position, glm::vec3 const& target, glm::vec3 const& cameraUp = glm::vec3{0.f,1.f,0.f}) {
			//probably store a position, target, and camera up variable in this class, then hand out a pointer to those variables
			//being lazy rn
			this->position = position;
			this->target = target;
			this->cameraUp = cameraUp;
			dataHasBeenUpdated = 2;
		}
		
	private:
		std::vector<std::unique_ptr<EWEBuffer>>* uniformBuffers{};
		GlobalUbo ubo{};

		uint8_t dataHasBeenUpdated = 0;
		glm::vec3 position;
		glm::vec3 target;
		glm::vec3 cameraUp{ 0.f, 1.f, 0.f };

	};
}