#pragma once

#include <EWEngine/Systems/PipelineSystem.h>
namespace EWE {
	class BackgroundPipe : public PipelineSystem {
	public:
		BackgroundPipe();
		~BackgroundPipe() {}

		void DrawInstanced(EWEModel* model) final;
	private:
		void CreatePipeLayout() final;
		void CreatePipeline() final;

		//EWEDescriptorSetLayout* vertexIndexBufferLayout{ nullptr };


	};
}
