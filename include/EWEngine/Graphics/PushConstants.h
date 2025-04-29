#pragma once


#include <EWGraphics/Data/EngineDataTypes.h>

namespace EWE {
	struct SpritePushConstantData {
		lab::mat4 modelMatrix{ 1.f };
		int textureID;
	};

	struct PlayerPushConstantData {
		lab::mat4 modelMatrix{ 1.f };
		int index_BoneCount = 0;
	};
	struct LightningPushConstants {
		lab::vec4 pointA;
		lab::vec4 pointB;
		lab::vec4 vectorAB;
		lab::vec4 midPoint;
	};

	struct PointLightPushConstants {
		lab::vec4 position{};
		lab::vec4 color{ 1.f };
		float radius{};
	};
	struct Grid2DPushConstantData {
		lab::vec4 scaleOffset{ 1.f };
		alignas(16)lab::vec2 gridScale{ 1.f };
		alignas(16)lab::vec3 color{ 1.f };
	};
	struct Array2DPushConstantData {
		lab::vec4 scaleOffset{ 1.f }; //need to change this to lab::mat3 transform, later
		alignas(16) lab::vec3 color{ 1.f }; //?idk if id stuff anything else right here
		alignas(16) int textureID;
		float depth{0.f};
	};
	struct Single2DPushConstantData {
		lab::mat3 transform;
		lab::vec3 color{ 1.f };
		Single2DPushConstantData() {}
		Single2DPushConstantData(lab::mat3 const& transform) : transform{ transform } {}
		Single2DPushConstantData(lab::mat3 const& transform, lab::vec3 color) : transform{ transform }, color{ color } {}
	};
	struct UV2DPushConstantData {

		lab::mat3 transform;
		lab::vec3 color{ 1.f };
		alignas(8)lab::vec2 uv{1.f};
		UV2DPushConstantData() {}
		UV2DPushConstantData(lab::mat3 const& transform) : transform{ transform } {}
		UV2DPushConstantData(lab::mat3 const& transform, lab::vec3 color) : transform{ transform }, color{ color } {}
	};

	struct LordDeliverMeFromThisEvilPushConstantData {
		lab::vec4 scaleOffset;
	};

	//deprecated
	/*
	struct NineUIPushConstantData {
		alignas(16)lab::vec2 scale;
		alignas(16)lab::vec4 offset; //xy = translation, z = borderSize
		alignas(16)lab::vec3 color;
		alignas(16)int textureID;
	};
	*/
	struct ModelTimePushData {
		lab::mat4 modelMatrix{ 1.f };
		float sinTime;
	};
	struct ModelPushData {
		lab::mat4 modelMatrix{ 1.f };
	};
	struct ModelAndNormalPushData {
		lab::mat4 modelMatrix;
		lab::mat3 normalMatrix;
	};
	struct PushTileConstantData {
		lab::vec3 translation{ 0.f };
		alignas(16)lab::vec3 scale{ 1.f };
	};

	struct UVScrollingPushData {
		lab::vec2 uvScroll{ 0.f };
	};

	struct OrbOverlayPushData {
		//128 bytes of space, currently using 56
		lab::mat3 transform;
		alignas(16)lab::vec4 hpData;
		alignas(16)lab::vec4 orbColor;
		alignas(16)lab::vec2 scrollData;
	};
	struct HPContainerPushData {
		//128 bytes of space, currently using 56
		lab::mat3 transform;
		alignas(16)lab::vec4 hpData;
		alignas(16)lab::vec4 orbColor;
	};
	struct ExpBarPushData {
		lab::vec4 scaleOffset;
		alignas(16)float expPercent;
	};
	struct CastleHealthPushData {
		lab::vec4 scaleOffset;
		alignas(16)lab::vec4 healthCutoff;
		alignas(16) lab::vec4 healthColor;
	};
}