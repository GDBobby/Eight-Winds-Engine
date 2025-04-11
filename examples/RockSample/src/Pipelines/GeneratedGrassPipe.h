#pragma once


#include <EWEngine/Systems/PipelineSystem.h>

struct GrassBufferObject {
	float spacing;
	float height;
	float time;
	float windDir;
	glm::vec4 endDistance;
	float windStrength;
	VkBool32 displayLOD;
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