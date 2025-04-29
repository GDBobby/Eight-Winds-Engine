#pragma once

#include <EWGraphics/PipelineSystem.h>

struct TessBufferObject {
	lab::mat4 proj;
	lab::mat4 view;
	lab::vec4 frustumPlanes[6];
	lab::vec2 viewportDim;
	float displacementFactor;
	float tessFactor;
	float tessEdgeSize;
	int octaves;
	float worldPosNoiseScaling;
	float sandHeight;
	float grassHeight;
	int renderUnderwater = VK_TRUE;
};

namespace EWE {
	class TerrainPipe : public PipelineSystem {
	public:
		TerrainPipe();
		~TerrainPipe() override;
        
		void CreatePipeLayout() final;
		void CreatePipeline() final;

		//EWEDescriptorSetLayout* vertexIndexBufferLayout{ nullptr };


	};
}
