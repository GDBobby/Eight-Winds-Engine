#pragma once

#include "EWEngine/Graphics/Model/Model.h"
#include <memory>
#include "EWEngine/Data/TransformInclude.h"
#include "EWEngine/Data/EngineDataTypes.h"


namespace EWE {

	class EWEGameObject {
	public:
		EWEGameObject() {}
		static EWEGameObject createGameObject() {
			return EWEGameObject{};
		}

		//static EWEGameObject makeTextBilboard(float size, std::string text, int xPos, int yPos);

		/* DO NOT COPY, ONLY MOVE, IDK BOUT THESE SO I CANT FORCE THAT
		EWEGameObject(const EWEGameObject&) = delete;
		EWEGameObject& operator=(const EWEGameObject&) = delete;
		EWEGameObject(EWEGameObject&&) = default;
		EWEGameObject& operator=(EWEGameObject&&) = default;
		*/

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

		TextureID textureID{ 0 };
		//int16_t textureFlags = -1;

		//optional pointer components
		std::shared_ptr<EWEModel> model{};

	private:
		
		bool collision = false;



	protected:
		
	};
}