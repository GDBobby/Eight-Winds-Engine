#pragma once
class 
{
};

#pragma once

#include "TerrainPipe.h"
#include <EWEngine/Systems/PipelineSystem.h>

namespace EWE {
	class Simple_Terrain_Pipe : public PipelineSystem {
	public:
		Simple_Terrain_Pipe();
		~Simple_Terrain_Pipe() {}

		void CreatePipeLayout() final;
		void CreatePipeline() final;

		//EWEDescriptorSetLayout* vertexIndexBufferLayout{ nullptr };


	};
}
