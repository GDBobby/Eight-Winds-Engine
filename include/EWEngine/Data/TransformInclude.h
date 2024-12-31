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
	void mat4(float* buffer) const;

	glm::mat3 normalMatrix();

	bool similar(TransformComponent& second);

protected:
	std::array<float, 3> invScaleSquared{};

	glm::mat4 modelMatrix{};
	glm::mat3 normMat{};
};

struct Matrix3ForGLSL {
	std::array<glm::vec4, 3> columns{ glm::vec4{0.f}, glm::vec4{0.f}, glm::vec4{0.f} };
};

struct Transform2D {
	glm::vec2 translation{0.f};
	float rotation{0.f};
	glm::vec2 scale{ 1.f };

	Matrix3ForGLSL Matrix() const;

	Matrix3ForGLSL Matrix(glm::vec2 const& meterScaling) const;
	Matrix3ForGLSL Matrix(const glm::mat3 conversionMatrix, const glm::vec2 tilesOnScreen) const;

};

