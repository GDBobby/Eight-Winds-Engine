#pragma once

#include <glm/glm.hpp>

namespace EWE {
#define MAX_LIGHTS 10

	struct GlobalUbo {
		glm::mat4 projView;
		//glm::mat4 inverseView{ 1.f };
		glm::vec3 cameraPos{ 1.f }; //4 just for alignment

		//alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, 3.f, -1.f });
		//glm::vec4 ambientLightColor{ 1.f, 0.7f, 0.7f, .02f };  // w is intensity
	};
	struct PointLightData {
		glm::vec4 position{}; //ignores w
		glm::vec4 color{}; //w is intensity
	};

	struct LightBufferObject {
		glm::vec4 ambientColor{};
		glm::vec4 sunlightDirection{}; //w for sun power
		glm::vec4 sunlightColor{};
		PointLightData pointLights[MAX_LIGHTS];
		uint8_t numLights;
	};

	/*
	struct SpotLightData {
		glm::vec4 position{};
		glm::vec4 color{};
		glm::vec4 direction; //W in the direction is the cutoff
		//float cutoff;
	};

	struct SpotlightBufferObject {
		glm::vec4 ambientColor{};
		glm::vec4 sunlightDirection{};
		glm::vec4 sunlightColor{};
		PointLightData pointLights[MAX_LIGHTS];
		int pointNumLights{};
		SpotLightData spotLights[MAX_LIGHTS];
		int spotNumLights{};
	};
	*/


}