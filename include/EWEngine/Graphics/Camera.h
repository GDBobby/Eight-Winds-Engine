#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Device_Buffer.h"

namespace EWE {
	struct GlobalUbo {
		glm::mat4 projView;
		//glm::mat4 inverseView{ 1.f };
		glm::vec3 cameraPos{ 1.f }; //4 just for alignment

		//alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, 3.f, -1.f });
		//glm::vec4 ambientLightColor{ 1.f, 0.7f, 0.7f, .02f };  // w is intensity
	};
	class EWECamera {
	public:

		void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

		void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f,1.0f, 0.0f});
		//void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.0f,1.0f, 0.0f });
		void NewViewTarget(glm::vec3 const& position, glm::vec3 const& target, glm::vec3 const& cameraUp);
		void NewViewTarget(glm::vec3 const& position, glm::vec3 const& target) {
			const glm::vec3 upDir{ 0.f, 1.f, 0.f };
			NewViewTarget(position, target, upDir);
		}
		void ViewTargetDirect(uint8_t currentFrame);
		void SetViewYXZ(glm::vec3 const& position, glm::vec3 const& rotation);

		const glm::mat4& GetProjection() const { return projection; }
		const glm::mat4& GetView() const { return view; }
		//const glm::mat4& getInverseView() const { return inverseViewMatrix; }

		void BindUBO(uint8_t frameIndex) {
			//printf("camera set ubo \n");
			uniformBuffers->at(frameIndex)->WriteToBuffer(&ubo);
			uniformBuffers->at(frameIndex)->Flush();
		}

		void SetBuffers(std::vector<EWEBuffer*>* buffers) {
			uniformBuffers = buffers;
			assert(uniformBuffers->size() > 0);
		}
		void UpdateViewData(glm::vec3 const& position, glm::vec3 const& target, glm::vec3 const& cameraUp = glm::vec3{0.f,1.f,0.f}) {
			//probably store a position, target, and camera up variable in this class, then hand out a pointer to those variables
			//being lazy rn
			this->position = position;
			this->target = target;
			this->cameraUp = cameraUp;
			dataHasBeenUpdated = 2;
		}
		
	private:
		std::vector<EWEBuffer*>* uniformBuffers{};
		GlobalUbo ubo{};

		glm::mat4 projection;
		glm::mat4 view;

		uint8_t dataHasBeenUpdated = 0;
		glm::vec3 position;
		glm::vec3 target;
		glm::vec3 cameraUp{ 0.f, 1.f, 0.f };

	};
}