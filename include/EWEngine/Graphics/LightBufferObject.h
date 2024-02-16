#pragma once

#include <glm/glm.hpp>

namespace EWE {
#define MAX_LIGHTS 10

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
	struct PlayerBoneObject {//CAN NOT SHORTEN, ONLY INT,BOOL, not sure if its worth the trouble of dealing with bool operation?
		int playerIndex{};
		int boneCount{};
	};
	*/

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