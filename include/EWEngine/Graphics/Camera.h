#pragma once

#include "EWEngine/Graphics/LightBufferObject.h"
#include "EWEngine/Graphics/Device_Buffer.h"

#include <glm/glm.hpp>
#include <vector>


namespace EWE {
	class EWECamera {
	public:
		EWECamera();

		void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

		void SetViewDirection(const glm::vec3 position, const glm::vec3 direction, const glm::vec3 up = glm::vec3{0.0f,1.0f, 0.0f});
		//void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.0f,1.0f, 0.0f });
		void NewViewTarget(glm::vec3 const& position, glm::vec3 const& target, glm::vec3 const& cameraUp);
		void NewViewTarget(glm::vec3 const& position, glm::vec3 const& target) {
			const glm::vec3 upDir{ 0.f, 1.f, 0.f };
			NewViewTarget(position, target, upDir);
		}
		void ViewTargetDirect();
		void SetViewYXZ(glm::vec3 const& position, glm::vec3 const& rotation);

		const glm::mat4& GetProjection() const { return projection; }
		const glm::mat4& GetView() const { return view; }
		//const glm::mat4& getInverseView() const { return inverseViewMatrix; }

		void BindBothUBOs();
		void BindUBO();

		void SetBuffers();
		void UpdateViewData(glm::vec3 const& position, glm::vec3 const& target, glm::vec3 const& cameraUp = glm::vec3{ 0.f,1.f,0.f });
		void PrintCameraPos();

		std::array<glm::vec4, 6> GetFrustumPlanes();
		
	private:
		std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT>* uniformBuffers;
		GlobalUbo ubo{};

		glm::mat4 projection{ 0.f };
		glm::mat4 view{ 0.f };

		uint8_t dataHasBeenUpdated = 0;
		glm::vec3 position;
		glm::vec3 target;
		glm::vec3 cameraUp{ 0.f, 1.f, 0.f };

	};
}