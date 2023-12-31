#include "EWEngine/graphics/EWE_Object.h"
#include "EWEngine/systems/SkinRendering/SkinRenderSystem.h"

namespace EWE {
    EweObject::EweObject(std::string objectPath, EWEDevice& device, bool globalTextures) {

        ImportData tempData = ImportData::loadData(objectPath);
        TextureMapping textureTracker;
        loadTextures(device, objectPath, tempData.nameExport, textureTracker, globalTextures);

        addToMaterialHandler(device, tempData, textureTracker);
    }
    EweObject::EweObject(std::string objectPath, EWEDevice& device, bool globalTextures, SkeletonID ownerID) : mySkinID{ SkinRenderSystem::getSkinID() } {
        std::cout << "weapon object construction : objectPath - " << objectPath << std::endl;
        ImportData tempData = ImportData::loadData(objectPath);
        TextureMapping textureTracker;
        loadTextures(device, objectPath, tempData.nameExport, textureTracker, globalTextures);

        //addToMaterialHandler(device, tempData, textureTracker);
        addToSkinHandler(device, tempData, textureTracker, ownerID);
    }
    EweObject::~EweObject() {
        std::shared_ptr<MaterialHandler> materialInstance = MaterialHandler::getMaterialHandlerInstance();

        ownedTextureIDs.sort(); //dgaf bout sorting but need it for std::list::unique()
        ownedTextureIDs.unique();
        //printf("before removing textures \n");
        for (auto iter = ownedTextureIDs.begin(); iter != ownedTextureIDs.end(); iter++) {
            materialInstance->removeByTransform(*iter, &transform);
        }
        //printf("after removing textures \n");
    }
    void EweObject::addToMaterialHandler(EWEDevice& device, ImportData& tempData, TextureMapping& textureTracker) {

        std::shared_ptr<MaterialHandler> materialInstance = MaterialHandler::getMaterialHandlerInstance();

        //Actor_Type actorType = (Actor_Type)(1 + isKatana);
        /*
        for (int i = 0; i < tempData.meshExport.meshes.size(); i++) {
            meshes.push_back(EWEModel::createMesh(device, tempData.meshExport.meshes[i].first, tempData.meshExport.meshes[i].second));
            //printf("meshes flag : %d \n", textureTracker.meshNames[i].first + 128);
            materialInstance->addMaterialObject(textureTracker.meshNames[i].first + 128, actorType, nullptr, meshes.back().get(), textureTracker.meshNames[i].second, &drawable);
        }
        for (int i = 0; i < tempData.meshNTExport.meshesNT.size(); i++) {
            meshes.push_back(EWEModel::createMesh(device, tempData.meshNTExport.meshesNT[i].first, tempData.meshNTExport.meshesNT[i].second));
            //printf("meshesNT flag : %d \n", textureTracker.meshNTNames[i].first + 128);
            materialInstance->addMaterialObject(textureTracker.meshNTNames[i].first + 128, actorType, nullptr, meshes.back().get(), textureTracker.meshNTNames[i].second, &drawable);
        }
        */

        for (int i = 0; i < tempData.meshSimpleExport.meshesSimple.size(); i++) {
            meshes.push_back(EWEModel::createMesh(device, tempData.meshSimpleExport.meshesSimple[i].first, tempData.meshSimpleExport.meshesSimple[i].second));
            materialInstance->addMaterialObject(textureTracker.meshSimpleNames[i].first, &transform, meshes.back().get(), textureTracker.meshSimpleNames[i].second, &drawable);
        }
        for (int i = 0; i < tempData.meshNTSimpleExport.meshesNTSimple.size(); i++) {
            meshes.push_back(EWEModel::createMesh(device, tempData.meshNTSimpleExport.meshesNTSimple[i].first, tempData.meshNTSimpleExport.meshesNTSimple[i].second));
            materialInstance->addMaterialObject(textureTracker.meshNTSimpleNames[i].first, &transform, meshes.back().get(), textureTracker.meshNTSimpleNames[i].second, &drawable);
        }
        /*
        for (int i = 0; i < tempData.meshExport.meshes.size(); i++) {
            printf("mesh first : %d \n", textureTracker.meshNames[i].first + 128);
        }
        for (int i = 0; i < tempData.meshNTExport.meshesNT.size(); i++) {
            printf("meshNT first : %d \n", textureTracker.meshNTNames[i].first + 128);
        }
        for (int i = 0; i < tempData.meshSimpleExport.meshesSimple.size(); i++) {
            printf("meshSimple first : %d \n", textureTracker.meshSimpleNames[i].first);
        }
        for (int i = 0; i < tempData.meshNTSimpleExport.meshesNTSimple.size(); i++) {
            printf("meshNTSimple first : %d \n", textureTracker.meshNTSimpleNames[i].first);
        }
        */
        size_t nameSum = tempData.meshExport.meshes.size() + tempData.meshNTExport.meshesNT.size() + tempData.meshSimpleExport.meshesSimple.size() + tempData.meshNTSimpleExport.meshesNTSimple.size();

        if (nameSum != meshes.size()) {
            std::cout << std::format("mesh to name mismatch - {}:{} \n", nameSum, meshes.size());
            throw std::runtime_error("failed to match mesh to name");
        }
    }

