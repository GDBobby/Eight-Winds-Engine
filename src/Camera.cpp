#include "EWEngine/Graphics/Camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <limits>
#include <glm/gtc/matrix_transform.hpp>

namespace EWE {
	void EWECamera::SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
		ubo.projection = glm::mat4{ 1.0f };
		ubo.projection[0][0] = 2.f / (right - left);
		ubo.projection[1][1] = 2.f / (top - bottom);
		ubo.projection[2][2] = 1.f / (far - near);
		ubo.projection[3][0] = -(right + left) / (right - left);
		ubo.projection[3][1] = -(bottom + top) / (bottom - top);
		ubo.projection[3][2] = -near / (far - near);
	}

	void EWECamera::SetPerspectiveProjection(float fovy, float aspect, float near, float far) {
		if (!(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f)) {
			return;
		}

		ubo.projection = glm::perspective(-fovy, aspect, near, far);
	}

	void EWECamera::SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
		const glm::vec3 w{ glm::normalize(direction) };
		const glm::vec3 u{ glm::cross(w, up) }; //up needs to be passed in normalized
		const glm::vec3 v{ glm::cross(w, u) };

		ubo.view = glm::mat4{ 1.f };
		ubo.view[0][0] = u.x;
		ubo.view[1][0] = u.y;
		ubo.view[2][0] = u.z;

		ubo.view[0][1] = v.x;
		ubo.view[1][1] = v.y;
		ubo.view[2][1] = v.z;

		ubo.view[0][2] = w.x;
		ubo.view[1][2] = w.y;
		ubo.view[2][2] = w.z;

		ubo.view[3][0] = -glm::dot(u, position);
		ubo.view[3][1] = -glm::dot(v, position);
		ubo.view[3][2] = -glm::dot(w, position);

		ubo.cameraPos.x = position.x;
		ubo.cameraPos.y = position.y;
		ubo.cameraPos.z = position.z;
	}

	void EWECamera::NewViewTarget(glm::vec3 const& position, glm::vec3 const& target, glm::vec3 const& cameraUp) {

		//ubo.view = glm::lookAt(position, target, cameraUp);
		//inverseubo.view = glm::inverse(ubo.view);
		
		//f is going to be constant in this top down, or i could change it on a rare zoom
		glm::vec3 f = glm::normalize(target - position);
		glm::vec3 s = glm::normalize(glm::cross(f, cameraUp));
		glm::vec3 u = glm::cross(s, f);
			
		ubo.view[0][0] = s.x;//
		ubo.view[1][0] = s.y;
		ubo.view[2][0] = s.z;
		ubo.view[0][1] = u.x;//
		ubo.view[1][1] = u.y;
		ubo.view[2][1] = u.z;
		ubo.view[0][2] = -f.x;//
		ubo.view[1][2] = -f.y;
		ubo.view[2][2] = -f.z;
		ubo.view[3][0] = -dot(s, position);
		ubo.view[3][1] = -dot(u, position);
		ubo.view[3][2] = dot(f, position);

		ubo.cameraPos.x = position.x;
		ubo.cameraPos.y = position.y;
		ubo.cameraPos.z = position.z;
		//printf("camera pos : %.2f:%.2f:%.2f \n", ubo.cameraPos.x, ubo.cameraPos.y, ubo.cameraPos.z);
	}

	void EWECamera::ViewTargetDirect(uint8_t currentFrame) {


		if (dataHasBeenUpdated == 0) {
			return;
		}
		dataHasBeenUpdated--;
		//printf("view target direct, sizeof globalubo : %lu \n", sizeof(GlobalUbo));

		glm::vec3 f = glm::normalize(target - position);
		glm::vec3 s = glm::normalize(glm::cross(f, cameraUp));
		glm::vec3 u = glm::cross(s, f);

		float* mem = reinterpret_cast<float*>(uniformBuffers->at(currentFrame)->getMappedMemory());
		constexpr size_t viewOffset = offsetof(GlobalUbo, view) / sizeof(float);
		mem[viewOffset + 0] = s.x;
		mem[viewOffset + 4] = s.y;
		mem[viewOffset + 8] = s.z;

		mem[viewOffset + 1] = u.x;
		mem[viewOffset + 5] = u.y;
		mem[viewOffset + 9] = u.z;

		mem[viewOffset + 2] = -f.x;
		mem[viewOffset + 6] = -f.y;
		mem[viewOffset + 10] = -f.z;

		mem[viewOffset + 12] = -dot(s, position);
		mem[viewOffset + 13] = -dot(u, position);
		mem[viewOffset + 14] = dot(f, position);

		constexpr size_t camPosOffset = offsetof(GlobalUbo, cameraPos) / sizeof(float);
		mem[camPosOffset + 0] = position.x;
		mem[camPosOffset + 1] = position.y;
		mem[camPosOffset + 2] = position.z;

		//printf("view target direct, currentFrmae : %d \n", currentFrame);
		uniformBuffers->at(currentFrame)->flush();
	}

	void EWECamera::SetViewYXZ(glm::vec3 const& position, glm::vec3 const& rotation) {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
		const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
		const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
		//ubo.view = glm::mat4{ 1.f };
		ubo.view[0][0] = u.x;
		ubo.view[1][0] = u.y;
		ubo.view[2][0] = u.z;
		ubo.view[0][1] = v.x;
		ubo.view[1][1] = v.y;
		ubo.view[2][1] = v.z;
		ubo.view[0][2] = w.x;
		ubo.view[1][2] = w.y;
		ubo.view[2][2] = w.z;
		ubo.view[3][0] = -glm::dot(u, position);
		ubo.view[3][1] = -glm::dot(v, position);
		ubo.view[3][2] = -glm::dot(w, position);

		ubo.cameraPos.x = position.x;
		ubo.cameraPos.y = position.y;
		ubo.cameraPos.z = position.z;
	}
}