#pragma once

#include "EWEngine/Systems/PipelineSystem.h"

namespace EWE {
	class Pipe_SimpleTextured : public PipelineSystem {
		//largely, this should not be used in release.
		//this is a quick and easy way to get an object in
		//but objects should be added with the material system, using the albedo only material flag.
		//i dont really know, but im suspecting that adding an object in without a normal/rough/metal is going to be bad.
	public:
		Pipe_SimpleTextured();

	protected:
		void CreatePipeLayout() final;
		void CreatePipeline() final;

	};
}

