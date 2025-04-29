#pragma once
#include <EWGraphics/PipelineSystem.h>

namespace EWE {
	class PerlinPipe : public PipelineSystem {
		PerlinPipe();
		~PerlinPipe();

		void CreatePipeLayout() final;
		void CreatePipeline() final;
	};
}

