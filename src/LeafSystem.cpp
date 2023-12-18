#include "EWEngine/LoadingScreen/LeafSystem.h"


LeafSystem::LeafSystem(EWE::EWEDevice& device) : ranDev{}, randomGen{ ranDev() }, ellipseRatioDistribution{ 1.f,2.f }, rotRatioDistribution{ 1.f, 4.f },
angularFrequencyDistribution{ glm::pi<float>(), glm::two_pi<float>() }, initTimeDistribution{ 0.f, 20.f },
motionDistribution{ 0, 100 }, ellipseOscDistribution{ 0.75f, 1.25f }, depthVarianceDistribution{ -5.f, 5.f },
widthVarianceDistribution{ -80.f, 60.f }, fallSwingVarianceDistribution{ 5.f, 10.f }, initHeightVarianceDistribution{ 2.f, 40.f }, varianceDistribution{ -.5f, .5f },
rockDist{ 1.75f, 2.25f }
{
	loadLeafModel(device);
	leafTextureID = EWE::EWETexture::addGlobalTexture(device, "leaf.jpg");
	//printf("leafTextureID :%d \n", leafTextureID);
	for (int i = 0; i < LEAF_COUNT; i++) {
		//printf("leaf initiation : %d \n", i);
		//0.7071067811865475
		//origin -= average velocity * randomTime; 

		//average velocity

		//give a starting position in a box, then subtract position by an inverse amount of time
		//could even just 


		leafs[i].angF = angularFrequencyDistribution(randomGen);
		leafs[i].ellRatio = ellipseRatioDistribution(randomGen);
		leafs[i].rotRatio = rotRatioDistribution(randomGen);
		leafs[i].ellOsc = ellipseOscDistribution(randomGen);
		leafs[i].fallAmplitude = fallSwingVarianceDistribution(randomGen);
		leafs[i].swingAmplitude = fallSwingVarianceDistribution(randomGen);
		leafs[i].rotSpeed = angularFrequencyDistribution(randomGen);


		float depth = depthVarianceDistribution(randomGen);
		float width = widthVarianceDistribution(randomGen);
		leafs[i].origin = glm::vec3{ 0.707106781f * (width - depth) - 3.f, 0.f, 0.707106781f * (depth - width) - 3.f };

		int motDis = motionDistribution(randomGen);
		if (motDis < 10) {
			leafs[i].fallMotion = LF_Steady;
			leafs[i].transform.rotation.x = -glm::half_pi<float>() * .9f;
			leafs[i].averageVelocity = glm::vec3(WIND_SPEED, gravity * glm::pi<float>() * 9.f / 4.f, -WIND_SPEED);
			leafs[i].transform.translation = leafs[i].origin - leafs[i].averageVelocity * (20.f - initTimeDistribution(randomGen) * 0.666f);
		}
		else if (motDis < 43) {
			leafs[i].fallMotion = LF_Fluttering;
			//velocity.y = (gravity + (leaf.swingAmplitude * glm::sin(2.f * leaf.angF * leaf.time)) / leaf.rotRatio) * leaf.ellOsc
			leafs[i].averageVelocity = glm::vec3(WIND_SPEED, gravity * 3.f, -WIND_SPEED);
			leafs[i].transform.translation = leafs[i].origin - leafs[i].averageVelocity * (20.f - initTimeDistribution(randomGen));
		}
		else if (motDis < 75) {
			leafs[i].fallMotion = LF_Chaotic;
			leafs[i].time += initTimeDistribution(randomGen);
			leafs[i].averageVelocity = glm::vec3(WIND_SPEED, gravity * 3.f, -WIND_SPEED);
			leafs[i].transform.translation = leafs[i].origin - leafs[i].averageVelocity * (20.f - initTimeDistribution(randomGen));
			//velocity.y = (gravity + (leaf.swingAmplitude * glm::sin(2.f * leaf.angF * leaf.time)) / leaf.rotRatio) * leaf.ellOsc;
		}
		else {
			leafs[i].fallMotion = LF_Spiral;
			leafs[i].averageVelocity = glm::vec3(WIND_SPEED, gravity * 3.f, -WIND_SPEED);
			leafs[i].origin -= leafs[i].averageVelocity * 20.f;
			leafs[i].time = initTimeDistribution(randomGen);
		}
		//leafs[i].fallMotion = (LeafFallMotions)motionDistribution(randomGen);

		//leafs[i].transform.translation = leafs[i].origin;
		//fallSwingVarianceDistribution(randomGen);
		transformBuffer[i] = leafs[i].transform.mat4();
	}

	//printf("after leaf construction \n");
}


