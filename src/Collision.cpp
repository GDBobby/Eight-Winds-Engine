#include "EWEngine/Collision.h"

#include <glm/gtc/matrix_transform.hpp>


namespace EWE {
	std::vector<std::vector<EWEGameObject>*> Collision::collisionObjects;

	collisionReturn Collision::checkForGround(std::array<float, 3>& translationA, float attemptedVerticalMovement, float radius) { //return the y coordinate for the ground checked, if found
		//std::cout << "beginning of checkforground: " << std::endl;
		//running cylinder, quad right now
		//get the bottom of the cylinder, for the player the bottom is going to be the transform. right now im going to focus collisions including the player
		//std::cout << "starting check for ground" << std::endl;

		//printf("collisionObjects.size(): %d \n", collisionObjects.size());
		for (int i = 0; i < collisionObjects.size(); i++) {
			//printf("collisionObjects[i].size(): %d \n", collisionObjects[i]->size());
			//std::cout << "inside? : " << i << std::endl;
			//currently only texturedGameObjects will be used for collision, eventually ill use textured as well, and texturedGameObjects will be invisible collision only
			//avoid placing collideables above/below each other.

			//as is, its being treated like a cube and not a cylinder
			for (int j = 0; j < collisionObjects[i]->size(); j++) {
				if (collisionObjects[i]->at(j).hasCollision()) {

					if (((translationA[0] + radius) > (collisionObjects[i]->at(j).transform.translation.x - (collisionObjects[i]->at(j).transform.scale.x / 2.f))) &&
						((translationA[0] - radius) < (collisionObjects[i]->at(j).transform.translation.x + (collisionObjects[i]->at(j).transform.scale.x / 2.f))) &&
						((translationA[2] + radius) > (collisionObjects[i]->at(j).transform.translation.z - (collisionObjects[i]->at(j).transform.scale.z / 2.f))) &&
						((translationA[2] - radius) < (collisionObjects[i]->at(j).transform.translation.z + (collisionObjects[i]->at(j).transform.scale.z / 2.f))) &&
						(translationA[1] > collisionObjects[i]->at(j).transform.translation.y) && 
						((translationA[1] + attemptedVerticalMovement) < (collisionObjects[i]->at(j).transform.translation.y))
						) {
						//std::cout << "within X and Z, moving down thru the object" << std::endl;
						//std::cout << "end of checkforground: " << std::endl;
						return { true, collisionObjects[i]->at(j).transform.translation.y };


					}
				}
			}
		}
		//std::cout << "ending check for ground" << std::endl;
		//std::cout << "end of checkforground: " << std::endl;
		return collisionReturn{};
	}

	bool Collision::checkIfStillGrounded(std::array<float, 3>& translationA, float radius) {
		//printf("collisionObjects.size(): %d \n", collisionObjects.size());
		//std::cout << "beginning of still grounded " << std::endl;
		for (int i = 0; i < collisionObjects.size(); i++) {
			//std::cout << "inside? : " << i << std::endl;
			//currently only texturedGameObjects will be used for collision, eventually ill use textured as well, and texturedGameObjects will be invisible collision only
			//avoid placing collideables above/below each other.

			//printf("collisionObjects[i].size(): %d \n", collisionObjects[i]->size());

			//as is, its being treated like a cube and not a cylinder
			for (int j = 0; j < collisionObjects[i]->size(); j++) {
				if (collisionObjects[i]->at(j).hasCollision()) {
					if (collisionObjects[i]->at(j).isWall) {
						continue;
					}
					if (((translationA[0] + radius) > (collisionObjects[i]->at(j).transform.translation.x - (collisionObjects[i]->at(j).transform.scale.x / 2.f))) &&
						((translationA[0] - radius) < (collisionObjects[i]->at(j).transform.translation.x + (collisionObjects[i]->at(j).transform.scale.x / 2.f))) &&
						((translationA[2] + radius) > (collisionObjects[i]->at(j).transform.translation.z - (collisionObjects[i]->at(j).transform.scale.z / 2.f))) &&
						((translationA[2] - radius) < (collisionObjects[i]->at(j).transform.translation.z + (collisionObjects[i]->at(j).transform.scale.z / 2.f)))// &&
						//(translationa[1] == collisionObjects[i]->at(j).transform.translation.y)
						) {
						//std::cout << "in bounds, grounded? ~ " <<  translationa[1] << ":" << collisionObjects[i]->at(j).transform.translation.y << std::endl;
						//std::cout << "end of still grounded " << std::endl;
						return true;


					}
				}
			}
		}
		//std::cout << "lost the floor" << std::endl;
		return false;
	}

	bool Collision::checkForWallCollision(std::array<float, 3>& translationA, glm::vec3 intendedMovement, float radius, float height) {
		for (int i = 0; i < collisionObjects.size(); i++) {
			for (int j = 0; j < collisionObjects[i]->size(); j++) {
				if (!collisionObjects[i]->at(j).isWall) {
					continue;
				}
				/*
				this is gonna be tough, come back to it later
				need to consider rotations, scale, etc
				*/

				if (((translationA[0] + radius) > (collisionObjects[i]->at(j).transform.translation.x - (collisionObjects[i]->at(j).transform.scale.x))) &&
					((translationA[0] - radius) < (collisionObjects[i]->at(j).transform.translation.x + (collisionObjects[i]->at(j).transform.scale.x))) &&
					((translationA[2] + radius) > (collisionObjects[i]->at(j).transform.translation.z - (collisionObjects[i]->at(j).transform.scale.z))) &&
					((translationA[2] - radius) < (collisionObjects[i]->at(j).transform.translation.z + (collisionObjects[i]->at(j).transform.scale.z)))// &&
					//(translationa[1] == collisionObjects[i]->at(j).transform.translation.y)
					) {
					//std::cout << "in bounds, grounded? ~ " <<  translationa[1] << ":" << collisionObjects[i]->at(j).transform.translation.y << std::endl;
					return true;


				}
			}
		}
		return false;
	}
}