    void EweObject::addToSkinHandler(EWEDevice& device, ImportData& tempData, TextureMapping& textureTracker, SkeletonID skeletonOwner) {
        if ((tempData.meshNTSimpleExport.meshesNTSimple.size() > 0) || (tempData.meshSimpleExport.meshesSimple.size() > 0)) {
            printf("weapon can not have simple meshes \n");
            throw std::exception("object can not have both simple meshes");
        }

        /*
                for (int i = 0; i < tempData.meshSimpleExport.meshesSimple.size(); i++) {
            meshes.push_back(EWEModel::createMesh(device, tempData.meshSimpleExport.meshesSimple[i].first, tempData.meshSimpleExport.meshesSimple[i].second));
            materialInstance->addMaterialObject(textureTracker.meshSimpleNames[i].first, &transform, meshes.back().get(), textureTracker.meshSimpleNames[i].second, &drawable);
        }
        for (int i = 0; i < tempData.meshNTSimpleExport.meshesNTSimple.size(); i++) {
            meshes.push_back(EWEModel::createMesh(device, tempData.meshNTSimpleExport.meshesNTSimple[i].first, tempData.meshNTSimpleExport.meshesNTSimple[i].second));
            materialInstance->addMaterialObject(textureTracker.meshNTSimpleNames[i].first, &transform, meshes.back().get(), textureTracker.meshNTSimpleNames[i].second, &drawable);
        */

        if (tempData.meshExport.meshes.size() > 0) {
            meshes.reserve(tempData.meshExport.meshes.size());
            for (uint16_t i = 0; i < tempData.meshExport.meshes.size(); i++) {
                meshes.push_back(EWEModel::createMesh(device, tempData.meshExport.meshes[i].first, tempData.meshExport.meshes[i].second));
                SkinRenderSystem::addWeapon(textureTracker.meshNames[i], meshes[i].get(), mySkinID, skeletonOwner);
            }
        }
        else if (tempData.meshNTExport.meshesNT.size() > 0) {
            meshes.reserve(tempData.meshNTExport.meshesNT.size());
            for (uint16_t i = 0; i < tempData.meshNTExport.meshesNT.size(); i++) {
                meshes.push_back(EWEModel::createMesh(device, tempData.meshNTExport.meshesNT[i].first, tempData.meshNTExport.meshesNT[i].second));
                SkinRenderSystem::addWeapon(textureTracker.meshNTNames[i], meshes[i].get(), mySkinID, skeletonOwner);
            }
        }
        else {
            std::cout << "invalid weapon type \n";
            throw std::exception("invalid weapon type?");
        }
       
    }

