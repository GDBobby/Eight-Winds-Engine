#pragma once

#include "EWEngine/Systems/PipelineSystem.h"

namespace EWE {
	class Pipe_Skybox : PipelineSystem {
	public:
		Pipe_Skybox(EWEDevice& device);

	protected:
		void createPipeLayout(EWEDevice& device);
		void createPipeline(EWEDevice& device);
	};
}

