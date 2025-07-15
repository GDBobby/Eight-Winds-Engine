#pragma once

#include <EWGraphics/PipelineSystem.h>

namespace EWE {
	class VoxelTerrainPipe : public PipelineSystem {
	public:
		VoxelTerrainPipe();
		~VoxelTerrainPipe() override;
        
		void CreatePipeLayout() final;
		void CreatePipeline() final;
	};
}
