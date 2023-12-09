/*
#include "AssimpModel.h"
#include <iostream>
#include <assimp/postprocess.h>

namespace EWE {
    //std::map<int16_t, std::vector<AssimpModel::MapInfo>> AssimpModel::materialMeshes;

    void AssimpModel::loadModel(std::string path) {
        Assimp::Importer import;
        printf("pre-scene \n");
        const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        printf("after scene \n");
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }

        processNode(scene->mRootNode, scene); //might remove this and just replace it with meshes
        printf("model thread finished : %s \n", directory.c_str());
    }
    *
    AssimpModel::~AssimpModel() {
        for (int i = textures.size() - 1; i >= 0; i--) {
            textures[i].destroy();
            textures.pop_back();
        }
    }
    *

    void AssimpModel::processNode(aiNode* node, const aiScene* scene) {
        // process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            printf("processing node : %d \n", i);
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            aiMaterial* material;
            if (mesh->mMaterialIndex < scene->mNumMaterials) {
               material = scene->mMaterials[mesh->mMaterialIndex];
            }
            if (material != nullptr) {
                //get if material has normal
                *
                if(hasNormal){
                    //load mesh
                }
                else{
                    //load mesh NT
                }
                *
                //materialMeshes[pipelineFlags[materialMeshCount]].push_back({this, materialMeshCount });
                //materialMeshCount++;
            }
            meshes.push_back(processMesh(mesh, scene));
            *
            else {
                if (pipelineFlags[materialMeshCount] != -1) {
                    materialMeshes[pipelineFlags[materialMeshCount]].push_back(processMesh(mesh, scene));
                }
                else {
                    printf("-1 pipeline flag, material mesh : %d \n", materialMeshCount);
                }
                materialMeshCount++;
            }
            *
            if (directory == "SWANGAcaddy") {
                printf("mesh name : %s \n", mesh->mName.C_Str());
            }
            else if (directory == "ellen") {
                printf("mesh name : %s \n", mesh->mName.C_Str());
            }
        }
        // then do the same for each of its children
        printf("node count? :%d \n", node->mNumChildren);
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            printf("processing node children : %d \n", i);
            processNode(node->mChildren[i], scene);
        }
    }

    std::unique_ptr<EWEModel> AssimpModel::processMesh(aiMesh* mesh, const aiScene* scene) {
        // data to fill
        std::vector<EWEModel::AVertex> vertices;
        std::vector<uint32_t> indices;
        //std::vector<Texture> textures;
        //printf("mesh name : %s \n", mesh->mName.C_Str());
        // walk through each of the mesh's vertices
        printf("processing mesh, vertex count : %d \n", mesh->mNumVertices);
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            EWEModel::AVertex vertex;
            //SetVertexBoneDataToDefault(vertex);
            
            vertex.position.x = mesh->mVertices[i].x;// / 100.f;
            vertex.position.y = mesh->mVertices[i].y;// / 100.f;
            vertex.position.z = mesh->mVertices[i].z;// / 100.f;
            
            //vertex.normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
            
            if (mesh->HasNormals())
            {
                vertex.normal.x = mesh->mNormals[i].x;
                vertex.normal.y = mesh->mNormals[i].y;
                vertex.normal.z = mesh->mNormals[i].z;
            }
            else {
                std::cout << "why no normals" << std::endl;
            }
            if (mesh->HasTangentsAndBitangents()) {
                vertex.tangent.x = mesh->mTangents[i].x;
                vertex.tangent.y = mesh->mTangents[i].y;
                vertex.tangent.z = mesh->mTangents[i].z;
            }
            else {
                //printf("mesh doesnt have tangents \n");
            }

            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vertex.uv.x = mesh->mTextureCoords[0][i].x;
                vertex.uv.y = mesh->mTextureCoords[0][i].y;
            }
            else {
                vertex.uv = glm::vec2(0.0f, 0.0f);
            }
            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        if (mesh->mNumFaces != (mesh->mNumVertices / 3)) {
            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                aiFace face = mesh->mFaces[i];
                // retrieve all indices of the face and store them in the indices vector
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    indices.push_back(face.mIndices[j]);
                }
            }
        }
        else {
            printf("faces is equal to vertices / 3 \n");
        }
        // process materials
        *
        if (scene->HasMaterials()) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            printf("\n");
            printf("material diffuse count : %d \n", material->GetTextureCount(aiTextureType_DIFFUSE));
            printf("material specular count : %d \n", material->GetTextureCount(aiTextureType_SPECULAR));
            printf("material height count : %d \n", material->GetTextureCount(aiTextureType_HEIGHT));
            printf("material ambient count : %d \n", material->GetTextureCount(aiTextureType_AMBIENT));
            printf("\n");
        }
        *
        
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        
        //std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        //std::textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        //std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        //textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        //std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        //textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        //std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        //textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        

        // return a mesh object created from the extracted mesh data
        //ExtractBoneWeightForVertices(ertices, mesh, scene);
        //printf("mesh vertices, indices -  %d:%d \n", vertices.size(), indices.size());
        if (indices.size() != 0) {
            return EWEModel::createMesh(eweDevice, vertices, indices);
        }
        else {
            return EWEModel::createMesh(eweDevice, vertices);
        }
    }
    void AssimpModel::loadMatTexture(aiMaterial* mat, aiTextureType type, std::string typeName) {
        * once we get into more than 1 texture type, aka normals
        * have this function return a vector of textures per type
        * after collecting all texture types, put them in 1 big texture, 
        *

        uint32_t textureCount = mat->GetTextureCount(type);
        for (int i = 0; i < textureCount; i++) {
            aiString str;
            std::string tempString;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (uint32_t j = 0; j < textureIDs.size(); j++) {
                //?
            }
            tempString = str.C_Str();
            printf("texture string? %s \n", tempString.c_str());

        }
    }

    void AssimpModel::loadArenaTextures() {

        uint32_t meshCount = meshes.size();
        //printf("meshes size : %d \n", meshes.size());
        //meshes[0].
        textureIDs.reserve(meshCount);
        std::string textureDirectory = directory;
       // textureDirectory += directory;
        textureDirectory += "/";

        std::string individualTextureDirs[7] = {
            "planks.png",
            "granite.png",
            "sand.png",
            "sandstone.png",
            "wood.png",
            "grate.png",
            "weird.png"
        };
        for (int i = 0; i < 7; i++) {
            std::string finalDir = textureDirectory; 
            finalDir += individualTextureDirs[i];
            textureIDs.emplace_back(EWETexture::addModeTexture(eweDevice, finalDir, EWETexture::tType_material));
        }
    }
    void AssimpModel::loadKatanaTexture() {
        uint32_t meshCount = meshes.size();
        textureIDs.reserve(meshCount);
        std::string textureDirectory = "Katana/katana.png";
        textureIDs.emplace_back(EWETexture::addGlobalTexture(eweDevice, textureDirectory, EWETexture::tType_material));
    }
    void AssimpModel::loadRoadCrossTexture() {
        //textureIDs.reserve(meshes.size());
        std::string textureDirectory = "concrete.jpg";
        int32_t texID = EWETexture::addModeTexture(eweDevice, textureDirectory, EWETexture::tType_material);
        textureIDs.resize(meshes.size(), texID);
        transform.scale = glm::vec3{ 1.f, -1.f, 1.f };
    }
    void AssimpModel::loadCaddyTexture() {
        printf("loading caddy textures \n");
        textureIDs.reserve(8);
        std::string textureDirectory = "caddy/";

        std::string individualTextureDirs[8] = {
            "grill",
            "bumper_front",
            "body",
            "bumper_rear",
            "hood",
            "skirts",
            "spoiler",
            "wheel",
        };
        enum meshNames {
            grill,
            bumper_front,
            body,
            bumper_rear,
            hood,
            skirts,
            spoiler,
            wheel,
        };

        std::vector<uint32_t> tempIDs;
        for (int i = 0; i < 8; i++) {
            std::string finalDir = textureDirectory;
            finalDir += individualTextureDirs[i];
            finalDir += ".png";
            tempIDs.emplace_back(EWETexture::addModeTexture(eweDevice, finalDir, EWETexture::tType_metalRough));
        }

        //sorting the IDs
        for (int i = 0; i < 8; i++) {
            textureIDs.push_back(tempIDs[body]);
        }
        for (int i = 0; i < 5; i++) {
            textureIDs.push_back(tempIDs[bumper_rear]);
        }
        textureIDs.push_back(tempIDs[hood]);

        textureIDs.push_back(tempIDs[skirts]);
        textureIDs.push_back(tempIDs[skirts]);
        textureIDs.push_back(tempIDs[skirts]);

        textureIDs.push_back(tempIDs[spoiler]);

        textureIDs.push_back(tempIDs[wheel]);
        textureIDs.push_back(tempIDs[wheel]);
        textureIDs.push_back(tempIDs[wheel]);
        textureIDs.push_back(tempIDs[wheel]);

        textureIDs.push_back(tempIDs[grill]);

        for (int i = 0; i < 8; i++) {
            textureIDs.push_back(tempIDs[bumper_front]);
        }



        printf("texutreID count in caddy : %d \n", textureIDs.size());
        for (int i = 0; i < textureIDs.size(); i++) {
			//printf("textureID : %d \n", textureIDs[i]);
		}
    }
    std::vector<int16_t> AssimpModel::loadEllenTextures() {
        std::vector<std::string> individualTextureDirs = {
            "Std_Eye_R",
            "Std_Cornea_R",
            "Std_Eye_L",
            "Std_Cornea_L",

            "Silk_Charmeuse_FRONT_2375",//"__0",
            //"High_Heels",

            "Std_Skin_Head",
            "Std_Skin_Body",
            "Std_Skin_Arm",
            "Std_Skin_Leg",
            "Std_Nails",
            "Std_Eyelash",
        };

        std::vector<int16_t> pipelineFlags;
        for (int i = 0; i < individualTextureDirs.size(); i++) {
            std::string finalDir = "female/";
            finalDir += individualTextureDirs[i];
            std::pair<int16_t, int32_t> returnPair = EWETexture::addSmartModeTexture(eweDevice, finalDir, EWETexture::tType_smart);
            printf("ellen texture - return pair - %d:%d \n", returnPair.first, returnPair.second);
            textureIDs.emplace_back(returnPair.second);
            pipelineFlags.push_back(returnPair.first);
            
        }
        return pipelineFlags;
    }

    void AssimpModel::loadObstacleTextures() {
        *
        uint32_t meshCount = meshes.size();
        printf("meshes size : %d \n", meshes.size());
        //meshes[0].
        textureIDs.reserve(meshCount);
        std::string textureDirectory = directory;
        //textureDirectory += directory;
        textureDirectory += "/";

        std::string individualTexture = textureDirectory;
        individualTexture += "hazard";

        textureIDs.emplace_back(EWETexture::addMaterialTexture(eweDevice, individualTexture, ".png")); //main

        individualTexture = textureDirectory + "rockTile";
        textureIDs.emplace_back(EWETexture::addMaterialTexture(eweDevice, individualTexture, ".png")); //bleachers
        *
    }
}
*/