#pragma once

#include "EWEngine/EWE_GameObject.h"


#include <cmath>
#include <random>

namespace EWE {
	class LightningSystem {
	public:
		LightningSystem() : r{}, randomGen{ r() }, branchDistribution{ 4,6 }, rotationDistribution{ 0.f, glm::pi<float>() } {

			translations.resize(50);
			for (int i = 0; i < translations.size() - 1; i++) {
				if (i < (translations.size() - 1)) {
					translations[i].resize(branchDistribution(randomGen));
				}
				else {
					translations[i].resize(1);
				}
			}
		}
		std::random_device r{};
		std::default_random_engine randomGen;
		std::uniform_int_distribution<int> branchDistribution;
		std::uniform_real_distribution<float> rotationDistribution;
		
		//i dont understand 3d math but this works
		glm::vec4 beginningOffset = { -62.555f, 105.59201f, 26.48977f, 1.f };

		bool active = true;

		std::vector<std::vector<glm::vec3>> translations{};

		uint32_t updateTimer = 0;
		uint32_t updateDelay = 48;

		void beginLightning();

		void update(glm::mat4* swordMatrix, float rotation, std::array<float, 3>& startingTranslation, glm::vec3& secondTranslation, std::array<float, 3>& finalTranslation);

		void update(const glm::mat4& startingMatrix, float rotation, std::vector<glm::vec3*>& translations);

		uint8_t getCurrentActive() { return currentActive; }
		uint8_t currentActive{ 0 };

	private:

	};
}
