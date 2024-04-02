#pragma once

#include "EWEngine/Systems/PipelineSystem.h"

namespace EWE {
	class Pipe_Skybox : PipelineSystem {
	public:
		Pipe_Skybox();

	protected:
		void CreatePipeLayout();
		void CreatePipeline();
	};
}

