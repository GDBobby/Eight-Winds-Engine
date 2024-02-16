#pragma once
#include "EWEngine/EWE_GameObject.h"

namespace EWE {

	class PointLight : public EWEGameObject
	{
	public:

		static PointLight makePointLight(float intensity = 1.f, float radius = 0.1f, glm::vec3 lightColor = glm::vec3(1.f)) {
			return PointLight{ intensity, radius, lightColor };
		}
		//static PointLight makeTextBilboard(float size, std::string text, int xPos, int yPos);

		PointLight(const PointLight&) = delete;
		PointLight& operator=(const PointLight&) = delete;
		PointLight(PointLight&&) = default;
		PointLight& operator=(PointLight&&) = default;

		void static update(float frameTime, std::vector<PointLight>& pointLights);

		float lightIntensity = 1.0f;
		glm::vec3 color{ 1.f };
	private:
		PointLight(float intensity, float radius, glm::vec3 lightColor);
	};

	class SpotLight : public EWEGameObject {
	public:
		float lightIntensity = 1.0f;
		glm::vec3 color{ 1.f };
		glm::vec3 direction;
		float cutoff;
		static SpotLight makeSpotLight(glm::vec3 position, glm::vec3 lightColor, float intensity, glm::vec3 direction, float cutoff = (glm::pi<float>() / 3)) {
			return SpotLight{position, lightColor, intensity, direction, cutoff};
		}

	private:
		SpotLight(glm::vec3 position, glm::vec3 lightColor, float intensity, glm::vec3 lightDirection, float angleCutoff = (glm::pi<float>() / 3)) {
			transform.translation = position;
			color = lightColor;
			lightIntensity = intensity;
			direction = lightDirection;
			cutoff = angleCutoff;
		}
	};
}