#include "EWEngine/LoadingScreen/LeafSystem.h"

#include "EWEngine/Graphics/Textures/Texture_Manager.h"

namespace EWE {
	LeafSystem::LeafSystem(EWEDevice& device) : ranDev{}, randomGen{ ranDev() }, ellipseRatioDistribution{ 1.f,2.f }, rotRatioDistribution{ 1.f, 4.f },
		angularFrequencyDistribution{ glm::pi<float>(), glm::two_pi<float>() }, initTimeDistribution{ 0.f, 20.f },
		motionDistribution{ 0, 100 }, ellipseOscDistribution{ 0.75f, 1.25f }, depthVarianceDistribution{ -5.f, 5.f },
		widthVarianceDistribution{ -80.f, 60.f }, fallSwingVarianceDistribution{ 5.f, 10.f }, initHeightVarianceDistribution{ 2.f, 40.f }, varianceDistribution{ -.5f, .5f },
		rockDist{ 1.75f, 2.25f },
		device{device}
	{
		myID = UINT32_MAX;

		leafBuffer.reserve(MAX_FRAMES_IN_FLIGHT);
		leafBufferData.reserve(MAX_FRAMES_IN_FLIGHT);
		transformDescriptor.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);


		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			leafBuffer.emplace_back(new EWEBuffer(device, sizeof(glm::mat4) * LEAF_COUNT, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
			leafBuffer[i]->map();
			leafBufferData.emplace_back(reinterpret_cast<float*>(leafBuffer[i]->getMappedMemory()));

			if (!
				EWEDescriptorWriter(DescriptorHandler::getLDSL(LDSL_boned), DescriptorPool_Global)
				.writeBuffer(0, leafBuffer[i]->descriptorInfo())
				.build(transformDescriptor[i])
				) {
				printf("loading desc set failure \n");
			}
		}

		leafs.resize(LEAF_COUNT);
		loadLeafModel(device);

		createPipeline(device);
		leafTextureID = Texture_Builder::createSimpleTexture("leaf.jpg", false, false, VK_SHADER_STAGE_FRAGMENT_BIT);


		//printf("leafTextureID :%d \n", leafTextureID);
		for (auto& leaf : leafs) {
			//printf("leaf initiation : %d \n", i);
			//0.7071067811865475
			//origin -= average velocity * randomTime; 

			//average velocity

			//give a starting position in a box, then subtract position by an inverse amount of time
			//could even just 


			leaf.angF = angularFrequencyDistribution(randomGen);
			leaf.ellRatio = ellipseRatioDistribution(randomGen);
			leaf.rotRatio = rotRatioDistribution(randomGen);
			leaf.ellOsc = ellipseOscDistribution(randomGen);
			leaf.fallAmplitude = fallSwingVarianceDistribution(randomGen);
			leaf.swingAmplitude = fallSwingVarianceDistribution(randomGen);
			leaf.rotSpeed = angularFrequencyDistribution(randomGen);


			float depth = depthVarianceDistribution(randomGen);
			float width = widthVarianceDistribution(randomGen);
			leaf.origin = glm::vec3{ 0.707106781f * (width - depth) - 3.f, 0.f, 0.707106781f * (depth - width) - 3.f };

			int motDis = motionDistribution(randomGen);
			if (motDis < 10) {
				leaf.fallMotion = LF_Steady;
				leaf.transform.rotation.x = -glm::half_pi<float>() * .9f;
				leaf.averageVelocity = glm::vec3(WIND_SPEED, gravity * glm::pi<float>() * 9.f / 4.f, -WIND_SPEED);
				leaf.transform.translation = leaf.origin - leaf.averageVelocity * (20.f - initTimeDistribution(randomGen) * 0.666f);
			}
			else if (motDis < 43) {
				leaf.fallMotion = LF_Fluttering;
				//velocity.y = (gravity + (leaf.swingAmplitude * glm::sin(2.f * leaf.angF * leaf.time)) / leaf.rotRatio) * leaf.ellOsc
				leaf.averageVelocity = glm::vec3(WIND_SPEED, gravity * 3.f, -WIND_SPEED);
				leaf.transform.translation = leaf.origin - leaf.averageVelocity * (20.f - initTimeDistribution(randomGen));
			}
			else if (motDis < 75) {
				leaf.fallMotion = LF_Chaotic;
				leaf.time += initTimeDistribution(randomGen);
				leaf.averageVelocity = glm::vec3(WIND_SPEED, gravity * 3.f, -WIND_SPEED);
				leaf.transform.translation = leaf.origin - leaf.averageVelocity * (20.f - initTimeDistribution(randomGen));
				//velocity.y = (gravity + (leaf.swingAmplitude * glm::sin(2.f * leaf.angF * leaf.time)) / leaf.rotRatio) * leaf.ellOsc;
			}
			else {
				leaf.fallMotion = LF_Spiral;
				leaf.averageVelocity = glm::vec3(WIND_SPEED, gravity * 3.f, -WIND_SPEED);
				leaf.origin -= leaf.averageVelocity * 20.f;
				leaf.time = initTimeDistribution(randomGen);
			}
			//leaf.fallMotion = (LeafFallMotions)motionDistribution(randomGen);

			//leaf.transform.translation = leaf.origin;
			//fallSwingVarianceDistribution(randomGen);
			leaf.transform.mat4(leafBufferData[frameIndex]);
		}

		//printf("after leaf construction \n");
	}