    void EweObject::loadTextures(EWEDevice& device, std::string objectPath, ImportData::NameExportData& importData, TextureMapping& textureTracker, bool globalTextures) {
        //TEXTURES
        //this should be put in a separate function but im too lazy rn
        //printf("before loading ewe textures \n");

        std::pair<ShaderFlags, TextureID> returnPair;
        for (int i = 0; i < importData.meshNames.size(); i++) {
            importData.meshNames[i] = importData.meshNames[i].substr(0, importData.meshNames[i].find_first_of("."));
            if (importData.meshNames[i].find("lethear") != importData.meshNames[i].npos) {
                for (int j = 0; j < importData.meshNames.size(); j++) {
                    if (j == i) { continue; }
                    if (importData.meshNames[j].find("leather") != importData.meshNames[j].npos) {
                        if (j >= i) {
                            printf("leather farther back than lethear 2 ?  \n");
                        }
                        else {
                            returnPair = textureTracker.meshNames[j];
                            if (returnPair.first <= 0) {
                                printf("return pair error? : %d \n", returnPair.first);
                            }
                            textureTracker.meshNames.push_back(returnPair);
                            ownedTextureIDs.push_back(returnPair.second);
                            break;
                        }
                    }
                }
                continue;
            }
            std::string finalDir = objectPath;
            finalDir += "\\" + importData.meshNames[i];
            if (globalTextures) {
                returnPair = EWETexture::addSmartGlobalTexture(device, finalDir, EWETexture::tType_smart);
            }
            else {
                returnPair = EWETexture::addSmartSceneTexture(device, finalDir, EWETexture::tType_smart);
            }
            //printf("normal map texture? - return pair.first, &8 - %d;%d \n", returnPair.first, returnPair.first & 8);
            if (returnPair.first < 0) {
                printf("FAILED TO FIND PIPE, NEED TO THROW AN ERROR : %d \n", returnPair.first);
            }
            else {
                textureTracker.meshNames.push_back(returnPair);
                ownedTextureIDs.push_back(returnPair.second);
            }
        }
        //printf("after mesh texutres \n");
        for (int i = 0; i < importData.meshNTNames.size(); i++) {
            importData.meshNTNames[i] = importData.meshNTNames[i].substr(0, importData.meshNTNames[i].find_first_of("."));
            std::string finalDir = objectPath;
            finalDir += "\\" + importData.meshNTNames[i];
            if (globalTextures) {
                returnPair = EWETexture::addSmartGlobalTexture(device, finalDir, EWETexture::tType_smart);
            }
            else {
                returnPair = EWETexture::addSmartSceneTexture(device, finalDir, EWETexture::tType_smart);
            }
            //printf("no normal map texture? - return pair.first, &8 - %d;%d \n", returnPair.first, returnPair.first & 8);
            if (returnPair.first < 0) {
                printf("FAILED TO FIND PIPE, NEED TO THROW AN ERROR : %d \n", returnPair.first);
            }
            else {
                textureTracker.meshNTNames.push_back(returnPair);
                ownedTextureIDs.push_back(returnPair.second);
            }
        }
        //printf("after mesh nt texutres \n");


        for (int i = 0; i < importData.meshSimpleNames.size(); i++) {
            importData.meshSimpleNames[i] = importData.meshSimpleNames[i].substr(0, importData.meshSimpleNames[i].find_first_of("."));
            std::string finalDir = objectPath;
            finalDir += "\\" + importData.meshSimpleNames[i];
            //printf("simple names final Dir : %s \n", finalDir.c_str());
            if (globalTextures) {
                returnPair = EWETexture::addSmartGlobalTexture(device, finalDir, EWETexture::tType_smart);
            }
            else {
                returnPair = EWETexture::addSmartSceneTexture(device, finalDir, EWETexture::tType_smart);
            }
            //printf("no normal map texture? - return pair.first, &8 - %d;%d \n", returnPair.first, returnPair.first & 8);
            if (returnPair.first < 0) {
                printf("FAILED TO FIND PIPE, NEED TO THROW AN ERROR : %d \n", returnPair.first);
            }
            else {
                textureTracker.meshSimpleNames.push_back(returnPair);
                ownedTextureIDs.push_back(returnPair.second);
            }
        }

        for (int i = 0; i < importData.meshNTSimpleNames.size(); i++) {
            importData.meshNTSimpleNames[i] = importData.meshNTSimpleNames[i].substr(0, importData.meshNTSimpleNames[i].find_first_of("."));
            std::string finalDir = objectPath;
            finalDir += "\\" + importData.meshNTSimpleNames[i];
            if (globalTextures) {
                returnPair = EWETexture::addSmartGlobalTexture(device, finalDir, EWETexture::tType_smart);
            }
            else {
                returnPair = EWETexture::addSmartSceneTexture(device, finalDir, EWETexture::tType_smart);
            }
            //printf("no normal map texture? - return pair.first, &8 - %d;%d \n", returnPair.first, returnPair.first & 8);
            if (returnPair.first < 0) {
                printf("FAILED TO FIND PIPE, NEED TO THROW AN ERROR : %d \n", returnPair.first);
            }
            else {
                textureTracker.meshNTSimpleNames.push_back(returnPair);
                ownedTextureIDs.push_back(returnPair.second);
            }
        }
    }


}