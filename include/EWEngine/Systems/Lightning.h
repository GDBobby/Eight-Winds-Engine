#pragma once

#include "LAB/Vector.h"
#include "LAB/Matrix.h"

#include <cmath>
#include <random>

namespace EWE {
	class LightningSystem {
	public:
		LightningSystem() : r{}, randomGen{ r() }, branchDistribution{ 4,6 }, rotationDistribution{ 0.f, lab::PI<float> } {

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
		lab::vec4 beginningOffset = { -62.555f, 105.59201f, 26.48977f, 1.f };

		bool active = true;

		std::vector<std::vector<lab::vec3>> translations{};

		uint32_t updateTimer = 0;
		uint32_t updateDelay = 48;

		void beginLightning();

		void update(lab::mat4* swordMatrix, float rotation, std::array<float, 3>& startingTranslation, lab::vec3& secondTranslation, std::array<float, 3>& finalTranslation);

		void update(const lab::mat4& startingMatrix, float rotation, std::vector<lab::vec3*>& translations);

		uint8_t getCurrentActive() { return currentActive; }
		uint8_t currentActive{ 0 };

	private:

	};
}
