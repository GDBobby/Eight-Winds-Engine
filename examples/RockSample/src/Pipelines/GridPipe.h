#pragma once

#include <EWEngine/Systems/PipelineSystem.h>
namespace EWE {
	class GridPipe : public PipelineSystem {
	public:
		GridPipe();

		void PushAndDraw(void* push) final;
	private:
		void CreatePipeLayout() final;
		void CreatePipeline() final;
	};
}
