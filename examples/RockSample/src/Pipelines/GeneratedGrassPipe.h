#pragma once


#include <EWEngine/Systems/PipelineSystem.h>

struct GrassBufferObject {
	float spacing;
	float height;
	float time;
	float windDir;
	float endDistance;
	float windStrength;
	VkBool32 cullGrassHeight;
	//float lengthGroundPosV2;
};

namespace EWE {
	class GeneratedGrassPipe : public PipelineSystem {
	public:
		GeneratedGrassPipe();
		~GeneratedGrassPipe() {}

		void CreatePipeLayout() final;
		void CreatePipeline() final;


	};
}