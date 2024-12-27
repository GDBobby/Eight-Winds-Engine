#pragma once

#include "EWEngine/Graphics/Model/Model.h"
#include <memory>
#include "EWEngine/Data/TransformInclude.h"
#include "EWEngine/Data/EngineDataTypes.h"


namespace EWE {

	class EWEGameObject {
	public:
		EWEGameObject() {}
		~EWEGameObject() {
			if (model != nullptr) {
				Deconstruct(model);
			}
		}


		//static EWEGameObject makeTextBilboard(float size, std::string text, int xPos, int yPos);

		EWEGameObject(const EWEGameObject&) = delete;
		EWEGameObject& operator=(const EWEGameObject&) = delete;
		
		EWEGameObject(EWEGameObject&& other) noexcept {
			this->model = other.model;
			other.model = nullptr;
		}
		EWEGameObject& operator=(EWEGameObject&&) = default;
		

		void giveCollision() {
			collision = true;
		}
		bool hasCollision() {
			return collision;
		}
		bool isWall = false;
		bool activeTarget = true;
		bool isTarget = false;

		bool drawable = true;

		TransformComponent transform{};

		std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptor{ VK_NULL_HANDLE, VK_NULL_HANDLE };
		//int16_t textureFlags = -1;

		//optional pointer components
		EWEModel* model{nullptr};

	private:
		
		bool collision = false;



	protected:
		
	};
}