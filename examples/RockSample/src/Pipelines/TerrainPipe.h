#pragma once

#include <EWEngine/Systems/PipelineSystem.h>

struct TessBufferObject {
	glm::mat4 proj;
	glm::mat4 view;
	glm::vec4 frustumPlanes[6];
	glm::vec2 viewportDim;
	float displacementFactor;
	float tessFactor;
	float tessEdgeSize;
	int octaves;
	float worldPosNoiseScaling;
	float sandHeight;
	float grassHeight;
};

namespace EWE {
	class TerrainPipe : public PipelineSystem {
	public:
		TerrainPipe();
		~TerrainPipe() {}
        
		void CreatePipeLayout() final;
		void CreatePipeline() final;

		//EWEDescriptorSetLayout* vertexIndexBufferLayout{ nullptr };


	};
}
