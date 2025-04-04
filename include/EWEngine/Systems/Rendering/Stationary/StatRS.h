#pragma once
#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/Pipeline.h"

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
			std::vector<uint32_t> freedTransformIDs{};
		};
		std::vector<PipelineStruct> pipelineStructs{};
		std::unique_ptr<EWEBuffer> transformBuffer;
		uint32_t modelLimit;

		void Init(uint32_t pipelineCount, uint32_t modelLimit);

	public:

		static void InitStaticRS(uint32_t pipelineCount, uint32_t modelLimit);
		static void DestructStaticRS();

		static bool AddStaticObject(uint16_t PipelineID, std::unique_ptr<EWEModel>& model, ImageID imgID, TransformComponent& transform);
		static bool AddStaticToBack();



	};
}
