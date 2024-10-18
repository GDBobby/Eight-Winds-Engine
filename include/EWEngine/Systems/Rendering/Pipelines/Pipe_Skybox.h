#pragma once

#include "EWEngine/Systems/PipelineSystem.h"

namespace EWE {
	class Pipe_Skybox : PipelineSystem {
	public:
		Pipe_Skybox();
		~Pipe_Skybox() override {}

	protected:
		void CreatePipeLayout() final;
		void CreatePipeline() final;
	};
}

