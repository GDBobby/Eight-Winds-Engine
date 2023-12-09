/*
#pragma once

#include "graphics/EWE_texture.h"
#include "EWE_GameObject.h"
#include "ObjectManager.h"
#include "graphics/EWE_FrameInfo.h"

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

//binary
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

void inline convertVec3(glm::vec3& glmvec3, float* setter) {
	glmvec3.x = setter[0];
	glmvec3.y = setter[1];
	glmvec3.z = setter[2];
}
void inline convertVec4(glm::vec4& glmvec4, float* setter3, float wSetter) {
	glmvec4.x = setter3[0];
	glmvec4.y = setter3[1];
	glmvec4.z = setter3[2];
	glmvec4.w = wSetter;
}

#define MAX_LB_PATH 128 //this is the max length a string can be in level builder, specifically for imgui input boxes

namespace EWE {
	struct BobTransform {
		float translation[3];
		float scale[3];
		float rotation[3];
		void operator=(TransformComponent& transform) {
			//memcpy(translation, &transform.translation, sizeof(translation));
			translation[0] = transform.translation.x;
			translation[1] = transform.translation.y;
			translation[2] = transform.translation.z;
			//memcpy(rotation, &transform.rotation, sizeof(rotation));
			rotation[0] = transform.rotation.x;
			rotation[1] = transform.rotation.y;
			rotation[2] = transform.rotation.z;
			//memcpy(scale, &transform.scale, sizeof(scale));
			scale[0] = transform.scale.x;
			scale[1] = transform.scale.y;
			scale[2] = transform.scale.z;
		}
		BobTransform(TransformComponent& transform) {
			memcpy(translation, &transform.translation, sizeof(translation));
			memcpy(rotation, &transform.rotation, sizeof(rotation));
			memcpy(scale, &transform.scale, sizeof(scale));
		}
		BobTransform() {
			translation[0] = 0.f;
			translation[1] = 0.f;
			translation[2] = 0.f;

			rotation[0] = 0.f;
			rotation[1] = 0.f;
			rotation[2] = 0.f;

			scale[0] = 1.f;
			scale[1] = 1.f;
			scale[2] = 1.f;
		}

		template<class Archive>
		void serialize(Archive& archive, const unsigned int version) {
			archive& translation;
			archive& scale;
			archive& rotation;
		}
	};
	void inline convertTransform(TransformComponent& transform, BobTransform& bobTransform) {
		convertVec3(transform.translation, bobTransform.translation);
		convertVec3(transform.rotation, bobTransform.rotation);
		convertVec3(transform.scale, bobTransform.scale);
	}

	class LevelExportData {
	public:
		enum Map_Type {
			pvp,
			target,
			obstacle,
			builder,
		} mapType;

		struct TangentObjectData {
			std::vector<bobAVertex> vertices;
			std::vector<uint32_t> indices;
			std::string texturePath = "";
			EWETexture::texture_type textureType = EWETexture::tType_smart;
			BobTransform bTransform;

			TangentObjectData(std::vector<bobAVertex>& vertex, std::vector<uint32_t>& index, EWETexture::TextureData& textureData, TransformComponent& transform) : vertices{ vertex }, indices{ index }, bTransform{ transform } {
				//printf("material object constructor? \n");
				//indices = index;
				texturePath = textureData.path;
				textureType = textureData.tType;
			}
			TangentObjectData() { }
			template<class Archive>
			void serialize(Archive& archive, const unsigned int version) {
				archive& vertices;
				archive& indices;
				archive& texturePath;
				archive& textureType;
				archive& bTransform;
			}
		};
		struct NoTangentObjectData {
			std::vector<bobAVertexNT> vertices;
			std::vector<uint32_t> indices;
			std::string texturePath = "";
			EWETexture::texture_type textureType = EWETexture::tType_none;
			BobTransform bTransform;

			NoTangentObjectData(std::vector<bobAVertexNT>& vertex, std::vector<uint32_t>& index, EWETexture::TextureData& textureData, TransformComponent& transform) : vertices{ vertex }, indices{ index }, bTransform{ transform } {
				indices = index;
				texturePath = textureData.path;
				textureType = textureData.tType;
			}
			NoTangentObjectData() {}
			template<class Archive>
			void serialize(Archive& archive, const unsigned int version) {
				archive& vertices;
				archive& indices;
				archive& texturePath;
				archive& textureType;
				archive& bTransform;
			}
		};

		struct SunValues {
			float direction[3];
			float color[3];
			float intensity{ 0.f };

			SunValues() { }
			SunValues(float* dir, float* col, float intensity) : intensity{ intensity } {
				memcpy(direction, dir, sizeof(direction));
				memcpy(color, col, sizeof(color));
			}
			SunValues(float dirX, float dirY, float dirZ, float colR, float colG, float colB, float intensity) : intensity{ intensity } {
				direction[0] = dirX;
				direction[1] = dirY;
				direction[2] = dirZ;

				color[0] = colR;
				color[1] = colG;
				color[2] = colB;
			}

			template<class Archive>
			void serialize(Archive& archive, const unsigned int version) {
				archive& direction;
				archive& color;
				archive& intensity;
			}
		};
		std::vector<NoTangentObjectData> ntObjects;
		std::vector<TangentObjectData> tangentObjects;
		SunValues sun;
		std::string versionTracker = "1.0.0";
		std::string targetPathName = ""; //if targetPath = "" then not a target map
		uint32_t targetTimer = 30;
		uint32_t targetCount = 0;
		BobTransform startLocation;

		template<class Archive>
		void serialize(Archive& archive, const unsigned int version) {
			archive& versionTracker;
			archive& ntObjects;
			archive& tangentObjects;
			archive& sun;
			archive& targetPathName;
			archive& targetTimer;
			archive& startLocation;
		}
	};

	//for loading and saving levels
	class EweLevelLoader {
		EweLevelLoader(const EweLevelLoader&) = delete;
		EweLevelLoader& operator=(const EweLevelLoader&) = delete;
		EweLevelLoader(EweLevelLoader&&) = default;
		EweLevelLoader& operator=(EweLevelLoader&&) = default;
	public:
		static void saveLevel(std::string levelName, ObjectManager* objectManager, std::map<uint32_t, std::map<uint32_t, EWEGameObject>*>& objectList, LevelExportData::SunValues sunVal, std::string targetPath, TransformComponent& spawnPosition, uint32_t targetTimer);
		static void loadLevelToTarget(EWEDevice& device, std::string levelName, ObjectManager* objectManager, LightBufferObject& lbo, LevelExportData::Map_Type mapType, TransformComponent& spawnTransform, uint32_t& targetTimer);
		static uint32_t loadLevelToBuilder(EWEDevice& device, std::string levelName, ObjectManager* objectManager, std::map<uint32_t, std::map<uint32_t, EWEGameObject>*>& objectList, LightBufferObject& lbo, LevelExportData::Map_Type mapType, TransformComponent& spawnTransform, char* targetPath, uint32_t& targetTimer);
	private:
		//LevelExportData levelExport;
	};
}
*/
