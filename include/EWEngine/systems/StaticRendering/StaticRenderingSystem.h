#pragma once
#include "EWEngine/graphics/model/EWE_Model.h"
#include "EWEngine/graphics/EWE_pipeline.h"

namespace EWE {
	class StaticRenderSystem {
	private:
		static StaticRenderSystem* skinnedMainObject;
		struct GPUStruct {
			std::unique_ptr<EWEModel> modelData;
			TextureID textureIDs;
			std::vector<TransformID> transformIDs{};
		};
		struct PipelineStruct {
			PipelineID pipeline;
			uint16_t pipeLayoutIndex; //a lot of work to find this value, might as well just store it
			std::vector<GPUStruct> objectData{};
			std::vector<uint32_t> freedTransformIDs{};
			/*
			PipelineStruct(std::unique_ptr<EWEPipeline> pipeline, GPUStruct& objectData)
				: pipeline{std::move(pipeline)}, objectData{std::move(objectData)}
			{}
			void addObject(std::unique_ptr<EWEModel> modelData, TextureID textureID) {

			}
			*/
		};
		std::vector<PipelineStruct> pipelineStructs{};
		std::unique_ptr<EWEBuffer> transformBuffer;
		uint32_t modelLimit;

		StaticRenderSystem(EWEDevice& device, uint32_t pipelineCount, uint32_t modelLimit);

	public:
		static void initStaticRS(EWEDevice& device, uint32_t pipelineCount, uint32_t modelLimit);
		static void destructStaticRS() {
			delete skinnedMainObject;
		}

		static bool addStaticObject(uint16_t PipelineID, std::unique_ptr<EWEModel>& model, TextureID textureIDs, TransformComponent& transform);
		static bool addStaticToBack();



	};
}
