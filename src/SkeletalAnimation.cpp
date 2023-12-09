/*


#include "SkeletalAnimation.h"

namespace lve {


	SkeletalAnimation::SkeletalAnimation(const std::string& animationPath, Skeletal* skeleton) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);

		std::cout << "mNumAnimations : " << scene->mNumAnimations << std::endl;

		auto animation = scene->mAnimations[0];
		m_Duration = animation->mDuration;
		m_TicksPerSecond = animation->mTicksPerSecond;
		std::cout << "mDuration : " << m_Duration << "  ~   m_ticksPerSecond : " << m_TicksPerSecond << std::endl;

		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation; //where is this even used
		globalTransformation = globalTransformation.Inverse();
		ReadHeirarchyData(m_RootNode, scene->mRootNode);
		ReadMissingBones(animation, *skeleton);
	}
	SkeletalAnimation::SkeletalAnimation(const std::string& animationPath, std::shared_ptr<Skeletal>& skeleton) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);

		std::cout << "mNumAnimations : " << scene->mNumAnimations << std::endl;

		auto animation = scene->mAnimations[0];
		m_Duration = animation->mDuration;
		m_TicksPerSecond = animation->mTicksPerSecond;
		std::cout << "mDuration : " << m_Duration << "  ~   m_ticksPerSecond : " << m_TicksPerSecond << std::endl;

		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		globalTransformation = globalTransformation.Inverse();
		ReadHeirarchyData(m_RootNode, scene->mRootNode);
		ReadMissingBones(animation, *skeleton);
	}
	SkeletalAnimation::SkeletalAnimation(const aiScene* scene, int animationIter, std::map<std::string, lve::BoneInfo>& boneInfoMap, int& boneCount) {
		if (animationIter > scene->mNumAnimations) {
			std::cout << "?? animation iter too high" << std::endl;
		}
		m_Duration = scene->mAnimations[animationIter]->mDuration;
		m_TicksPerSecond = scene->mAnimations[animationIter]->mTicksPerSecond;

		ReadHeirarchyData(m_RootNode, scene->mRootNode);
		ReadMissingBones(scene->mAnimations[animationIter], boneInfoMap, boneCount);
	}
	Bone* SkeletalAnimation::FindBone(const std::string& name) {
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)	{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}
	void SkeletalAnimation::ReadMissingBones(const aiAnimation* animation, Skeletal& skeleton) {
		int size = animation->mNumChannels;
		//std::cout << "mNumChannels : " << animation->mNumChannels << std::endl;

		auto& boneInfoMap = skeleton.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		int& boneCount = skeleton.GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++) {
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].id = boneCount;
				boneCount++;
			}
			m_Bones.push_back(Bone(channel->mNodeName.data,
				boneInfoMap[channel->mNodeName.data].id, channel));
		}

		m_BoneInfoMap = boneInfoMap;
	}
	void SkeletalAnimation::ReadMissingBones(const aiAnimation* animation, std::map<std::string, lve::BoneInfo>& boneInfoMap, int& boneCount) {
		int size = animation->mNumChannels;
		//std::cout << "mNumChannels : " << animation->mNumChannels << std::endl;

		//auto& boneInfoMap = skeleton.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		//int& boneCount = skeleton.GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++) {
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].id = boneCount;
				boneCount++;
			}
			m_Bones.push_back(Bone(channel->mNodeName.data,
				boneInfoMap[channel->mNodeName.data].id, channel));
		}

		m_BoneInfoMap = boneInfoMap;
	}
	void SkeletalAnimation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src) {
		assert(src);

		dest.name = src->mName.data;
		//dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.transformation[0][0] = src->mTransformation.a1; dest.transformation[1][0] = src->mTransformation.a2; dest.transformation[2][0] = src->mTransformation.a3; dest.transformation[3][0] = src->mTransformation.a4;
		dest.transformation[0][1] = src->mTransformation.b1; dest.transformation[1][1] = src->mTransformation.b2; dest.transformation[2][1] = src->mTransformation.b3; dest.transformation[3][1] = src->mTransformation.b4;
		dest.transformation[0][2] = src->mTransformation.c1; dest.transformation[1][2] = src->mTransformation.c2; dest.transformation[2][2] = src->mTransformation.c3; dest.transformation[3][2] = src->mTransformation.c4;
		dest.transformation[0][3] = src->mTransformation.d1; dest.transformation[1][3] = src->mTransformation.d2; dest.transformation[2][3] = src->mTransformation.d3; dest.transformation[3][3] = src->mTransformation.d4;
		dest.childrenCount = src->mNumChildren;

		for (int i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHeirarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}




	//animator
	Animator::Animator(SkeletalAnimation* animation) {
		m_CurrentTime = 0.0;
		m_CurrentAnimation = animation;
		//m_FinalBoneMatrices.resize(100, glm::mat4(1.0f)); ?? why not
		uint32_t boneSize = animation->getBoneSize();
		std::cout << "bone id map size : " << boneSize << std::endl;

		m_FinalBoneMatrices.reserve(boneSize);
		for (int i = 0; i < boneSize; i++) { m_FinalBoneMatrices.push_back(glm::mat4(1.0f)); }
	}
	Animator::Animator() {
		m_CurrentTime = 0.0;
		m_CurrentAnimation = nullptr;
	}
	void Animator::init(SkeletalAnimation* idleAnimation) {
		m_CurrentTime = 0.0;
		m_CurrentAnimation = idleAnimation;

		uint32_t boneSize = idleAnimation->getBoneSize();
		m_FinalBoneMatrices.reserve(boneSize);
		for (int i = 0; i < boneSize; i++) { m_FinalBoneMatrices.push_back(glm::mat4(1.0f)); }
	}
	void Animator::UpdateAnimation(float dt) {
		m_DeltaTime = dt;
		//std::cout << "inside update animation " << std::endl;
		if (m_CurrentAnimation) {
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			//std::cout << "pre updateanimation calc bone transform" << std::endl;
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode());
			//std::cout << "post updateanimation calc bone transform" << std::endl;
		}
	}
	void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransforM) {
		//std::cout << "inside calculate bone transform" << std::endl;
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone) {
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->childrenCount; i++) { CalculateBoneTransform(&node->children[i], globalTransformation); }

		//std::cout << "end of calculate bone transform" << std::endl;
	}



	///skeleton handler
	std::map<std::string, const aiScene*> SkeletonHandler::existingPaths;

	SkeletonHandler::SkeletonHandler(LveDevice& device, std::string filePath) : lveDevice{ device } {

		//aiScene* scene; need const to improt readfile
		if (existingPaths.find(filePath) == existingPaths.end()) {
			Assimp::Importer import;
			existingPaths[filePath] = import.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);
			if (!existingPaths[filePath] || existingPaths[filePath]->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !existingPaths[filePath]->mRootNode)
			{
				std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
				return;
			}
			processNode(existingPaths[filePath]->mRootNode, existingPaths[filePath]);

			//reimplement skeletalAnimation
			animationCount = existingPaths[filePath]->mNumAnimations;
			for (int i = 0; i < animationCount; i++) {
				//scene->mAnimations[i]->mName
				animations.push_back(SkeletalAnimation(existingPaths[filePath], i, m_BoneInfoMap, m_BoneCounter));
				std::cout << "duration " << i << " ? : " << animations.back().GetDuration() << std::endl;
			}
			currentAnimationIterator = 0;
			animator.init(&animations[0]);
		}
		else {
			//lets just do a hard copy
			animator.init(&animations[0]);
		}
	}
	void SkeletonHandler::switchAnimation(int animationIter) {
		if ((animationIter >= animationCount)) {
			//std::cout << "switch animation iter too high, doing a modulus temporarily" << std::endl;
			if (animationCount == 0) { printf("0animation count return \n"); return; }

			animationIter %= animationCount;
		}
		currentAnimationIterator = animationIter;
		animator.PlayAnimation(&animations[animationIter]);
		printf("animation iter from switch %d \n", animationIter);
	}
	void SkeletonHandler::UpdateCurrentAnimation(float dt) {
		animator.UpdateAnimation(dt);
	}
	void SkeletonHandler::processNode(aiNode* node, const aiScene* scene) {
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	std::unique_ptr<LveModel> SkeletonHandler::processMesh(aiMesh* mesh, const aiScene* scene) {
		// data to fill
		std::vector<LveModel::boneVertex> vertices;
		std::vector<uint32_t> indices;
		//std::vector<Texture> textures;

		// walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			LveModel::boneVertex vertex;
			SetVertexBoneDataToDefault(vertex);

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
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		// return a mesh object created from the extracted mesh data
		ExtractBoneWeightForVertices(vertices, mesh, scene);

		return LveModel::createMesh(lveDevice, vertices, indices);
	}

	void SkeletonHandler::ExtractBoneWeightForVertices(std::vector<LveModel::boneVertex>& vertices, aiMesh* mesh, const aiScene* scene) {
		for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			int boneID = -1;
			std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
			if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
			{
				BoneInfo newBoneInfo;
				newBoneInfo.id = m_BoneCounter;
				//newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
				newBoneInfo.offset[0][0] = mesh->mBones[boneIndex]->mOffsetMatrix.a1; newBoneInfo.offset[1][0] = mesh->mBones[boneIndex]->mOffsetMatrix.a2; newBoneInfo.offset[2][0] = mesh->mBones[boneIndex]->mOffsetMatrix.a3; newBoneInfo.offset[3][0] = mesh->mBones[boneIndex]->mOffsetMatrix.a4;
				newBoneInfo.offset[0][1] = mesh->mBones[boneIndex]->mOffsetMatrix.b1; newBoneInfo.offset[1][1] = mesh->mBones[boneIndex]->mOffsetMatrix.b2; newBoneInfo.offset[2][1] = mesh->mBones[boneIndex]->mOffsetMatrix.b3; newBoneInfo.offset[3][1] = mesh->mBones[boneIndex]->mOffsetMatrix.b4;
				newBoneInfo.offset[0][2] = mesh->mBones[boneIndex]->mOffsetMatrix.c1; newBoneInfo.offset[1][2] = mesh->mBones[boneIndex]->mOffsetMatrix.c2; newBoneInfo.offset[2][2] = mesh->mBones[boneIndex]->mOffsetMatrix.c3; newBoneInfo.offset[3][2] = mesh->mBones[boneIndex]->mOffsetMatrix.c4;
				newBoneInfo.offset[0][3] = mesh->mBones[boneIndex]->mOffsetMatrix.d1; newBoneInfo.offset[1][3] = mesh->mBones[boneIndex]->mOffsetMatrix.d2; newBoneInfo.offset[2][3] = mesh->mBones[boneIndex]->mOffsetMatrix.d3; newBoneInfo.offset[3][3] = mesh->mBones[boneIndex]->mOffsetMatrix.d4;

				m_BoneInfoMap[boneName] = newBoneInfo;
				boneID = m_BoneCounter;
				m_BoneCounter++;
			}
			else
			{
				boneID = m_BoneInfoMap[boneName].id;
			}
			assert(boneID != -1);
			auto weights = mesh->mBones[boneIndex]->mWeights;
			int numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				assert(vertexId <= vertices.size());
				SetVertexBoneData(vertices[vertexId], boneID, weight);
			}
		}
	}

	void SkeletonHandler::SetVertexBoneData(LveModel::boneVertex& vertex, int boneID, float weight) {
		for (int i = 0; i < MAX_BONE_WEIGHTS; ++i)
		{
			if (vertex.m_BoneIDs[i] < 0) {
				vertex.m_Weights[i] = weight;
				vertex.m_BoneIDs[i] = boneID;
				break;
			}
		}
	}

	void SkeletonHandler::SetVertexBoneDataToDefault(LveModel::boneVertex& vertex) {
		for (int i = 0; i < MAX_BONE_WEIGHTS; i++) {
			vertex.m_BoneIDs[i] = -1;
			vertex.m_Weights[i] = 0.0f;
		}
	}

}
*/