#include "EWEngine/Graphics/PointLight.h"



#include <array>

namespace EWE {
    PointLight::PointLight(float intensity, float radius, glm::vec3 lightColor) : lightIntensity{ intensity }, color{ lightColor } {
        transform.scale.x = radius;
    }


	void PointLight::update(float frameTime, std::vector<PointLight>& pointLights) {


		auto rotateLight = glm::rotate(glm::mat4(1.f), frameTime, { 0.f, 1.f, 0.f });

		for (int i = 0; i < pointLights.size(); i++) {
			//update light position
			pointLights[i].transform.translation = glm::vec3(rotateLight * glm::vec4(pointLights[i].transform.translation, 1.f));
		}
	}
}