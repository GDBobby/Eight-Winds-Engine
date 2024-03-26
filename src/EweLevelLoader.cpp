
/*
#include "levelbuilder/EweLevelLoader.h"

namespace EWE {
	uint32_t EweLevelLoader::loadLevelToBuilder(std::string levelName, ObjectManager* objectManager, std::map<uint32_t, std::map<uint32_t, EWEGameObject>*>& objectList, LightBufferObject& lbo, LevelExportData::Map_Type mapType, TransformComponent& spawnTransform, char* targetPath, uint32_t& targetTimer) {
		uint32_t objectCounter = 0;
		if (mapType == LevelExportData::builder) {
			LevelExportData levelImport;

			std::string levelNameSub = levelName;
			levelNameSub = levelNameSub.substr(levelNameSub.find_last_of("/"));
			levelNameSub = levelNameSub.substr(levelNameSub.find_last_of("\\"));
			levelNameSub = levelNameSub.substr(0, levelNameSub.find_first_of("."));
			if (levelNameSub.length() == 0) {
				return 0;
			}
			levelNameSub = "TargetLevels\\" + levelNameSub + ".ewm";

			std::ifstream inFile(levelNameSub, std::ifstream::binary);
			boost::archive::binary_iarchive binary_input_archive(inFile, boost::archive::no_header);
			binary_input_archive& levelImport;
			printf("loaded file %s \n", levelName.c_str());
			inFile.close();
			if (levelImport.targetPathName.length() < MAX_LB_PATH) {
				std::copy(levelImport.targetPathName.begin(), levelImport.targetPathName.end(), targetPath);
				//for (int i = 0; i < levelImport.targetPathName.length(); i++) {
				//	targetPath[i] = levelImport.targetPathName[i];
				//}
				targetPath[levelImport.targetPathName.length()] = '\0';
			}
			else {
				printf("import path name was too long \n");
				return 0;
			}
			targetTimer = levelImport.targetTimer;
			printf("load timer %d \n", targetTimer);
			//targetPath[levelImport.targetPathName.length()] = '/0';
			convertVec4(lbo.sunlightColor, levelImport.sun.color, levelImport.sun.intensity);
			convertTransform(spawnTransform, levelImport.startLocation);
			printf("spawn translation y? %.1f \n", levelImport.startLocation.translation[1]);
			convertVec4(lbo.sunlightDirection, levelImport.sun.direction, 0.f);
			//lbo.sunlightDirection.x = levelImport.sun.direction[0];
			//lbo.sunlightDirection.y = levelImport.sun.direction[1];
			//lbo.sunlightDirection.z = levelImport.sun.direction[2];
			//lbo.sunlightDirection.w = 0.f;
			if (levelImport.mapType == LevelExportData::Map_Type::target) {
				auto tempTextureID = EWETexture::addSceneMaterialTexture(device, levelImport.targetPathName, EWETexture::tType_smart);
				if ((tempTextureID.first >= 0) && (tempTextureID.second >= 0)) {
					objectManager->targetTexturePair = tempTextureID;
				}
				else {
					printf("failed to load target texture into builder, tex was bad - %d:%d \n", tempTextureID.first, tempTextureID.second);
				}
			}

			//PlayerObject::poVector[0].setSpawnLocation(levelImport.startLocation.translation, levelImport.startLocation.rotation);
			spawnTransform.translation.x = levelImport.startLocation.translation[0];
			spawnTransform.translation.y = levelImport.startLocation.translation[1];
			spawnTransform.translation.z = levelImport.startLocation.translation[2];

			spawnTransform.rotation.x = levelImport.startLocation.rotation[0];
			spawnTransform.rotation.y = levelImport.startLocation.rotation[1];
			spawnTransform.rotation.z = levelImport.startLocation.rotation[2];

			for (int i = 0; i < levelImport.tangentObjects.size(); i++) {
				EWETexture::texture_type textureType = levelImport.tangentObjects[i].textureType;
				if (textureType == EWETexture::tType_smart) {
					auto textureID = EWETexture::addSceneMaterialTexture(device, levelImport.tangentObjects[i].texturePath, EWETexture::tType_smart);
					if (textureID.second >= 0 && textureID.first >= 0) {
						objectList[objectCounter] = &objectManager->dynamicBuildObjects;
						objectManager->dynamicBuildObjects[objectCounter].model = std::make_shared<BuilderModel>(device, levelImport.tangentObjects[i].vertices, levelImport.tangentObjects[i].indices);
						objectManager->dynamicBuildObjects[objectCounter].textureFlags = textureID.first;
						objectManager->dynamicBuildObjects[objectCounter].textureID = textureID.second;
						convertTransform(objectManager->dynamicBuildObjects[objectCounter].transform, levelImport.tangentObjects[i].bTransform);
						if (objectManager->targetTexturePair.second == textureID.second) {
							objectManager->dynamicBuildObjects[objectCounter].isTarget = true;
						}
						objectCounter++;
					}
					else {
						printf("trying to load a bad material texture - %d:%d \n", textureID.first, textureID.second);
					}
				}
				else {
					printf("importing tangent without smart texture? \n");
				}
			}
			for (int i = 0; i < levelImport.ntObjects.size(); i++) {
				auto textureID = EWETexture::addSmartModeTexture(device, levelImport.ntObjects[i].texturePath, EWETexture::tType_smart);
				if ((textureID.first) >= 0 && (textureID.second >= 0)) {
					objectList[objectCounter] = &objectManager->dynamicBuildObjects;
					objectManager->dynamicBuildObjects[objectCounter].model = std::make_shared<BuilderModel>(device, levelImport.ntObjects[i].vertices, levelImport.ntObjects[i].indices);
					objectManager->dynamicBuildObjects[objectCounter].textureFlags = textureID.first;
					objectManager->dynamicBuildObjects[objectCounter].textureID = textureID.second;
					convertTransform(objectManager->dynamicBuildObjects[objectCounter].transform, levelImport.ntObjects[i].bTransform);
					objectCounter++;
				}
				else {
					printf("trying to load a non-smart texture? - %d:%d \n", textureID.first, textureID.second);
				}
			}
		}
		else {
			printf(" 1111 Calling the builder level loader without being in builder? \n");
		}
		return objectCounter;
	}

	void EweLevelLoader::loadLevelToTarget(std::string levelName, ObjectManager* objectManager, LightBufferObject& lbo, LevelExportData::Map_Type mapType, TransformComponent& spawnTransform, uint32_t& targetTimer) {
		LevelExportData levelImport;

		std::ifstream inFile(levelName, std::ifstream::binary);
		printf("before importing \n");
		boost::archive::binary_iarchive binary_input_archive(inFile, boost::archive::no_header);
		printf("after importing \n");
		binary_input_archive& levelImport;
		printf("loaded filesssssssss %s \n", levelName.c_str());
		inFile.close();
		//char targetPath[128] = levelImport.targetPathName.c_str();
		convertVec4(lbo.sunlightColor, levelImport.sun.color, levelImport.sun.intensity);
		convertTransform(spawnTransform, levelImport.startLocation);
		printf("spawn translation y? %.1f \n", levelImport.startLocation.translation[1]);
		//spawnTransform = levelImport.startLocation;
		convertVec4(lbo.sunlightDirection, levelImport.sun.direction, 0.f);
		//lbo.sunlightDirection.x = levelImport.sun.direction[0];
		//lbo.sunlightDirection.y = levelImport.sun.direction[1];
		//lbo.sunlightDirection.z = levelImport.sun.direction[2];
		//lbo.sunlightDirection.w = 0.f;
		TextureID targetTextureID;
		if (mapType == LevelExportData::Map_Type::target) {
			auto tempTextureID = EWETexture::addSceneMaterialTexture(device, levelImport.targetPathName, EWETexture::tType_smart);
			printf("target texture id? %d \n", tempTextureID.second);
			if ((tempTextureID.first) >= 0 && (tempTextureID.second >= 0)) {
				targetTextureID = tempTextureID.second;
			}
			else {
				printf("loaded bad texture for target - %d:%d \n", tempTextureID.first, tempTextureID.second);
				throw std::exception("bad target map \n");
			}
		}
		targetTimer = levelImport.targetTimer;
		printf("load targetTimer %d \n", targetTimer);
		//PlayerObject::setSpawnLocation(0, levelImport.startLocation.translation, levelImport.startLocation.rotation);
		//PlayerObject::poVector[0].setSpawnLocation(levelImport.startLocation.translation, levelImport.startLocation.rotation);

		spawnTransform.translation.x = levelImport.startLocation.translation[0];
		spawnTransform.translation.y = levelImport.startLocation.translation[1];
		spawnTransform.translation.z = levelImport.startLocation.translation[2];

		spawnTransform.rotation.x = levelImport.startLocation.rotation[0];
		spawnTransform.rotation.y = levelImport.startLocation.rotation[1];
		spawnTransform.rotation.z = levelImport.startLocation.rotation[2];

		if (mapType != LevelExportData::builder) {



			for (int i = 0; i < levelImport.tangentObjects.size(); i++) {
				EWETexture::texture_type textureType = levelImport.tangentObjects[i].textureType;
				if (textureType == EWETexture::tType_smart) {
					auto textureID = EWETexture::addSceneMaterialTexture(device, levelImport.tangentObjects[i].texturePath, textureType);
					
					if ((textureID.second >= 0) && (textureID.first >= 0)) {
						objectManager->materialGameObjects.emplace_back();
						objectManager->materialGameObjects.back().model = EWEModel::createMesh(device, levelImport.tangentObjects[i].vertices, levelImport.tangentObjects[i].indices);
						objectManager->materialGameObjects.back().textureID = textureID.second;
						objectManager->materialGameObjects.back().textureFlags = textureID.first;
						convertTransform(objectManager->materialGameObjects.back().transform, levelImport.tangentObjects[i].bTransform);
						if (objectManager->targetTexturePair.second == textureID.second) {
							objectManager->materialGameObjects.back().isTarget = true;
							objectManager->maxTargets++;
						}
						else {
							objectManager->materialGameObjects.back().giveCollision();
						}
					}
					else {
						printf("trying to load a bad material texture - %d:%d \n", textureID.first, textureID.second);
					}
				}
				else {
					printf("importing tangent no smart texture? \n");
				}
			}

			for (int i = 0; i < levelImport.ntObjects.size(); i++) {
				EWETexture::texture_type textureType = levelImport.ntObjects[i].textureType;
				if (textureType == EWETexture::tType_smart) {
					auto textureID = EWETexture::addSmartModeTexture(device, levelImport.ntObjects[i].texturePath, textureType);
					if ((textureID.first >= 0) && (textureID.second >= 0)) {
						objectManager->materialGameObjects.emplace_back();
						objectManager->materialGameObjects.back().model = EWEModel::createMesh(device, levelImport.ntObjects[i].vertices, levelImport.ntObjects[i].indices);
						objectManager->materialGameObjects.back().textureFlags = textureID.first;
						objectManager->materialGameObjects.back().textureID = textureID.second;
						convertTransform(objectManager->materialGameObjects.back().transform, levelImport.ntObjects[i].bTransform);
						if (objectManager->targetTexturePair.second == textureID.second) {
							objectManager->materialGameObjects.back().isTarget = true;
							objectManager->maxTargets++;
						}
						else {
							objectManager->materialGameObjects.back().giveCollision();
						}
					}
					else {
						printf("trying to load a bad material texture - %d:%d \n", textureID.first, textureID.second);
					}
				}
				if (textureType == EWETexture::tType_none) {
					printf("please dont import a simple object \n");
				}
				else {
					printf("importing simple object that has a material texture? %d \n", textureType);
				}
			}


		}
		else {
			printf(" 0000 calling builder load while not in builder? \n");
		}
	}


	void EweLevelLoader::saveLevel(std::string levelName, ObjectManager* objectManager, std::map<uint32_t, std::map<uint32_t, EWEGameObject>*>& objectList, LevelExportData::SunValues sunVal, std::string targetPath, TransformComponent& spawnPosition, uint32_t targetTimer) {
		LevelExportData levelExport;
		printf("beginning of save level \n");
		levelExport.targetPathName = targetPath;
		levelExport.targetTimer = targetTimer;
		printf("save target timer : %d \n", levelExport.targetTimer);

		printf("saving level target name : %s \n", levelExport.targetPathName.c_str());
		levelExport.sun = sunVal;
		levelExport.startLocation = spawnPosition;
		printf("spawn translation y? %.1f \n", levelExport.startLocation.translation[1]);
		for (auto iter = objectList.begin(); iter != objectList.end(); iter++) {
			//printf("each iter? \n");
			if (iter->second == &objectManager->dynamicBuildObjects) {
				auto textureData = EWETexture::getTextureData(iter->second->at(iter->first).textureID);
				BuilderModel* buildModel = ((BuilderModel*)iter->second->at(iter->first).model.get());
				if (buildModel->vType == BuilderModel::vT_NT) {
					levelExport.ntObjects.emplace_back(buildModel->verticesNT, buildModel->indices, textureData, iter->second->at(iter->first).transform);
				}
				else if (buildModel->vType == BuilderModel::vT_tangent) {
					levelExport.tangentObjects.emplace_back(buildModel->verticesTangent, buildModel->indices, textureData, iter->second->at(iter->first).transform);
				}
				else {
					printf("whats this vType? : %d \n", buildModel->vType);
				}
			}
			*
			if (iter->second == &objectManager->builderObjects) {
				//printf("simple objects \n");
				//if (((BuilderModel*)iter->second->at(iter->first).model.get())->simpleVertices.size() > 0) {
					//auto tempVec = &(((BuilderModel*)iter->second->at(iter->first).model.get())->simpleVertices);
				levelExport.simpleObjects.emplace_back(
					((BuilderModel*)iter->second->at(iter->first).model.get())->verticesNT,
					((BuilderModel*)iter->second->at(iter->first).model.get())->indices,
					iter->second->at(iter->first).transform
				);
			}
			else if (iter->second == &objectManager->texturedBuildObjects) {
				//printf("textured objects \n");
				levelExport.simpleObjects.emplace_back(
					((BuilderModel*)iter->second->at(iter->first).model.get())->verticesNT,
					((BuilderModel*)iter->second->at(iter->first).model.get())->indices,
					EWETexture::getTextureData(iter->second->at(iter->first).textureID),
					iter->second->at(iter->first).transform
				);
				levelExport.targetCount += (levelExport.simpleObjects.back().texturePath == levelExport.targetPathName);
			}
			else if (iter->second == &objectManager->materialBuildObjects) {
				//printf("Materrial objects \n");
				//printf("vertex size : %d ~ indices size : %d ~ textureID : %d \n", ((BuilderModel*)iter->second->at(iter->first).model.get())->materiaEWErtices.size(), ((BuilderModel*)iter->second->at(iter->first).model.get())->indices.size(), iter->second->at(iter->first).textureID);
				levelExport.materialObjects.emplace_back(
					((BuilderModel*)iter->second->at(iter->first).model.get())->verticesTangent,
					((BuilderModel*)iter->second->at(iter->first).model.get())->indices,
					EWETexture::getTextureData(iter->second->at(iter->first).textureID),
					iter->second->at(iter->first).transform
				);
				//printf("after emplace back \n");
				levelExport.targetCount += (levelExport.materialObjects.back().texturePath == levelExport.targetPathName);
			}
			else if (iter->second == &objectManager->transparentBuildObjects) {
				//printf("transparent objects \n");
				levelExport.materialObjects.emplace_back(
					((BuilderModel*)iter->second->at(iter->first).model.get())->verticesTangent,
					((BuilderModel*)iter->second->at(iter->first).model.get())->indices,
					EWETexture::getTextureData(iter->second->at(iter->first).textureID),
					iter->second->at(iter->first).transform,
					true
				);
				levelExport.targetCount += (levelExport.materialObjects.back().texturePath == levelExport.targetPathName);
			}
			*
			//printf("end of each iter \n");
		}
		//printf("end of object iter \n");
		std::string exportFile = levelName;
		exportFile = exportFile.substr(exportFile.find_last_of("/"));
		exportFile = exportFile.substr(exportFile.find_last_of("\\"));
		exportFile = exportFile.substr(0, exportFile.find_first_of("."));
		if (exportFile.length() == 0) {
			return;
		}

		exportFile = "TargetLevels\\" + exportFile + ".ewm";

		std::ofstream outFile(exportFile, std::ofstream::binary);
		boost::archive::binary_oarchive binary_output_archive(outFile, boost::archive::no_header);
		binary_output_archive& levelExport;
		printf("archived file to %s \n", exportFile.c_str());
		outFile.close();
	}
}
	*/
