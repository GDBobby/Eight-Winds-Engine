#pragma once
#include "EWE_camera.h"
#include "EWEngine/LoadingScreen/LeafSystem.h"

namespace EWE {

#define MAX_LIGHTS 10

	struct PointLightData {
		glm::vec4 position{}; //ignores w
		glm::vec4 color{}; //w is intensity
	};
	struct SpotLightData {
		glm::vec4 position{};
		glm::vec4 color{};
		glm::vec4 direction; //W in the direction is the cutoff
		//float cutoff;
	};
	struct LightBufferObject {
		glm::vec4 ambientColor{};
		glm::vec4 sunlightDirection{}; //w for sun power
		glm::vec4 sunlightColor{};
		PointLightData pointLights[MAX_LIGHTS];
		uint8_t numLights;
	};
	struct PlayerBoneObject {//CAN NOT SHORTEN, ONLY INT,BOOL, not sure if its worth the trouble of dealing with bool operation?
		int playerIndex{};
		int boneCount{};
	};
	struct MaterialBufferObject {
		glm::vec3 ambient{};
		glm::vec3 diffuse{};
		glm::vec3 specular{};
		float shininess{};
	};
	struct SpotBufferObject {
		glm::vec4 ambientColor{};
		glm::vec4 sunlightDirection{};
		glm::vec4 sunlightColor{};
		PointLightData pointLights[MAX_LIGHTS];
		int pointNumLights{};
		SpotLightData spotLights[MAX_LIGHTS];
		int spotNumLights{};
	};

	struct FrameInfo {
		std::pair<VkCommandBuffer, uint8_t> cmdIndexPair{};
		float time{0.f};
		//std::vector<glm::mat4> playerMats;
		//std::vector<SpotLight>& spotLights;
	};
	struct FrameInfo2D {
		std::pair<VkCommandBuffer, uint8_t> cmdIndexPair{};
		//std::vector<std::vector<GameObject2D>>& uiObjects;
		bool menuActive{};
	};
	struct FrameInfoLoading {
		std::pair<VkCommandBuffer, uint8_t> cmdIndexPair{};
		LeafSystem* leafSystem{nullptr};
	};
}