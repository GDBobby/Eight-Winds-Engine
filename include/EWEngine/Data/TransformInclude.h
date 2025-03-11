#pragma once

#include <EWEngine/Data/EngineDataTypes.h>

namespace EWE {
	struct TransformComponent {
		glm::vec3 translation{ 0.f };
		glm::vec3 scale{ 1.0f };
		glm::vec3 rotation{ 0.0f };


		glm::mat4 mat4();
		void mat4(float* buffer) const;

		Matrix3ForGLSL normalMatrix();

		bool similar(TransformComponent& second);

	protected:
		std::array<float, 3> invScaleSquared{};

		glm::mat4 modelMatrix{};
		Matrix3ForGLSL normMat{};
	};


	struct Transform2D {
		glm::vec2 translation{ 0.f };
		float rotation{ 0.f };
		glm::vec2 scale{ 1.f };

		Matrix3ForGLSL Matrix() const;
		/*
		Matrix3ForGLSL Matrix(glm::vec2 const& meterScaling) const;
		Matrix3ForGLSL Matrix(const glm::mat3 conversionMatrix, const glm::vec2 tilesOnScreen) const;
		*/
		Matrix3ForGLSL MatrixNoRotation() const;

	};
} //namespace EWE
