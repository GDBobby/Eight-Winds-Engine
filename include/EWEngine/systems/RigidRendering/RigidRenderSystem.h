#pragma once

#include "../../EWE_model.h"  
#include "../../Data/EngineDataTypes.h"

//this is still a WIP

namespace EWE {
    struct MaterialInfo {
        TransformComponent* ownerTransform; //if nullptr and not playerOwned, error
        EWEModel* meshPtr;
        bool* drawable;
        //Actor_Type actorType = Actor_None;

        //int32_t textureID;
        MaterialInfo() {
            printf("Default construction of material info??? \n");
            ownerTransform = nullptr;
            meshPtr = nullptr;
            //textureID = 0;
        }
        MaterialInfo(TransformComponent* tComp, EWEModel* meshP, bool* drawable) : ownerTransform{ tComp }, meshPtr{ meshP }, drawable{ drawable } {}
    };


    //singleton
    class MaterialHandler {
    public:
        static std::shared_ptr<MaterialHandler> getMaterialHandlerInstance() {
            static std::shared_ptr<MaterialHandler> materialHandlerInstance{ new MaterialHandler };
            return materialHandlerInstance;
        }
    private:
        //MaterialHandler() {}

        MaterialHandler() = default;
        //~MaterialHandler() = default;
        MaterialHandler(const MaterialHandler&) = delete;
        MaterialHandler& operator=(const MaterialHandler&) = delete;

        std::map<ShaderFlags, std::map<TextureID, std::vector<MaterialInfo>>> materialMap;
    public:
        ~MaterialHandler() {}
        const std::map<ShaderFlags, std::map<TextureID, std::vector<MaterialInfo>>>& getMaterialMap() {
            return materialMap;
        }
        const std::map<ShaderFlags, std::map<TextureID, std::vector<MaterialInfo>>>& cleanAndGetMaterialMap() {
            for (auto iter = materialMap.begin(); iter != materialMap.end();) {
                if (iter->second.size() == 0) {
                    iter = materialMap.erase(iter);
                    //clean up pipeline or leave it be? not sure how expensive it is to maintain while not in use
                }
                else {
                    iter++;
                }
            }
            return materialMap;
        }
        void addMaterialObject(ShaderFlags flags, TextureID textureID, MaterialInfo& materialInfo) {
            if (materialInfo.meshPtr == nullptr) {
                printf("NULLTPR MESH EXCEPTION \n");
                throw std::exception("nullptr mesh");
            }
            materialMap[flags][textureID].push_back(materialInfo);
        }
        void addMaterialObject(ShaderFlags flags, TransformComponent* ownerTransform, EWEModel* modelPtr, uint32_t textureID, bool* drawable) {
            if (modelPtr == nullptr) {
                printf("NULLTPR MESH EXCEPTION \n");
                throw std::exception("nullptr mesh");
            }
            materialMap[flags][textureID].emplace_back(ownerTransform, modelPtr, drawable);

        }

        void addMaterialObjectFromTexID(TextureID copyID, TransformComponent* ownerTransform) {
            for (auto iter = materialMap.begin(); iter != materialMap.end(); iter++) {
                for (auto iterTexID = iter->second.begin(); iterTexID != iter->second.end(); iterTexID++) {
                    if (iterTexID->first == copyID) {
                        if (iterTexID->second.size() == 0 || iterTexID->second[0].meshPtr == nullptr) {
							printf("NULLTPR MESH EXCEPTION or SIZE IS 0 \n");
							throw std::exception("nullptr mesh or size is 0");
						}

                        iterTexID->second.push_back(iterTexID->second[0]);
                        iterTexID->second.back().ownerTransform = ownerTransform;
                        iterTexID->second.back().meshPtr = iterTexID->second[0].meshPtr;
                        return;
                    }
                }
            }
        }

        void removeByTransform(TextureID textureID, TransformComponent* ownerTransform) {
            for (auto iter = materialMap.begin(); iter != materialMap.end(); iter++) {
                for (auto iterTexID = iter->second.begin(); iterTexID != iter->second.end(); iterTexID++) {
                    if (iterTexID->first == textureID) {
                        for (int i = 0; i < iterTexID->second.size(); i++) {
                            if (iterTexID->second[i].ownerTransform == ownerTransform) {
                                iterTexID->second.erase(iterTexID->second.begin() + i);
                                i--;
                            }
                        }
                    }
                }

            }
        }

        std::vector<TextureID> checkAndClearTextures() {
            std::vector<TextureID> returnVector;
            for (auto iter = materialMap.begin(); iter != materialMap.end(); iter++) {

                //bool removedTexID = false;
                for (auto iterTexID = iter->second.begin(); iterTexID != iter->second.end();) {
                    if (iterTexID->second.size() == 0) {
                        returnVector.push_back(iterTexID->first);
                        iterTexID = iter->second.erase(iterTexID);
                        //removedTexID = true;
                    }
                    else {
                        iterTexID++;
                    }
                }
                //if (!removedTexID) {
                //    iter++;
                //}
            }
            return returnVector;
        }
    };
}