/*
#pragma once

//#include "EWE_model.h"
#include "EWE_game_object.h"
#include "EWE_texture.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

//std
#include <map>
#include <thread>
//#include <list>


namespace EWE {
	class AssimpModel {
	public:
		//static std::map<int16_t, std::vector<MapInfo>> materialMeshes;

		std::string directory;
		bool gammaCorrection = false;

		//chatgpt recommended this to fix the map of std unique ptrs throwing an error
		//something about unique_ptr cant be copied but it can be moved
		//this code block fixed it
		AssimpModel() = default;
		AssimpModel(const AssimpModel& other) = delete;
		AssimpModel& operator=(const AssimpModel& other) = delete;
		AssimpModel(AssimpModel&& other) = default;
		AssimpModel& operator=(AssimpModel&& other) = default;
		//^chatgpt code block

		AssimpModel(EWEDevice& device, std::string const& path, bool gamma = false) : gammaCorrection{ gamma }, EWEDevice{device} {
			printf("assimp model path : %s \n", path.c_str());
			directory = path.substr(path.find_last_of('/') + 1, path.find_last_of('.') - path.find_last_of('/') - 1);
			printf("assimp model directory : %s \n", directory.c_str());
			if (directory == "ellen") {
				//printf("ellen meshes size : %d \n", meshes.size());
				hasMaterials = true;
				pipelineFlags = loadEllenTextures();
				if (pipelineFlags.size() == 0) {
					printf("throw an error, pipeline flags size is 0");
				}
				loadModel(path);
				printf("mesh count, texture count : %d:%d \n ", meshes.size(), textureIDs.size());
			}
			else {
				//std::thread modelThread{ &AssimpModel::loadModel, this, path };
				loadModel(path);
				loadTexture();
				//std::thread textureThread{ &AssimpModel::loadTexture, this };
				//loadModel(path);

				//modelThread.join();
			}
			//textureThread.join();
			printf("assimpModel constructor finished \n");
		}

		//~AssimpModel();
		

		TransformComponent transform{};
		bool hasMaterials = false;
		uint32_t materialMeshCount = 0;

		std::vector<int16_t> pipelineFlags;

		std::vector<std::unique_ptr<EWEModel>> meshes;
		std::vector<uint32_t> textureIDs;
		std::vector<uint32_t> normalIDs;

		//AssimpModel(EWEDevice& device, aiScene* scene) : EWEDevice{device} {}

		//auto& GetBoneInfoMap() { return m_BoneInfoMap; }
		//int& GetBoneCount() { return m_BoneCounter; }

	private:
		*
		std::map<std::string, BoneInfo> m_BoneInfoMap; //
		int m_BoneCounter = 0;
		
		void SetVertexBoneData(EWEModel::boneVertex& vertex, int boneID, float weight);
		void SetVertexBoneDataToDefault(EWEModel::boneVertex& vertex) {
			for (int i = 0; i < MAX_BONE_WEIGHTS; i++) {
				vertex.m_BoneIDs[i] = -1;
				vertex.m_Weights[i] = 0.0f;
			}
		}
		void ExtractBoneWeightForVertices(std::vector<EWEModel::AVertex>& vertices, aiMesh* mesh, const aiScene* scene);
		*
		
        void loadTexture() {
			if (directory == "arena") {
				loadArenaTextures();
			}
			else if (directory == "blenderObstacle") {
				loadObstacleTextures();
			}
			else if (directory == "Katana_export") {
				loadKatanaTexture();
			}
			else if (directory == "RoadCross") {
				printf("road cross meshes size : %d \n", meshes.size());
				loadRoadCrossTexture();
			}
			else if (directory == "SWANGAcaddy") {
				printf("caddy meshes size : %d \n", meshes.size());
				loadCaddyTexture();
			}
			printf("texture thread finished \n");
		}

		std::vector<int16_t> loadEllenTextures();
		void loadArenaTextures();
		void loadObstacleTextures(); //disabled currently
		void loadKatanaTexture();
		void loadRoadCrossTexture();
		void loadCaddyTexture();

		void loadMatTexture(aiMaterial* mat, aiTextureType type, std::string typeName);
		void loadModel(std::string path);
		EWEDevice& eweDevice;
		void processNode(aiNode* node, const aiScene* scene);
		std::unique_ptr<EWEModel> processMesh(aiMesh* mesh, const aiScene* scene);
	};
}
*/