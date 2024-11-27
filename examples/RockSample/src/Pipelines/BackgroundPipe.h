#pragma once

#include <EWEngine/Systems/PipelineSystem.h>
namespace EWE {
	class BackgroundPipe : public PipelineSystem {
	public:
		BackgroundPipe(EWEDevice& device);
		~BackgroundPipe() {
			if (vertexIndexBufferLayout) {
				delete vertexIndexBufferLayout;
			}
		}

		EWEDescriptorSetLayout& getVertexIndexBufferLayout() {
			return *vertexIndexBufferLayout;
		}

		void drawInstanced(EWEModel* model) override;
	private:
		void createPipeLayout(EWEDevice& device) override;
		void createPipeline(EWEDevice& device) override;

		EWEDescriptorSetLayout* vertexIndexBufferLayout{ nullptr };


	};
}
