#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

struct TransformComponent {
	glm::vec3 translation{0.f}; 
	glm::vec3 scale{ 1.0f };
	glm::vec3 rotation{ 0.0f };


	glm::mat4 mat4();
	void mat4(float* buffer);

	glm::mat3 normalMatrix();

	bool similar(TransformComponent& second);

protected:
	std::array<float, 3> invScaleSquared{};

	glm::mat4 modelMatrix{};
	glm::mat3 normMat{};
};