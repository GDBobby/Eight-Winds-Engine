#include "TileMap.h"

#include <EWEngine/Graphics/Model/Vertex.h>
#include <EWEngine/Graphics/Texture/Image_Manager.h>
#include <EWEngine/Systems/PipelineSystem.h>

#include "../Pipelines/PipelineHeaderWrapper.h"

#include <fstream>
#include <filesystem>

const std::array<glm::vec4, 4> baseVertices = {
			glm::vec4(0.5f, 0.0f, 0.5f, 1.f),
			glm::vec4(-0.5f, 0.0f, 0.5f, 1.f),
			glm::vec4(-0.5f, 0.0f, -0.5f, 1.f),
			glm::vec4(0.5f, 0.0f, -0.5f, 1.f),
};

namespace EWE {

	//(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	std::array<uint32_t, 4> TileMap::getIndices(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
		return {
			//clockwise starting from top left
			(uint32_t)0 + x + (y * (width + 1)),
			(uint32_t)1 + x + (y * (width + 1)),
			(uint32_t)1 + x + ((y + 1) * (width + 1)),
			(uint32_t)0 + x + ((y + 1) * (width + 1)),
		};
	}
	std::array<uint32_t, 4> TileMap::getIndices(uint32_t tilePosition, uint16_t width, uint16_t height) {
		uint16_t x = tilePosition % width;
		uint16_t y = static_cast<uint16_t>(std::floor(static_cast<float>(tilePosition) / static_cast<float>(width)));
		return TileMap::getIndices(x, y, width, height);
	}

	TileMap::TileMap(std::string fileLocation, TileSet::TileSet_Enum tileSetID) : tileSet{tileSetID } {

		std::vector<glm::vec4> outVertices{};
		std::vector<uint32_t> indices{};
		buildTileMap(fileLocation, outVertices, indices);

		
	}
	TileMap::TileMap(std::vector<glm::vec4>& outVertices, std::vector<uint32_t>& indices, std::vector<glm::vec2>& tileUVs, TileSet::TileSet_Enum tileSetID) : tileSet{ tileSetID } {
		//buildTileMapByVertex(device, outVertices, indices, tileUVs);
	}
	TileMap::TileMap(TileSet::TileSet_Enum tileSetID) : tileSet{ tileSetID } {

	}

	void TileMap::renderTiles() {
		auto pipe = PipelineSystem::At(Pipe::background);

		pipe->BindPipeline();
		//pipe->bindModel(tileSet.tileModel.get());

		//pipe->bindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameInfo.index));
		//pipe->bindDescriptor(1, &descriptorSet);
		//pipe->bindTextureDescriptor(2, tileSet.tileSetTexture);

		pipe->BindDescriptor(0, &descriptorSet);

		//ModelPushData push;
		//push.modelMatrix = floorTransform.mat4();
		//push.normalMatrix = floorTransform.normalMatrix();

		PushTileConstantData push{};
		
		//push.uvScroll.x = 1.f / static_cast<float>(tileSet.width);
		//push.uvScroll.y = 1.f / static_cast<float>(tileSet.height);
		pipe->Push(&push);

		pipe->DrawInstanced(tileSet.tileModel); 
		EWE_VK(vkCmdDraw, VK::Object->GetFrameBuffer(), 6, width * height, 0, 0);
	}
	void TileMap::buildTileMap(std::string const& fileLocation,
		std::vector<glm::vec4>& outVertices, std::vector<uint32_t>& indices) {
		std::string fileReal = "models/";
		fileReal += fileLocation;
		std::ifstream inStream{ fileReal };
		if (!inStream.is_open()) {
			if (!std::filesystem::exists(fileLocation)) {
				printf("loaded map doesn't exist : %s \n", fileLocation.c_str());
				assert(false);
			}
			inStream.open(fileLocation);
			if (!inStream.is_open()) {
				printf("failed to load map twice : %s \n", fileLocation.c_str());
				assert(false);
			}
		}
		inStream >> width;
		inStream >> height;
		if ((width == 0) || (height == 0)) {
			printf("invalid map size \n");
			assert(false);
		}

		uint32_t tileCount = width * height;
		tileFlags.reserve(tileCount);
		TransformComponent transform;
		transform.scale = glm::vec3{ tileSet.tileScale };
		indices.reserve(tileCount * 6);
		outVertices.reserve((width + 1) * (height + 1));
		glm::vec3 baseTranslation{ -static_cast<float>(width) * tileSet.tileScale / 2.f, 0.f, -static_cast<float>(height) * tileSet.tileScale / 2.f };
		printf("base translation - %.5f:%.5f:%.5f\n", baseTranslation.x, baseTranslation.y, baseTranslation.z);
		transform.translation = baseTranslation;
		glm::vec2 offsetPerTile{ 0.5f };
		glm::vec2 uvScaling{ 1.f / tileSet.width, 1.f / tileSet.height };

		std::vector<glm::vec2> tileUVOffset(width * height);


		uint32_t tileBuffer;
		glm::mat4 mat4{};

		//square 0,0
		inStream >> tileBuffer;
		//buildTileSquare(tileBuffer, transform, mat4, tileUVOffset[0]);

		//glm::vec4 buffer = mat4 * baseVertices[0];
		//printf("first method : (%.5f):(%.5f):(%.5f):(%.5f)\n", buffer.x, buffer.y, buffer.z, buffer.w);
		//buffer = baseVertices[0] * mat4;
		//printf("second method : (%.5f):(%.5f):(%.5f):(%.5f)\n", buffer.x, buffer.y, buffer.z, buffer.w);

		indices.push_back(0);
		outVertices.push_back(mat4 * baseVertices[0]);

		indices.push_back(1);
		glm::vec4 buffer = mat4 * baseVertices[1];
		printf("second vertex : (%.5f):(%.5f):(%.5f):(%.5f)\n", buffer.x, buffer.y, buffer.z, buffer.w);
		outVertices.push_back(mat4 * baseVertices[1]);

		indices.push_back(2);
		outVertices.push_back(mat4 * baseVertices[2]);
		indices.push_back(2);

		indices.push_back(3);
		outVertices.push_back(mat4 * baseVertices[3]);

		indices.push_back(0);

		//row 1 - 0,1 - 0,2 - 0,3...
		for (int x = 1; x < width; x++) {
			inStream >> tileBuffer;
			transform.translation = baseTranslation;
			transform.translation.x += offsetPerTile.x * static_cast<float>(x);
			//buildTileSquare(tileBuffer, transform, mat4, tileUVOffset[x]);

			indices.push_back(outVertices.size());
			outVertices.push_back(mat4 * baseVertices[0]);

			indices.push_back(indices[indices.size() - 7]);
			indices.push_back(indices[indices.size() - 4]);
			indices.push_back(indices.back());


			indices.push_back(outVertices.size());
			outVertices.push_back(mat4 * baseVertices[3]);
			indices.push_back(outVertices.size() - 2);

		}

		for (int y = 1; y < height; y++) {
			inStream >> tileBuffer;
			transform.translation = baseTranslation;
			transform.translation.z += offsetPerTile.y * static_cast<float>(y);
			//buildTileSquare(tileBuffer, transform, mat4, tileUVOffset[y * width]);
			indices.push_back(outVertices.size());

			outVertices.push_back(mat4 * baseVertices[0]);


			indices.push_back(outVertices.size());

			outVertices.push_back(mat4 * baseVertices[1]);

			indices.push_back(indices[indices.size() - (width * 6 + 1)]);
			indices.push_back(indices.back());

			indices.push_back(indices[indices.size() - (width * 6 + 4)]);

			indices.push_back(indices[indices.size() - 5]);


			//square 1 1

			for (int x = 1; x < width; x++) {
				inStream >> tileBuffer;
				transform.translation.x = baseTranslation.x + offsetPerTile.x * static_cast<float>(x);
				//buildTileSquare(tileBuffer, transform, mat4, tileUVOffset[y * width + x]);

				outVertices.push_back(mat4 * baseVertices[0]);
				indices.push_back(outVertices.size() - 1);

				indices.push_back(indices[indices.size() - 7]);

				indices.push_back(indices[indices.size() - (width * 6 + 1)]);
				indices.push_back(indices.back());

				indices.push_back(indices[indices.size() - (width * 6 + 4)]);

				indices.push_back(outVertices.size() - 1);

			}
		}


		tileVertexBuffer = Construct<EWEBuffer>({ sizeof(glm::vec4) * outVertices.size(), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT });
		tileVertexBuffer->Map();

		printf("storage buffer alignment : %lu \n", VK::Object->properties.limits.minStorageBufferOffsetAlignment);
		printf("tile vertex 1 : %.2f:%.2f:%.2f:%.2f \n", outVertices[0].x, outVertices[1].y, outVertices[2].z, outVertices[3].w);
		//for (int i = 0; i < outVertices.size(); i++) {
		//	tileVertexBuffer->writeToBufferAligned(&outVertices[i], sizeof(outVertices[0]), i);
		//}
		tileVertexBuffer->WriteToBuffer(outVertices.data(), outVertices.size() * sizeof(outVertices[0]));
		tileVertexBuffer->Flush();
		tileSet.tileModel->AddInstancing(tileUVOffset.size(), sizeof(glm::vec2), tileUVOffset.data(), Queue::transfer);

		tileIndexBuffer = Construct<EWEBuffer>({ sizeof(uint32_t) * indices.size(), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT });
		tileIndexBuffer->Map();
		//for (int i = 0; i < indices.size(); i++) {
		//	tileIndexBuffer->writeToBufferAligned(&indices[i], sizeof(uint32_t), i);
		//}

		tileIndexBuffer->WriteToBuffer(indices.data(), indices.size() * sizeof(indices[0]));
		tileVertexBuffer->Flush();
		//just need to create the storage buffer for the index and vertices now
		//descriptorSet = EWEDescriptorWriter(PipelineSystem::At(Pipe_background)->GetDSL(), DescriptorPool_Global)
		//	.WriteBuffer(0, tileVertexBuffer->DescriptorInfo())
		//	.WriteBuffer(1, tileIndexBuffer->DescriptorInfo())
		//	.Build();


		EWEDescriptorWriter descWriter(PipelineSystem::At(Pipe::background)->GetDSL(), DescriptorPool_Global);
		DescriptorHandler::AddGlobalsToDescriptor(descWriter, 0);
		descWriter.WriteBuffer(2, tileVertexBuffer->DescriptorInfo());
		descWriter.WriteBuffer(3, tileIndexBuffer->DescriptorInfo());
		descWriter.WriteBuffer(4, tileUVBuffer->DescriptorInfo());
		descWriter.WriteImage(5, Image_Manager::GetDescriptorImageInfo(tileSet.tileSetImage));
		descriptorSet = descWriter.Build();
	}

	void TileMap::buildTileMapByVertex(std::vector<glm::vec4>& outVertices, std::vector<uint32_t>& indices, std::vector<glm::vec2>& tileUVs) {



		createTileVertices(outVertices, width, height, tileSet.tileScale);
		createTileIndices(indices, width, height);

		createTileUVs(tileUVs, width, height, tileSet.tileSize, tileSet.tileSize);


		tileVertexBuffer = EWEBuffer::CreateAndInitBuffer(outVertices.data(), sizeof(outVertices[0]), outVertices.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		//tileSet.tileModel->AddInstancing(tileUVOffset.size(), sizeof(glm::vec2), tileUVOffset.data());

		tileIndexBuffer = EWEBuffer::CreateAndInitBuffer(indices.data(), sizeof(indices[0]), indices.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		//just need to create the storage buffer for the index and vertices now

		tileUVBuffer = EWEBuffer::CreateAndInitBuffer(tileUVs.data(), sizeof(tileUVs[0]), tileUVs.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		//descriptorSet = EWEDescriptorWriter(&((BackgroundPipe*)PipelineSystem::At(Pipe_background))->getVertexIndexBufferLayout(), DescriptorPool_Global)
		//	.WriteBuffer(0, tileVertexBuffer->DescriptorInfo())
		//	.WriteBuffer(1, tileIndexBuffer->DescriptorInfo())
		//	.WriteBuffer(2, tileUVBuffer->DescriptorInfo())
		//	.Build();

		EWEDescriptorWriter descWriter(PipelineSystem::At(Pipe::background)->GetDSL(), DescriptorPool_Global);
		DescriptorHandler::AddGlobalsToDescriptor(descWriter, 0);
		descWriter.WriteBuffer(2, tileVertexBuffer->DescriptorInfo());
		descWriter.WriteBuffer(3, tileIndexBuffer->DescriptorInfo());
		descWriter.WriteBuffer(4, tileUVBuffer->DescriptorInfo());
		descWriter.WriteImage(5, Image_Manager::GetDescriptorImageInfo(tileSet.tileSetImage));
		descriptorSet = descWriter.Build();
	}

	void TileMap::createTileVertices(std::vector<glm::vec4>& outVertices, int width, int height, float tileScale) {
		glm::vec3 baseTranslation{ -static_cast<float>(width) * tileScale / 2.f, 0.f, -static_cast<float>(height) * tileScale / 2.f };
		//printf("base translation - %.5f:%.5f:%.5f\n", baseTranslation.x, baseTranslation.y, baseTranslation.z);
		glm::vec2 offsetPerTile{ 0.5f };

		outVertices.reserve((width + 1) * (height + 1));
		glm::vec4 vertexTrans{ baseTranslation, 0.f };
		vertexTrans.y -= offsetPerTile.y * 1.5f;
		//vertices right here
		for (uint32_t y = 0; y < height + 1; y++) {
			vertexTrans.x = baseTranslation.x - offsetPerTile.y * 1.5f;
			vertexTrans.y += offsetPerTile.y;
			for (uint32_t x = 0; x < width + 1; x++) {
				//buildTileSquare(tileBuffer, transform, mat4, tileUVOffset[x + y * width]);
				vertexTrans.x += offsetPerTile.x;
				outVertices.push_back(vertexTrans);
			}
		}
	}
	void TileMap::createTileIndices(std::vector<uint32_t>& indices, int width, int height) {
		indices.reserve(height * width * 6);
		for (uint32_t y = 0; y < height; y++) {
			for (uint32_t x = 0; x < width; x++) {
				auto indexes = getIndices(x, y, width, height);
				indices.insert(indices.end(), indexes.begin(), indexes.end());


			}
		}
	}
	void TileMap::createTileUVs(std::vector<glm::vec2>& tileUVs, int width, int height, uint32_t tileWidth, uint32_t tileHeight) {
		tileUVs.reserve(width * height * 6);
		for (uint32_t y = 0; y < height; y++) {
			for (uint32_t x = 0; x < width; x++) {

				//indices
				tileUVs.emplace_back(0.f, 0.f);
				tileUVs.emplace_back(1.f / static_cast<float>(tileWidth), 0.f);
				tileUVs.emplace_back(1.f / static_cast<float>(tileWidth), 1.f / static_cast<float>(tileHeight));
				tileUVs.emplace_back(1.f / static_cast<float>(tileWidth), 1.f / static_cast<float>(tileHeight));
				tileUVs.emplace_back(0.f, 1.f / static_cast<float>(tileHeight));
				tileUVs.emplace_back(0.f, 0.f);
			}
		}
	}

	void TileMap::findTileFlags(std::vector<TileFlag>& tileFlags, int width, int height) {

	}
}