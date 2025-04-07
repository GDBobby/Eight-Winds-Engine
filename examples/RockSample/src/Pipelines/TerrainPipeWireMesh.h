#pragma once

#include <EWEngine/Systems/PipelineSystem.h>

namespace EWE {
	class TerrainPipeWireMesh : public PipelineSystem {
	public:
		TerrainPipeWireMesh();
		~TerrainPipeWireMesh() {}

		void CreatePipeLayout() final;
		void CreatePipeline() final;

	};

}