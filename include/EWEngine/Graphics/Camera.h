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

		void BindUBO(uint8_t frameIndex);

		void SetBuffers(std::vector<EWEBuffer*>* buffers) {
			assert(buffers->size() > 0);
			uniformBuffers = buffers;
		}
		void UpdateViewData(glm::vec3 const& position, glm::vec3 const& target, glm::vec3 const& cameraUp = glm::vec3{ 0.f,1.f,0.f });
		void PrintCameraPos();
		
	private:
		std::vector<EWEBuffer*>* uniformBuffers{};
		GlobalUbo ubo{};

		glm::mat4 projection{ 0.f };
		glm::mat4 view{ 1.f };

		uint8_t dataHasBeenUpdated = 0;
		glm::vec3 position;
		glm::vec3 target;
		glm::vec3 cameraUp{ 0.f, 1.f, 0.f };

	};
}