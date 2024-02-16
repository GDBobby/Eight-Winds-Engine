/*
#pragma once

#include "flecs.h"

#include "EWEngine/data/TransformInclude.h"	

#include <memory>
#include <mutex>

namespace EWE {
	struct Translation { //horizontal
		float x; 
		float y;
	};

	struct Rotation2D {
		float y;
	};
	struct Velocity {
		float x;
		float y;
	};

	struct Pathing {
		Translation currentTarget;
		float speed;

	};
	struct Animation {
		uint8_t animState;
		uint16_t animFrame;
	};
	struct ModelMatrix {
		glm::mat4 modelMatrix;
	};
	struct SkeleJointMatrices {
		glm::mat4 matrices[68];
	};
	struct DeerJointMatrices {
		glm::mat4 matrices[68];
	};
	struct LichJointMatrices {
		glm::mat4 matrices[68];
	};
	struct DevilJointMatrices {
		glm::mat4 matrices[68];
	};

	class ECSManager {
	public:
	private:
		static std::shared_ptr<ECSManager> singleton;
		static std::once_flag initFlag_;
	
		ECSManager(const ECSManager&) = delete;
		ECSManager& operator=(const ECSManager&) = delete;
		ECSManager() : ECSworld{} {
			ECSworld.system<Translation, Velocity>().each([](Translation& p, Velocity& v) {
					p.x += v.x;
					p.y += v.y;
				}
			);


			flecs::entity Bob = ECSworld.entity("Bob")
				.set(Translation{ 0, 0 });
				//.set(Velocity{ 1, 2 })

			// Show us what you got
			//std::cout << Bob.name() << "'s got [" << Bob.type().str() << "]\n";

			//printf("%s got %s \n", Bob.name().c_str(), Bob.type().str().c_str());

			// Run systems twice. Usually this function is called once per frame
			ECSworld.progress();
			ECSworld.progress();

			// See if Bob has moved (he has)
			//const Position* p = Bob.get<Position>();
			//std::cout << Bob.name() << "'s position is {" << p->x << ", " << p->y << "}\n";
			//printf("%s position - %.2f:%.2f \n", Bob.name().c_str(), p->x, p->y);
		}


		flecs::world ECSworld;

	public:

		~ECSManager() {
			printf("Deconstructing ECS manager \n");
		}
		static void cleanup() {
			if (!singleton) {
				printf("trying to deconstruct  an object that doesn't exist \n");
				throw std::exception("trying to deconstruct  an object that doesn't exist");
			}
			singleton.reset();
		}
		static std::shared_ptr<ECSManager>& getSingleton() {
			std::call_once(initFlag_, []() {
				singleton = std::shared_ptr<ECSManager>(new ECSManager());
			});
			return singleton;
		}




	};
}
*/