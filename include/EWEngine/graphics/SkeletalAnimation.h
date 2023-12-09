/*
#pragma once

#include "Bone.h"
#include "Skeletal.h"

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <map>
#include <vector>
#include <iostream>
//#include <iomanip>


namespace lve {
	struct AssimpNodeData
	{
		glm::mat4 transformation;
		std::string name;
		int childrenCount;
		std::vector<AssimpNodeData> children;
	};

	class SkeletalAnimation
	{
	public:
		SkeletalAnimation() = default;

		SkeletalAnimation(const std::string& animationPath, Skeletal* skeleton);
		SkeletalAnimation(const std::string& animationPath, std::shared_ptr<Skeletal>& skeleton);
		SkeletalAnimation(const aiScene* scene, int animationIter, std::map<std::string, lve::BoneInfo>& boneInfoMap, int& boneCount);

		~SkeletalAnimation() {}

		Bone* FindBone(const std::string& name);

		inline float GetTicksPerSecond() { return m_TicksPerSecond; }
		inline float GetDuration() { return m_Duration; }
		inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
		inline const std::map<std::string, BoneInfo>& GetBoneIDMap() {return m_BoneInfoMap;}
		inline uint32_t getBoneIDMapSize() { return m_BoneInfoMap.size(); }
		inline const int& getBoneSize() { return m_Bones.size(); }



	private:
		void ReadMissingBones(const aiAnimation* animation, Skeletal& skeleton);
		void ReadMissingBones(const aiAnimation* animation, std::map<std::string, lve::BoneInfo>& boneInfoMap, int& boneCount);

		void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

		float m_Duration;
		int m_TicksPerSecond;
		std::vector<Bone> m_Bones;
		AssimpNodeData m_RootNode;
		std::map<std::string, BoneInfo> m_BoneInfoMap;
	};

	class Animator {
	public:
		Animator(SkeletalAnimation* animation);
		Animator();

		void UpdateAnimation(float dt);
		void updateAnimation(float dt, int iterator) { m_DeltaTime = dt;
			//m_CurrentTime->GetTicksPerSecond()* dt;
		}

		void PlayAnimation(SkeletalAnimation* pAnimation) {
			m_CurrentAnimation = pAnimation;
			m_CurrentTime = 0.0f;
		}
		void init(SkeletalAnimation* idleAnimation);
		void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform = glm::mat4(1.0f));

		std::vector<glm::mat4>& GetFinalBoneMatrices() {return m_FinalBoneMatrices;}

	private:
		std::vector<glm::mat4> m_FinalBoneMatrices;
		SkeletalAnimation* m_CurrentAnimation;
		float m_CurrentTime;
		float m_DeltaTime;
	};

	class SkeletonHandler {
		static std::map<std::string, const aiScene*> existingPaths;

	public:
		SkeletonHandler(LveDevice& device, std::string filePath);
		void switchAnimation(int animationIter);
		void UpdateCurrentAnimation(float dt);

		auto& GetBoneInfoMap() { return m_BoneInfoMap; }
		int& GetBoneCount() { return m_BoneCounter; }

		std::vector<std::unique_ptr<LveModel>> meshes;

		std::vector<glm::mat4>& GetFinalBoneMatrices() {return animator.GetFinalBoneMatrices();}

		uint32_t getCurrentAnimationIter() { return currentAnimationIterator; }

	private:
		std::vector<SkeletalAnimation> animations;
		uint32_t currentAnimationIterator = 0;

		uint32_t animationCount = 0;

		Animator animator;

		LveDevice& lveDevice;
		std::map<std::string, BoneInfo> m_BoneInfoMap; //
		int m_BoneCounter = 0;

		void processNode(aiNode* node, const aiScene* scene);

		std::unique_ptr<LveModel> processMesh(aiMesh* mesh, const aiScene* scene);

		void ExtractBoneWeightForVertices(std::vector<LveModel::boneVertex>& vertices, aiMesh* mesh, const aiScene* scene);

		void SetVertexBoneData(LveModel::boneVertex& vertex, int boneID, float weight);
		void SetVertexBoneDataToDefault(LveModel::boneVertex& vertex);

	};
}
*/