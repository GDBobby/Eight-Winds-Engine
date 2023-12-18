#include "EWEngine/systems/Graphics/Lightning.h"
namespace EWE {


	void LightningSystem::beginLightning() {
		//resize it to 3
		//translations.resize(4); //normal value


		/*
		for (int i = 0; i < translations.size() - 2; i++) {
			translations[i].resize(branchDistribution(randomGen));
		}
		translations[2].resize(2);
		translations[3].resize(1);
		*/

		//do some random shit here
		//transforms[0].resize(randomValue);
		//do iw ant to do a random value every frame or once per lightning activation??
	}

	void LightningSystem::update(glm::mat4* swordMatrix, float rotation, std::array<float, 3>& startingTranslation, glm::vec3& secondTranslation, std::array<float, 3>& finalTranslation) {
		glm::vec3 startT = glm::vec3(startingTranslation[0], startingTranslation[1], startingTranslation[2]);
		glm::vec3 secondP = startT;
		secondP.y += 4.f;
		glm::vec3 finalT = glm::vec3(finalTranslation[0], finalTranslation[1], finalTranslation[2]);
		finalT.y += 1.f;
		std::vector<glm::vec3*> tempVec{ &startT, & secondP, & secondTranslation, & finalT };
		update(*swordMatrix, rotation, tempVec);
		return;
		//if (!active) { return; }
		//glm::vec4 begRot = ;
		translations[0][0] = *swordMatrix * beginningOffset;
		//float holder = translations[0][0].y;
		glm::vec3 tempRot = translations[0][0];
		tempRot.y = 0.f;
		tempRot = glm::rotate(glm::mat4(1.f), -rotation, glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(tempRot, 1.f);
		tempRot.y = translations[0][0].y;

		//translations[0][0] = tempRot + *anchorPoints[0];
		translations[0][0].x = startingTranslation[0] + tempRot.x;
		translations[0][0].y = startingTranslation[1] + tempRot.y;
		translations[0][0].z = startingTranslation[2] + tempRot.z;
		//translations[0][0].y += beginningOffset.y;
		//TransformComponent tempTransform = 
		//translations[0][0]
		//translations[0][0].y -= 2.f;

		//translations[1][0] = *anchorPoints[0];
		translations[1][0].x = startingTranslation[0];
		translations[1][0].y = startingTranslation[1] + 4.f;
		translations[1][0].z = startingTranslation[2];

		//special ball
		//translations[2][0] = *anchorPoints[1];
		translations[2][0] = secondTranslation;

		//translations[3][0] = *anchorPoints[2];
		//translations[3][0].y += 1.f;

		translations[3][0].x = finalTranslation[0];
		translations[3][0].y = finalTranslation[1] + 1.f;
		translations[3][0].z = finalTranslation[2];

		for (int i = 0; i < translations.size() - 1; i++) {
			if (i < 2) {
				translations[i].resize(branchDistribution(randomGen));
			}
			glm::vec3 startingPosition = translations[i][0];
			glm::vec3 endingPosition = translations[i + 1][0];
			glm::vec3 connector = (endingPosition - startingPosition) / static_cast<float>(translations[i].size());
			glm::vec3 theCross;
			if (i == 2) {
				theCross = glm::normalize(glm::cross(endingPosition - startingPosition, glm::vec3(1.f, 0.f, 0.f))) * 0.1f;
			}
			else {
				theCross = glm::normalize(glm::cross(endingPosition - startingPosition, glm::vec3(1.f, 0.f, 0.f))) * 0.25f;
			}
			for (int j = 1; j < translations[i].size(); j++) {
				translations[i][j] = glm::vec3(glm::rotate(glm::mat4(1.f), rotationDistribution(randomGen), connector) * glm::vec4(theCross, 1.f));
				translations[i][j] += startingPosition + (connector * static_cast<float>(j));
			}
		}

	}

	void LightningSystem::update(const glm::mat4& startingMatrix, float rotation, std::vector<glm::vec3*>& transPoints) {
		//if (!active) { return; }
		//glm::vec4 begRot = ;

		if (transPoints.size() == 0) {
			printf("0 lightning points??? \n");
			throw std::exception("0 lightning points??");
		}

		assert(transPoints.size() > 0);

		/*
		translations[0][0] = startingMatrix * beginningOffset;

		//float holder = translations[0][0].y;
		glm::vec3 tempRot = translations[0][0];
		tempRot.y = 0.f;
		tempRot = glm::rotate(glm::mat4(1.f), -rotation, glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(tempRot, 1.f);
		tempRot.y = translations[0][0].y;
		

		translations[0][0] = transPoints[0]; +tempRot;
		*/

		for (int i = 0; i < transPoints.size(); i++) { //size == 4, 0 1 2 3
			translations[i][0] = *transPoints[i];
			translations[i][0].y += 1.f;
		}

		for (int i = 0; i < transPoints.size() - 1; i++) { //i < 3
			translations[i].resize(branchDistribution(randomGen)); //0 1 2
			
			glm::vec3 startingPosition = translations[i][0];
			glm::vec3 endingPosition = translations[i + 1][0]; //up to 3

			//printf("starting translation[%d] : %.2f:%.2f:%.2f \n", i, startingPosition.x, startingPosition.y, startingPosition.z);
			//printf("ending translation[%d] : %.2f:%.2f:%.2f \n", i, endingPosition.x, endingPosition.y, endingPosition.z);


			glm::vec3 connector = (endingPosition - startingPosition) / static_cast<float>(translations[i].size());
			glm::vec3 theCross = glm::normalize(glm::cross(endingPosition - startingPosition, glm::vec3(0.f, 1.f, 0.f))) * 0.2f;
			
			for (int j = 1; j < translations[i].size(); j++) {
				translations[i][j] = glm::vec3(glm::rotate(glm::mat4(1.f), rotationDistribution(randomGen), connector) * glm::vec4(theCross, 1.f));
				translations[i][j] += startingPosition + (connector * static_cast<float>(j));
			}
		}
		currentActive = static_cast<uint8_t>(transPoints.size() - 1);

	}
}