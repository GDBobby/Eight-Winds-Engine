#include "EWEngine/Collision.h"

#include <glm/gtc/matrix_transform.hpp>


namespace EWE {
	std::vector<TransformComponent*> floors;
	std::vector<TransformComponent*> walls;

	collisionReturn Collision::checkForGround(std::array<float, 3>& translationA, float attemptedVerticalMovement, float radius) { //return the y coordinate for the ground checked, if found
		//std::cout << "beginning of checkforground: " << std::endl;
		//running cylinder, quad right now
		//get the bottom of the cylinder, for the player the bottom is going to be the transform. right now im going to focus collisions including the player
		//std::cout << "starting check for ground" << std::endl;

		//printf("collisionObjects.size(): %d \n", collisionObjects.size());
		for (auto& transform : floors) {

			if (((translationA[0] + radius) > (transform->translation.x - (transform->scale.x / 2.f))) &&
				((translationA[0] - radius) < (transform->translation.x + (transform->scale.x / 2.f))) &&
				((translationA[2] + radius) > (transform->translation.z - (transform->scale.z / 2.f))) &&
				((translationA[2] - radius) < (transform->translation.z + (transform->scale.z / 2.f))) &&
				(translationA[1] > transform->translation.y) && 
				((translationA[1] + attemptedVerticalMovement) < (transform->translation.y))
				) {
				//std::cout << "within X and Z, moving down thru the object" << std::endl;
				//std::cout << "end of checkforground: " << std::endl;
				return { true, transform->translation.y };


			}
			
			
		}
		//std::cout << "ending check for ground" << std::endl;
		//std::cout << "end of checkforground: " << std::endl;
		return collisionReturn{};
	}

	bool Collision::checkIfStillGrounded(std::array<float, 3>& translationA, float radius) {
		collisionReturn ret = checkForGround(translationA, 0.f, radius);
		return ret.check == false && ret.checkLocation == -69.f;
	}

	bool Collision::checkForWallCollision(std::array<float, 3>& translationA, glm::vec3 intendedMovement, float radius, float height) {
		
		for (auto& transform : walls) {
			if (((translationA[0] + radius) > (transform->translation.x - (transform->scale.x))) &&
				((translationA[0] - radius) < (transform->translation.x + (transform->scale.x))) &&
				((translationA[2] + radius) > (transform->translation.z - (transform->scale.z))) &&
				((translationA[2] - radius) < (transform->translation.z + (transform->scale.z)))// &&
				//(translationa[1] == transform->translation.y)
				) {
				//std::cout << "in bounds, grounded? ~ " <<  translationa[1] << ":" << transform->translation.y << std::endl;
				return true;


			}
		}
		return false;
	}

	void Collision::AddFloor(TransformComponent& transform) {
#if EWE_DEBUG
		for (auto& floor : floors) {
			assert(floor != &transform);
		}
#endif
		floors.push_back(&transform);
	}
	void Collision::AddWall(TransformComponent& transform) {
#if EWE_DEBUG
		for (auto& wall : walls) {
			assert(wall != &transform);
		}
#endif
		walls.push_back(&transform);
	}
	void Collision::RemoveCollider(TransformComponent& transform) {
		TransformComponent* check = &transform;
		for (uint16_t i = 0; i < floors.size(); i++) {
			if (floors[i] == check) {
				floors.erase(floors.begin() + i);
			}
		}
		for (uint16_t i = 0; i < walls.size(); i++) {
			if (walls[i] == check) {
				walls.erase(walls.begin() + i);
			}
		}
	}
	void Collision::ClearCollision() {
		floors.clear();
		walls.clear();
	}
}