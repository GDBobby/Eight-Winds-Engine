#pragma once
#include "EWGraphics/Model/Model.h"
#include "EWGraphics/Vulkan/Pipeline.h"
#include "LAB/Transform.h"
#include "EWGraphics/Data/EngineDataTypes.h"

namespace EWE {
	class StaticRenderSystem {
	private:
		struct GPUStruct {
			std::unique_ptr<EWEModel> modelData;
			ImageID img;
			std::vector<TransformID> transformIDs{};
		};
		struct PipelineStruct {
			PipelineID pipeline;
			uint16_t pipeLayoutIndex; //a lot of work to find this value, might as well just store it
			std::vector<GPUStruct> objectData{};
			std::vector<PipelineID> freedTransformIDs{};
		};
		std::vector<PipelineStruct> pipelineStructs{};
		std::unique_ptr<EWEBuffer> transformBuffer;
		uint32_t modelLimit;

		void Init(uint32_t pipelineCount, uint32_t modelLimit);

	public:

		static void InitStaticRS(uint32_t pipelineCount, uint32_t modelLimit);
		static void DestructStaticRS();

		static bool AddStaticObject(uint16_t PipelineID, std::unique_ptr<EWEModel>& model, ImageID imgID, lab::Transform<float, 3>& transform);
		static bool AddStaticToBack();



	};
}
