#pragma once

#include <glm/glm.hpp>

struct SimplePushConstantData {
	glm::mat4 modelMatrix{ 1.f };
	glm::mat4 normalMatrix{ 1.f };
};
struct SpritePushConstantData {
	glm::mat4 modelMatrix{ 1.f };
	int textureID;
};

struct PlayerPushConstantData {
	glm::mat4 modelMatrix{ 1.f };
	int index_BoneCount = 0;
};
struct LightningPushConstants {
	glm::vec4 pointA;
	glm::vec4 pointB;
	glm::vec4 vectorAB;
	glm::vec4 midPoint;
};

struct PointLightPushConstants {
	glm::vec4 position{};
	glm::vec4 color{1.f};
	float radius{};
};
struct Grid2DPushConstantData {
	glm::vec4 scaleOffset{ 1.f };
	alignas(16)glm::vec2 gridScale{1.f};
	alignas(16)glm::vec3 color{ 1.f };
};
struct Simple2DPushConstantData {
	glm::vec4 scaleOffset{ 1.f }; //4 floats here, 2 x 2
	alignas(16) glm::vec3 color{1.f}; //?idk if id stuff anything else right here
	alignas(16) int textureID;
};
struct NineUIPushConstantData {
	alignas(16)glm::vec2 scale;
	alignas(16)glm::vec4 offset; //xy = translation, z = borderSize
	alignas(16)glm::vec3 color;
	alignas(16)int textureID;
};
struct ModelTimePushData {
	glm::mat4 modelMatrix{ 1.f };
	float sinTime;
};
struct ModelPushData {
	glm::mat4 modelMatrix{ 1.f };
};
struct PushTileConstantData {
	glm::vec3 translation{0.f};
	alignas(16)glm::vec3 scale{ 1.f };
};

struct UVScrollingPushData {
	glm::vec2 uvScroll{ 0.f };
};

struct OrbOverlayPushData {
	//128 bytes of space, currently using 56
	glm::vec4 scaleOffset;
	alignas(16)glm::vec4 hpData;
	alignas(16)glm::vec4 orbColor;
	alignas(16)glm::vec2 scrollData;
};
struct HPContainerPushData {
	//128 bytes of space, currently using 56
	glm::vec4 scaleOffset;
	alignas(16)glm::vec4 hpData;
	alignas(16)glm::vec4 orbColor;
};
struct ExpBarPushData {
	glm::vec4 scaleOffset;
	alignas(16)float expPercent;
};
struct CastleHealthPushData {
	glm::vec4 scaleOffset;
	alignas(16)glm::vec4 healthCutoff;
	alignas(16) glm::vec4 healthColor;
};