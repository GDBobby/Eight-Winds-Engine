#pragma once


#include <EWEngine/Data/EngineDataTypes.h>

namespace EWE {
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
		glm::vec4 color{ 1.f };
		float radius{};
	};
	struct Grid2DPushConstantData {
		glm::vec4 scaleOffset{ 1.f };
		alignas(16)glm::vec2 gridScale{ 1.f };
		alignas(16)glm::vec3 color{ 1.f };
	};
	struct Array2DPushConstantData {
		glm::vec4 scaleOffset{ 1.f }; //need to change this to Matrix3ForGLSL transform, later
		alignas(16) glm::vec3 color{ 1.f }; //?idk if id stuff anything else right here
		alignas(16) int textureID;
		float depth{0.f};
	};
	struct Single2DPushConstantData {
		Matrix3ForGLSL transform;
		glm::vec3 color{ 1.f };
		Single2DPushConstantData() {}
		Single2DPushConstantData(Matrix3ForGLSL const& transform) : transform{ transform } {}
		Single2DPushConstantData(Matrix3ForGLSL const& transform, glm::vec3 color) : transform{ transform }, color{ color } {}
	};
	struct UV2DPushConstantData
	{

		Matrix3ForGLSL transform;
		glm::vec3 color{ 1.f };
		alignas(8)glm::vec2 uv{1.f};
		UV2DPushConstantData() {}
		UV2DPushConstantData(Matrix3ForGLSL const& transform) : transform{ transform } {}
		UV2DPushConstantData(Matrix3ForGLSL const& transform, glm::vec3 color) : transform{ transform }, color{ color } {}
	};

	struct LordDeliverMeFromThisEvilPushConstantData {
		glm::vec4 scaleOffset;
	};

	//deprecated
	/*
	struct NineUIPushConstantData {
		alignas(16)glm::vec2 scale;
		alignas(16)glm::vec4 offset; //xy = translation, z = borderSize
		alignas(16)glm::vec3 color;
		alignas(16)int textureID;
	};
	*/
	struct ModelTimePushData {
		glm::mat4 modelMatrix{ 1.f };
		float sinTime;
	};
	struct ModelPushData {
		glm::mat4 modelMatrix{ 1.f };
	};
	struct ModelAndNormalPushData {
		glm::mat4 modelMatrix;
		Matrix3ForGLSL normalMatrix;
	};
	struct PushTileConstantData {
		glm::vec3 translation{ 0.f };
		alignas(16)glm::vec3 scale{ 1.f };
	};

	struct UVScrollingPushData {
		glm::vec2 uvScroll{ 0.f };
	};

	struct OrbOverlayPushData {
		//128 bytes of space, currently using 56
		Matrix3ForGLSL transform;
		alignas(16)glm::vec4 hpData;
		alignas(16)glm::vec4 orbColor;
		alignas(16)glm::vec2 scrollData;
	};
	struct HPContainerPushData {
		//128 bytes of space, currently using 56
		Matrix3ForGLSL transform;
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
}