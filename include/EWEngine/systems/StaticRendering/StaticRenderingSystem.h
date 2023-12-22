#pragma once
#include "../../EWE_Model.h"
#include "../../graphics/EWE_pipeline.h"

namespace EWE {
	class StaticRenderSystem {
		static StaticRenderSystem* skinnedMainObject;

		struct GPUStruct {
			glm::mat4 transform;
			glm::mat3 normalTransform;
			EWEModel* modelData;
			std::vector<glm::mat4> transforms;
			std::vector<glm::mat3> normalTransforms;
		};

		struct PipelineStruct {
			std::unique_ptr<EWEPipeline> pipeline;
			uint16_t pipeLayoutIndex; //a lot of work to find this value, might as well just store it
			std::unordered_map<TextureID, std::vector<EWEModel*>> skeletonData; //key is skeletonID


		};
	};
}