void LeafSystem::fallCalculation(float timeStep, uint8_t frameIndex) {
	//angF = angularFrequency * timestep
	//EllRatio = ratio of minor to major axis in ellipse
	//rotRatio = ratio of elliptical oscillation to rotation of leaf itself
	//printf("beginning fall calculation \n");
	//printf("timeStep : %.5f \n", timeStep);
	for (int i = 0; i < LEAF_COUNT; i++) {
		LeafStruct& leaf = leafs[i];

		leaf.time += timeStep;
		//leaf.time = glm::clamp(leaf.time, 0.f, glm::two_pi<float>());
		
		//dw/dt = change in leaf angF
		//theta = angle with xy plane
		//	  ^ = sin(transform.rotation.x);
		//a = angle with xz plane
		//^ = dot(normalize(velocity), glm::vec3{0.f,-1.f,0.f});
		//V is velocity
		//p = density of leaf

		//Ka = friction in the direction of the fall
		glm::vec3 velocity{ 0.f };
		float angFT = leaf.angF * leaf.time;

		switch (leaf.fallMotion) {
		case LF_Steady: {
			//leaf.transform.translation.x += varianceDistribution(randomGen);
			//leaf.transform.translation.y = leaf.origin.y + gravity * leaf.time;
			//leaf.transform.translation.z += varianceDistribution(randomGen);

			//velocity = glm::vec3(0.f);
			
			velocity.x = varianceDistribution(randomGen) + (WIND_SPEED * leaf.ellOsc); //ellOsc for wind variance
			velocity.y = gravity * 1.5f * leaf.rotSpeed; //ellOsc isnt related but im plugging it for gravity variance
			velocity.z = varianceDistribution(randomGen) - (WIND_SPEED * leaf.ellOsc);

			leaf.transform.translation += velocity * timeStep;
			leaf.transform.rotation.y += leaf.rotSpeed * 2.f * timeStep;
			break;
		}
		case LF_Tumbling:
		case LF_Fluttering:
			leaf.transform.rotation.y -= glm::half_pi<float>() * timeStep * leaf.ellOsc; //ellOsc isnt related but im plugging it cause it fits
			velocity.x = -leaf.fallAmplitude * glm::cos(angFT) * glm::sin(leaf.transform.rotation.y)
				+ (WIND_SPEED * leaf.ellOsc); //wind, ellOsc for variance
			velocity.y = (gravity + (leaf.swingAmplitude * glm::sin(2.f * leaf.angF * leaf.time)) / leaf.rotRatio) * leaf.ellOsc * 2.f; //JUST USING ROTRATIO AND ELLOSC FOR GRAVITY VARIANCE
			velocity.z = -leaf.fallAmplitude * glm::cos(angFT) * glm::sin(leaf.transform.rotation.y)
				- (WIND_SPEED * leaf.ellOsc); //wind, ellOsc for variance
			leaf.transform.translation += velocity * timeStep;
			leaf.transform.rotation.z = -leaf.swingAmplitude * glm::sin(leaf.angF * (leaf.time - leaf.angF * 1.f / 16.f)) / (glm::two_pi<float>()) * glm::sin(leaf.transform.rotation.y) * .75f;
			break;


		case LF_Chaotic:
			//velocity.z = -leaf.fallAmplitude * glm::sin(leaf.angF * leaf.time);
			//all going clockwise

			leaf.transform.rotation.y += glm::half_pi<float>() * timeStep * leaf.ellOsc;
			velocity.x = -leaf.fallAmplitude * glm::cos(angFT) * glm::sin(leaf.transform.rotation.y)
				+ (WIND_SPEED * leaf.ellOsc); //wind, ellOsc for variance
			velocity.y = (gravity + (leaf.swingAmplitude * glm::sin(2.f * leaf.angF * leaf.time)) / leaf.rotRatio) * leaf.ellOsc * 2.f;//JUST USING ROTRATIO AND ELLOSC FOR GRAVITY VARIANCE
			velocity.z = -leaf.fallAmplitude * glm::cos(angFT) * glm::sin(leaf.transform.rotation.y)
				- (WIND_SPEED * leaf.ellOsc); //wind, ellOsc for variance
			leaf.transform.translation += velocity * timeStep;
			leaf.transform.rotation.z = -leaf.swingAmplitude * glm::sin(leaf.angF * (leaf.time - leaf.angF * 1.f / 16.f)) / (glm::two_pi<float>()) * glm::sin(leaf.transform.rotation.y) * .75f;
			//leaf.transform.rotation.x = -leaf.swingAmplitude * glm::sin(leaf.angF * (leaf.time - leaf.angF * 1.f / 16.f)) / (glm::two_pi<float>()) * glm::cos(leaf.transform.rotation.y) / 2.f;

			break;
			/*
		case LF_Tumbling:
		case LF_Fluttering: {
			//leaf.transform.translation.x = leaf.origin.x - (leaf.fallAmplitude / leaf.angF) * glm::sin(leaf.angF * leaf.time);
			//leaf.transform.translation.y = leaf.origin.y + (gravity * leaf.time) - ((leaf.swingAmplitude/(2.f * leaf.angF)) * glm::cos(2.f * leaf.angF * leaf.time));
			//velocity = glm::vec3(0.f);
			
			velocity.x = -leaf.fallAmplitude * glm::cos(angFT);
			velocity.y = gravity + leaf.swingAmplitude * glm::sin(2.f * angFT);
			//velocity.z = varianceDistribution(randomGen);
			

			leaf.transform.translation += velocity * timeStep;
			leaf.transform.rotation.z = -leaf.swingAmplitude * glm::sin(leaf.angF * (leaf.time - leaf.angF * 1.f / 16.f)) / (glm::two_pi<float>());

			break;
		}
		*/
		case LF_Helix:
		case LF_Spiral: {
			//velocity = glm::vec3(0.f);
			glm::vec3 oldPos = leaf.transform.translation;
			
			leaf.transform.translation.x = leaf.origin.x + leaf.ellOsc * cos(angFT / 2.f) * (10.f + leaf.ellRatio * sin(leaf.rotRatio * angFT))
				+ (WIND_SPEED * leaf.time * leaf.ellOsc); //wind, ellOsc for variance							
			leaf.transform.translation.y = leaf.origin.y + gravity * leaf.time * leaf.ellOsc * 1.9f; //maybe multiply gravity by some value between 0.8 and 0.9
			leaf.transform.translation.z = leaf.origin.z + leaf.ellOsc * sin(angFT / 2.f) * (10.f + leaf.ellRatio * sin(leaf.rotRatio * angFT))
				- (WIND_SPEED * leaf.time * leaf.ellOsc); //wind, ellOsc for variance

			velocity = (leaf.transform.translation - oldPos) / timeStep;
			//leaf.rotRef = glm::mod(leaf.rotRef + , glm::half_pi<float>() / 2.f);
			//leaf.transform.rotation.z = leaf.rotRef - glm::half_pi<float>() / 4.f;

			//leaf.transform.rotation.z = glm::sin(leaf.rotRef);
			float horiPerc = 1.f - (velocity.y / glm::length(velocity));

			//leaf.transform.rotation.x = -glm::cos(horiPerc) * glm::quarter_pi<float>();
			leaf.transform.rotation.x = -glm::sin(horiPerc * glm::quarter_pi<float>());
			leaf.transform.rotation.y = glm::atan(velocity.x, velocity.z) + (glm::quarter_pi<float>() * 3.f);
			leaf.transform.rotation.x = -glm::cos(horiPerc * glm::quarter_pi<float>());
			//leaf.transform.rotation.z = -glm::atan(velocity.x, velocity.y) - (glm::quarter_pi<float>() * 3.f);

			//printf("leaf.transform.rotation.y : %.5f \n", leaf.transform.rotation.y);

			// if velocity.y == gravity, i want rotation.y to be half pi
			//if velocity.y == 0, i want rotation.y to be 0
			//or the other way around?
			
			break;
		}
		}

		if (leaf.transform.translation.y <= 0.f) {
			//printf("leaf hit ground \n");
			/*
			if (i == 0) {
				printf("leaf hit ground \n");
			}
			*/
			//roll a new motion type?
			//leaf.transform.translation = leaf.origin;
			leaf.transform.translation = leaf.origin - (leaf.averageVelocity * 20.f);
			
			//leaf.origin = leaf.transform.translation;
			leaf.time = 0.f;

			/*
			if (leaf.fallMotion == LF_Chaotic) {
				leaf.time += initTimeDistribution(randomGen);
			}
			*/
			//leaf.transform.rotation = glm::vec3(0.f, 0.f, -glm::quarter_pi<float>());
			//leaf.transform.rotation.x = glm::half_pi<float>();
		}
		else {
			
			//forwardDir = { sin(player.FollowCamera->transform.rotation.y), -sin(player.FollowCamera->transform.rotation.x), cos(player.FollowCamera->transform.rotation.y) };
			//^ following that logic
			//leaf.transform.rotation = glm::vec3(glm::asin(velNorm.y), glm::asin(velNorm.x), glm::acos(velNorm.y));


			//get direction of velocity, reverse that into rotation


			//leaf.transform.rotation.z += (-4 * leaf.transform.rotation.z - (3.f * glm::pi<float>() * leafDensity * (velocityXMag + velocityYMag) * glm::cos(beta) * glm::sin(beta))) * timeStep;
			//(-3.f * glm::pi<float>() * leafDensity * (velocityXMag + velocityYMag) * glm::cos(beta) * glm::sin(beta)) dt

			
			//glm::mod(leaf.transform.rotation.z, glm::two_pi<float>());
		}




		transformBuffer[i] = leaf.transform.mat4();
		/*
		if (i == 0) {
			printf("translation, motion Type - %.3f:%.3f:%.3f - %d \n", leaf.transform.translation.x, leaf.transform.translation.y, leaf.transform.translation.z, leaf.fallMotion);
		}
		*/

	}
	//printf("before instancing \n");
	//return leafModel->updateInstancing(LEAF_COUNT, sizeof(glm::mat4), transformBuffer.data(), frameIndex, cmdBuf);
	//printf("after instancing \n");
	//printf("end of fall calculation \n");
	//could use a buffer and trim instances that are out of view, might be a compute shader kinda thing
	
}