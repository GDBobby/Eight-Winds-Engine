#include "EWEngine/Graphics/PointLight.h"



#include <array>

namespace EWE {
    PointLight::PointLight(float intensity, float radius, lab::vec3 lightColor) : lightIntensity{ intensity }, color{ lightColor } {
        transform.scale.x = radius;
    }


	void PointLight::update(float frameTime, std::vector<PointLight>& pointLights) {


		auto rotateLight = lab::RotateAroundY(frameTime);

		for (int i = 0; i < pointLights.size(); i++) {
			//update light position
			pointLights[i].transform.translation = lab::vec3(rotateLight * lab::vec4(pointLights[i].transform.translation, 1.f));
		}
	}
}