#pragma once
#include "../data/EWEImport.h"
#include "../graphics/EWE_texture.h"

#include <random>

//angF = angularFrequency * timestep
//EllRatio = ratio of minor to major axis in ellipse
//rotRatio = ratio of elliptical oscillation to rotation of leaf itself

//credit to Martin Pazout, his masters thesis on Falling Leaves Simulation
//idk how to cite so im just gonna put the school too
//czech technical university in prague


#define LEAF_COUNT 250
#define WIND_SPEED 4.f

class LeafSystem {
public:

	enum LeafFallMotions {
		LF_Steady,
		LF_Tumbling,
		LF_Fluttering,
		LF_Chaotic, //only diff between chaotic and tumbling is directrion of rotation
		LF_Helix,
		LF_Spiral, //no diff between helix and spiral, just variable values

		LF_Count,
	};

	struct LeafStruct {
		TransformComponent transform{};
		glm::vec3 origin{ 0.f };
		LeafFallMotions fallMotion{ LF_Steady };
		glm::vec3 averageVelocity{ 0 };
		float angF{0.f};
		float ellRatio{ 0.f };
		float rotRatio{ 0.f };
		float ellOsc{ 0.f };
		float time = 0.f;
		float fallAmplitude{ 0.f };
		float swingAmplitude{ 0.f };
		float rotSpeed{ 0.f };
	};


	//helix -> ellRatio~1  rotRatio~1
	//spiral -> elLRatio~0 rotRatio~4
	LeafSystem(EWE::EWEDevice& device);

	void fallCalculation(float timeStep, uint8_t frameIndex);
	void loadLeafModel(EWE::EWEDevice& device) {
		//printf("loading leaf model \n");
		std::ifstream inFile("models/leaf_simpleNTMesh.ewe", std::ifstream::binary);
		//inFile.open();
		if (!inFile.is_open()) {
			printf("failed to open leaf model \n");
			//std throw
		}
		//printf("before formatingg input file in mesh \n");
		boost::archive::binary_iarchive binary_input_archive(inFile, boost::archive::no_header);
		//printf("before synchronizing \n");
		//binary_input_archive& fileData;
		ImportData::meshNTSimpleData importMesh;
		binary_input_archive& importMesh;
		inFile.close();
		//printf("file read successfully \n");
		leafModel = EWE::EWEModel::createMesh(device, importMesh.meshesNTSimple[0].first, importMesh.meshesNTSimple[0].second);
		//printf("leaf model loaded \n");
	}

	std::unique_ptr<EWE::EWEModel> leafModel;
	TextureID leafTextureID = 0;
	void* getLeafTransformBuffer() {
		return transformBuffer.data();
	}

	void draw(VkCommandBuffer cmdBuf) {
		leafModel->BindAndDrawInstanceNoBuffer(cmdBuf, LEAF_COUNT);
	}

private:
	std::random_device ranDev;
	std::mt19937 randomGen;
	std::uniform_real_distribution<float> ellipseRatioDistribution;
	std::uniform_real_distribution<float> rotRatioDistribution;
	std::uniform_real_distribution<float> initTimeDistribution;
	std::uniform_real_distribution<float> angularFrequencyDistribution;
	std::uniform_real_distribution<float> ellipseOscDistribution;
	std::uniform_real_distribution<float> widthVarianceDistribution;
	std::uniform_real_distribution<float> depthVarianceDistribution;
	std::uniform_real_distribution<float> fallSwingVarianceDistribution;
	std::uniform_real_distribution<float> initHeightVarianceDistribution;
	std::uniform_real_distribution<float> varianceDistribution;
	std::uniform_real_distribution<float> rockDist;


	std::uniform_int_distribution<int> motionDistribution;

	float gravity = -1.06566f;
	float FrictionPerp = 5.f;
	float leafWeight = 1.f; //grams
	float leafDensity = 0.1f;
	float leafKA = 4.f;

	LeafStruct leafs[LEAF_COUNT];
	std::vector<glm::mat4> transformBuffer{ LEAF_COUNT };






/*
//POSITION:
	yVelocity = 200; //pixels per second
	oscFreq = 1.5; //oscillations per second
	oscDepth = 35; //oscillation depth (pixels)
	drift = 25; // drift (wind?) (pixels per second: - = left, + = right)

	value + [oscDepth * Math.sin(oscFreq * Math.PI * 2 * time) + drift * time,
		yVelocity * time, 0]

//Z ROTATION :

	seed_random(index, true);
	random(360);

//Y ROTATION :

	oscFreq = 1.5;
	maxTilt = 15; //degrees

	maxTilt* Math.cos(oscFreq* Math.PI * 2 * time)
*/
};