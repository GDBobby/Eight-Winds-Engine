#pragma once

#include <LAB/Vector.h>
#include <LAB/Matrix.h>

namespace EWE {
#define MAX_LIGHTS 10

	struct GlobalUbo {
		lab::mat4 projView;
		//lab::mat4 inverseView{ 1.f };
		lab::vec3 cameraPos{ 1.f }; //4 just for alignment

		//alignas(16) lab::vec3 lightDirection = glm::normalize(lab::vec3{ 1.f, 3.f, -1.f });
		//lab::vec4 ambientLightColor{ 1.f, 0.7f, 0.7f, .02f };  // w is intensity
	};
	struct PointLightData {
		lab::vec4 position{}; //ignores w
		lab::vec4 color{}; //w is intensity
	};

	struct LightBufferObject {
		lab::vec4 ambientColor{};
		lab::vec4 sunlightDirection{}; //w for sun power
		lab::vec4 sunlightColor{};
		PointLightData pointLights[MAX_LIGHTS];
		uint8_t numLights{ 0 };
	};

	/*
	struct SpotLightData {
		lab::vec4 position{};
		lab::vec4 color{};
		lab::vec4 direction; //W in the direction is the cutoff
		//float cutoff;
	};

	struct SpotlightBufferObject {
		lab::vec4 ambientColor{};
		lab::vec4 sunlightDirection{};
		lab::vec4 sunlightColor{};
		PointLightData pointLights[MAX_LIGHTS];
		int pointNumLights{};
		SpotLightData spotLights[MAX_LIGHTS];
		int spotNumLights{};
	};
	*/


}