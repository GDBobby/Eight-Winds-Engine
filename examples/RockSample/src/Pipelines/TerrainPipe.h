#pragma once

#include <EWEngine/Systems/PipelineSystem.h>

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
