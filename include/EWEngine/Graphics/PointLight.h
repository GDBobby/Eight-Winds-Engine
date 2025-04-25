#pragma once

#include <LAB/Transform.h>

#include <vector>

namespace EWE {

	class PointLight
	{
	public:

		static PointLight makePointLight(float intensity = 1.f, float radius = 0.1f, lab::vec3 lightColor = lab::vec3(1.f)) {
			return PointLight{ intensity, radius, lightColor };
		}
		//static PointLight makeTextBilboard(float size, std::string text, int xPos, int yPos);

		PointLight(const PointLight&) = delete;
		PointLight& operator=(const PointLight&) = delete;
		PointLight(PointLight&&) = default;
		PointLight& operator=(PointLight&&) = default;

		void static update(float frameTime, std::vector<PointLight>& pointLights);

		float lightIntensity = 1.0f;
		lab::vec3 color{ 1.f };
		lab::Transform<float, 3> transform;
	private:
		PointLight(float intensity, float radius, lab::vec3 lightColor);
	};

	class SpotLight {
	public:
		float lightIntensity = 1.0f;
		lab::vec3 color{ 1.f };
		lab::vec3 direction;
		float cutoff;
		lab::Transform<float, 3> transform;
		static SpotLight makeSpotLight(lab::vec3 position, lab::vec3 lightColor, float intensity, lab::vec3 direction, float cutoff = lab::GetPI_DividedBy(3.f)) {
			return SpotLight{position, lightColor, intensity, direction, cutoff};
		}

	private:
		SpotLight(lab::vec3 position, lab::vec3 lightColor, float intensity, lab::vec3 lightDirection, float angleCutoff = lab::GetPI_DividedBy(3.f)) {
			transform.translation = position;
			color = lightColor;
			lightIntensity = intensity;
			direction = lightDirection;
			cutoff = angleCutoff;
		}
	};
}