	LeafSystem::~LeafSystem() {
		vkDestroyShaderModule(device.device(), vertexShaderModule, nullptr);
		vkDestroyShaderModule(device.device(), vertexShaderModule, nullptr);

		vkDestroyPipelineLayout(device.device(), pipeLayout, nullptr);

		for (auto& buffer : leafBuffer) {
			delete buffer;
		}
	}

	void LeafSystem::fallCalculation(float timeStep, uint8_t frameIndex) {
		//angF = angularFrequency * timestep
		//EllRatio = ratio of minor to major axis in ellipse
		//rotRatio = ratio of elliptical oscillation to rotation of leaf itself
		//printf("beginning fall calculation \n");
		//printf("timeStep : %.5f \n", timeStep);
		for (auto& leaf : leafs) {

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



			leaf.transform.mat4(leafBufferData[frameIndex]);
			/*
			if (i == 0) {
				printf("translation, motion Type - %.3f:%.3f:%.3f - %d \n", leaf.transform.translation.x, leaf.transform.translation.y, leaf.transform.translation.z, leaf.fallMotion);
			}
			*/

		}
		leafBuffer[frameIndex]->flush();
		//printf("before instancing \n");
		//return leafModel->updateInstancing(LEAF_COUNT, sizeof(glm::mat4), transformBuffer.data(), frameIndex, cmdBuf);
		//printf("after instancing \n");
		//printf("end of fall calculation \n");
		//could use a buffer and trim instances that are out of view, might be a compute shader kinda thing

	}
	void LeafSystem::loadLeafModel(EWEDevice& device) {
		//printf("loading leaf model \n");
		std::ifstream inFile("models/leaf_simpleNTMesh.ewe", std::ifstream::binary);
		//inFile.open();
		if (!inFile.is_open()) {
			printf("failed to open leaf model \n");
			throw std::runtime_error("failed to open leaf model");
		}
		//printf("before formatingg input file in mesh \n");
		boost::archive::binary_iarchive binary_input_archive(inFile, boost::archive::no_header);
		//printf("before synchronizing \n");
		//binary_input_archive& fileData;
		ImportData::meshNTSimpleData importMesh;
		binary_input_archive& importMesh;
		inFile.close();
		//printf("file read successfully \n");
		leafModel = EWEModel::createMesh(device, importMesh.meshesNTSimple[0].first, importMesh.meshesNTSimple[0].second);
		//printf("leaf model loaded \n");
	}
	void LeafSystem::render(FrameInfo& frameInfo) {
		setFrameInfo(frameInfo);
		currentPipe = myID;
		bindPipeline();
		bindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameInfo.index));

		//printf("after binding descriptor set 0 \n");
		bindDescriptor(1, Texture_Manager::getDescriptorSet(leafTextureID));

		bindDescriptor(2, &transformDescriptor[frameInfo.index]);

		leafModel->BindAndDrawInstanceNoBuffer(cmdBuf, LEAF_COUNT);
	}
	void LeafSystem::createPipeline(EWEDevice& device) {
		createPipeLayout(device);


		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.pipelineLayout = pipeLayout;
		pipelineConfig.bindingDescriptions = LeafVertex::getBindingDescriptions();
		pipelineConfig.attributeDescriptions = LeafVertex::getAttributeDescriptions();

		printf("before loading vert shader \n");
		glslang::InitializeProcess();
		Pipeline_Helper_Functions::createShaderModule(device, ShaderBlock::getLoadingVertShader(), &vertexShaderModule);

		printf("before loading frag shader \n");
		Pipeline_Helper_Functions::createShaderModule(device, ShaderBlock::getLoadingFragShader(), &fragmentShaderModule);
		printf("after loading creation shaders \n");
		glslang::FinalizeProcess();

		pipe = std::make_unique<EWEPipeline>(device, vertexShaderModule, fragmentShaderModule, pipelineConfig);
	}
	void LeafSystem::createPipeLayout(EWEDevice& device) {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		std::vector<VkDescriptorSetLayout> setLayouts = {
			DescriptorHandler::getDescSetLayout(LDSL_global, device),
			TextureDSLInfo::getSimpleDSL(device, VK_SHADER_STAGE_FRAGMENT_BIT)->getDescriptorSetLayout(),
			DescriptorHandler::getDescSetLayout(LDSL_boned, device)
		};

		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
		pipelineLayoutInfo.pSetLayouts = setLayouts.data();

		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipeLayout) != VK_SUCCESS) {
			printf("failed to create leaf pipe layout\n");
			throw std::runtime_error("Failed to create pipe layout");
		}
	}
}