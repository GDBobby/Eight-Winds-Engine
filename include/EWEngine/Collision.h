#pragma once


#include "EWE_GameObject.h"

//class ObjectManager;

enum collisionTypes {
	col_sphere,
	col_cylinder,
	col_rectangularprism,
	col_cone,
	col_pyramid,
	col_plane,
	col_ground,

	col_notValid, //is this necessary?
};
struct quadBounds {
	float xLeft;
	float xRight;
	float zLeft;
	float zRight;
	float yPos;
};

struct collisionReturn {
	bool check = false;
	float checkLocation = -69.f;
};

/*
how to deal with tunneling? make collideable objects bigger than the biggest movement by frame. should be easy considering 250fps

if transform.x + scale/2 + radius < dist(A,B) && y
*/
namespace EWE {
	class Collision {
		//DO NOT CONSTRUCT THIS CLASS ANYWHERES BESIDES THE OBJECT MANAGER


	public:
		//i need collision to have access to all collideable objects, aka everything the object manager has access to
		static collisionReturn checkForGround(std::array<float, 3>& translationA, float attemptedVerticalMovement, float radius);

		//this float* need to be a float[3]
		static bool checkIfStillGrounded(std::array<float, 3>& translation, float radius);
		static bool checkForWallCollision(std::array<float, 3>& translationA, glm::vec3 intendedMovement, float radius, float height);

		static std::vector<std::vector<EWEGameObject>*> collisionObjects;

	private:




	};
}