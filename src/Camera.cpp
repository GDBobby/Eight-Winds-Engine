#include "EWEngine/Graphics/Camera.h"

#include "EWEngine/Graphics/DescriptorHandler.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <limits>
#include <glm/gtc/matrix_transform.hpp>

namespace EWE {
	EWECamera::EWECamera() {
		view[3][3] = 1.f;
	}


	void EWECamera::SetBuffers() {
		DescriptorHandler::SetCameraBuffers(uniformBuffers);
	}

	void EWECamera::SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
		//projection = glm::mat4{ 1.0f };
		projection = glm::identity<glm::mat4>();
		projection[0][0] = 2.f / (right - left);
		projection[1][1] = 2.f / (top - bottom);
		projection[2][2] = 1.f / (far - near);
		projection[3][0] = -(right + left) / (right - left);
		projection[3][1] = -(bottom + top) / (bottom - top);
		projection[3][2] = -near / (far - near);
	}

	void EWECamera::SetPerspectiveProjection(float fovy, float aspect, float near, float far) {
		//inverting aspect, to make it height / width instead of width / height
		projection = glm::perspective(fovy, aspect, near, far);
	}

	void EWECamera::SetViewDirection(const glm::vec3 position, const glm::vec3 forward, const glm::vec3 cameraUp) {

		const glm::vec3 right{ normalize(glm::cross(cameraUp, forward)) }; //up needs to be passed in normalized
		const glm::vec3 up{ normalize(glm::cross(forward, right)) };

		view[0][0] = right.x;
		view[1][0] = right.y;
		view[2][0] = right.z;

		view[0][1] = -up.x;
		view[1][1] = -up.y;
		view[2][1] = -up.z;

		view[0][2] = -forward.x;
		view[1][2] = -forward.y;
		view[2][2] = -forward.z;

		view[3][0] = -glm::dot(right, position);
		view[3][1] = glm::dot(up, position);
		view[3][2] = glm::dot(forward, position);

		ubo.projView = projection * view;
		//ubo.projection = projection;
		//ubo.view = view;
		ubo.cameraPos.x = position.x;
		ubo.cameraPos.y = position.y;
		ubo.cameraPos.z = position.z;
	}

	void EWECamera::NewViewTarget(glm::vec3 const& position, glm::vec3 const& target, glm::vec3 const& cameraUp) {
		glm::vec3 forward = glm::normalize(target - position);
		SetViewDirection(position, forward, cameraUp);

		//view = glm::lookAt(position, target, cameraUp);
		//inverseview = glm::inverse(view);
		return;
		
		//f is going to be constant in this top down, or i could change it on a rare zoom
		glm::vec3 right = glm::normalize(glm::cross(cameraUp, forward));
		glm::vec3 up = normalize(glm::cross(forward, right));
	}

	void EWECamera::ViewTargetDirect() {


		if (dataHasBeenUpdated == 0) {
			return;
		}
		dataHasBeenUpdated--;
		//printf("view target direct, sizeof globalubo : %zu \n", sizeof(GlobalUbo));

		const glm::vec3 f = glm::normalize(target - position);
		const glm::vec3 s = glm::normalize(glm::cross(f, cameraUp));
		const glm::vec3 u = glm::cross(s, f);

		float* mem = reinterpret_cast<float*>(&view);
			//reinterpret_cast<float*>(uniformBuffers->at(currentFrame)->getMappedMemory());
		//constexpr size_t viewOffset = offsetof(GlobalUbo, view) / sizeof(float);
		mem[0] = s.x;
		mem[4] = s.y;
		mem[8] = s.z;

		mem[1] = u.x;
		mem[5] = u.y;
		mem[9] = u.z;

		mem[2] = -f.x;
		mem[6] = -f.y;
		mem[10] = -f.z;

		mem[12] = -dot(s, position);
		mem[13] = -dot(u, position);
		mem[14] = dot(f, position);

		ubo.cameraPos = position;

		ubo.projView = projection * view;
		//ubo.projection = projection;
		//ubo.view = view;
		uniformBuffers->at(VK::Object->frameIndex)->WriteToBuffer(&ubo);


		//printf("view target direct, currentFrmae : %d \n", currentFrame);
		uniformBuffers->at(VK::Object->frameIndex)->Flush();
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
		//view = glm::mat4{ 1.f };
		view[0][0] = u.x;
		view[1][0] = u.y;
		view[2][0] = u.z;
		view[0][1] = v.x;
		view[1][1] = v.y;
		view[2][1] = v.z;
		view[0][2] = w.x;
		view[1][2] = w.y;
		view[2][2] = w.z;
		view[3][0] = -glm::dot(u, position);
		view[3][1] = -glm::dot(v, position);
		view[3][2] = -glm::dot(w, position);

		ubo.projView = projection * view;
		//ubo.projection = projection;
		//ubo.view = view;
		ubo.cameraPos = position;
	}
	void EWECamera::BindBothUBOs() {
		uniformBuffers->at(0)->WriteToBuffer(&ubo, sizeof(GlobalUbo));
		uniformBuffers->at(0)->Flush();
		uniformBuffers->at(1)->WriteToBuffer(&ubo, sizeof(GlobalUbo));
		uniformBuffers->at(1)->Flush();
	}
	void EWECamera::BindUBO() {
		//printf("camera set ubo \n");
		//printf("offset of camera pos : %zu\n", offsetof(GlobalUbo, GlobalUbo::cameraPos));
		uniformBuffers->at(VK::Object->frameIndex)->WriteToBuffer(&ubo, sizeof(GlobalUbo));
		uniformBuffers->at(VK::Object->frameIndex)->Flush();
	}
	void EWECamera::PrintCameraPos() {
		printf("camera pos : %.3f:%.3f:%.3f\n", ubo.cameraPos.x, ubo.cameraPos.y, ubo.cameraPos.z);
	}
	void EWECamera::UpdateViewData(glm::vec3 const& position, glm::vec3 const& target, glm::vec3 const& cameraUp) {
		//probably store a position, target, and camera up variable in this class, then hand out a pointer to those variables
		//being lazy rn
		this->position = position;
		this->target = target;
		this->cameraUp = cameraUp;
		dataHasBeenUpdated = MAX_FRAMES_IN_FLIGHT;
	};



	std::array<glm::vec4, 6> EWECamera::GetFrustumPlanes() {
		std::array<glm::vec4, 6> planes;
		enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, BACK = 4, FRONT = 5 };
		planes[LEFT].x = ubo.projView[0].w + ubo.projView[0].x;
		planes[LEFT].y = ubo.projView[1].w + ubo.projView[1].x;
		planes[LEFT].z = ubo.projView[2].w + ubo.projView[2].x;
		planes[LEFT].w = ubo.projView[3].w + ubo.projView[3].x;

		planes[RIGHT].x = ubo.projView[0].w - ubo.projView[0].x;
		planes[RIGHT].y = ubo.projView[1].w - ubo.projView[1].x;
		planes[RIGHT].z = ubo.projView[2].w - ubo.projView[2].x;
		planes[RIGHT].w = ubo.projView[3].w - ubo.projView[3].x;

		planes[TOP].x = ubo.projView[0].w - ubo.projView[0].y;
		planes[TOP].y = ubo.projView[1].w - ubo.projView[1].y;
		planes[TOP].z = ubo.projView[2].w - ubo.projView[2].y;
		planes[TOP].w = ubo.projView[3].w - ubo.projView[3].y;

		planes[BOTTOM].x = ubo.projView[0].w + ubo.projView[0].y;
		planes[BOTTOM].y = ubo.projView[1].w + ubo.projView[1].y;
		planes[BOTTOM].z = ubo.projView[2].w + ubo.projView[2].y;
		planes[BOTTOM].w = ubo.projView[3].w + ubo.projView[3].y;

		planes[BACK].x = ubo.projView[0].w + ubo.projView[0].z;
		planes[BACK].y = ubo.projView[1].w + ubo.projView[1].z;
		planes[BACK].z = ubo.projView[2].w + ubo.projView[2].z;
		planes[BACK].w = ubo.projView[3].w + ubo.projView[3].z;

		planes[FRONT].x = ubo.projView[0].w - ubo.projView[0].z;
		planes[FRONT].y = ubo.projView[1].w - ubo.projView[1].z;
		planes[FRONT].z = ubo.projView[2].w - ubo.projView[2].z;
		planes[FRONT].w = ubo.projView[3].w - ubo.projView[3].z;

		for (auto i = 0; i < planes.size(); i++) {
			const float length = glm::sqrt(planes[i].x * planes[i].x + planes[i].y * planes[i].y + planes[i].z * planes[i].z);
			planes[i] /= length;
		}

		return planes;
	}
}