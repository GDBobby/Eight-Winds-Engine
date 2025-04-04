#include "EWEngine/LoadingScreen/LeafSystem.h"
#include "EWEngine/Data/EWE_Import.h"
#include "EWEngine/Graphics/Texture/Image_Manager.h"

namespace EWE {
	//id like to move some of the random generation components to local scope on leaf generation, not sure which ones yet
	//id also like to attempt to move this entire calculation to the GPU on compute shaders
	LeafSystem::LeafSystem() :
#if EWE_DEBUG
		PipelineSystem{ Pipe::loading },
#endif
		ranDev{}, randomGen{ ranDev() }, ellipseRatioDistribution{ 1.f,2.f }, rotRatioDistribution{ 1.f, 4.f },
		angularFrequencyDistribution{ glm::pi<float>(), glm::two_pi<float>() }, initTimeDistribution{ 0.f, 20.f },
		motionDistribution{ 0, 100 }, ellipseOscDistribution{ 0.75f, 1.25f }, depthVarianceDistribution{ -5.f, 5.f },
		widthVarianceDistribution{ -80.f, 60.f }, fallSwingVarianceDistribution{ 5.f, 10.f }, initHeightVarianceDistribution{ 2.f, 40.f }, varianceDistribution{ -.5f, .5f },
		rockDist{ 1.75f, 2.25f }

	{
		CreatePipeline();

		//printf("leafTextureID :%d \n", leafTextureID);

		//printf("after leaf construction \n");
	}

	LeafSystem::~LeafSystem() {
#if DECONSTRUCTION_DEBUG
		printf("begin deconstructing leaf system \n");
#endif
		Deconstruct(leafModel);
		EWE_VK(vkDestroyShaderModule, VK::Object->vkDevice, vertexShaderModule, nullptr);
		EWE_VK(vkDestroyShaderModule, VK::Object->vkDevice, fragmentShaderModule, nullptr);

		EWE_VK(vkDestroyPipelineLayout, VK::Object->vkDevice, pipeLayout, nullptr);

		for (auto& buffer : leafBuffer) {
			Deconstruct(buffer);
		}
#if DECONSTRUCTION_DEBUG
		printf("end deconstructing leaf system \n");
#endif
	}

	void LeafSystem::InitData() {

		LoadLeafModel();
#if EWE_DEBUG
		printf("after leaf mesh\n");
#endif

		LoadLeafTexture();

#if EWE_DEBUG
		printf("after leaf texture\n");
#endif

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			leafBuffer[i] = Construct<EWEBuffer>({ sizeof(glm::mat4) * LEAF_COUNT, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT });

			leafBuffer[i]->Map();

			leafBufferData[i] = reinterpret_cast<float*>(leafBuffer[i]->GetMappedMemory());
		}
#if DEBUG_NAMING
		leafBuffer[0]->SetName("leaf instance[0]");
		leafBuffer[1]->SetName("leaf instance[1]");
#endif

		LeafPhysicsInitialization();
		CreateDescriptor();
	}

	void LeafSystem::LeafPhysicsInitialization(){
		leafs.resize(LEAF_COUNT);
		
		for (uint16_t i = 0; i < LEAF_COUNT; i++) {
			auto& leaf = leafs[i];
			//printf("leaf initiation : %d \n", i);
			//0.7071067811865475
			//origin -= average velocity * randomTime; 

			//average velocity

			//give a starting position in a box, then subtract position by an inverse amount of time


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
			const std::size_t memOffset = (sizeof(glm::mat4) / sizeof(float) * i);
			leaf.transform.mat4(leafBufferData[0] + memOffset);
			leaf.transform.mat4(leafBufferData[1] + memOffset);

			
		}
		leafBuffer[0]->Flush();
		leafBuffer[1]->Flush();

	}

	void LeafSystem::FallCalculation(float timeStep) {
		//angF = angularFrequency * timestep
		//EllRatio = ratio of minor to major axis in ellipse
		//rotRatio = ratio of elliptical oscillation to rotation of leaf itself
		//printf("beginning fall calculation \n");
		//printf("timeStep : %.5f \n", timeStep);v
		for(uint16_t i = 0; i < LEAF_COUNT; i++){
			auto& leaf = leafs[i];

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



			leaf.transform.mat4(leafBufferData[VK::Object->frameIndex] + (sizeof(glm::mat4) / sizeof(float) * i));
			/*
			if (i == 0) {
				printf("translation, motion Type - %.3f:%.3f:%.3f - %d \n", leaf.transform.translation.x, leaf.transform.translation.y, leaf.transform.translation.z, leaf.fallMotion);
			}
			*/

		}
		leafBuffer[VK::Object->frameIndex]->Flush();
		//printf("before instancing \n");
		//return leafModel->updateInstancing(LEAF_COUNT, sizeof(glm::mat4), transformBuffer.data(), frameIndex, cmdBuf);
		//printf("after instancing \n");
		//printf("end of fall calculation \n");
		//could use a buffer and trim instances that are out of view, might be a compute shader kinda thing

	}

	////this should be a graphics queue command buffer
	void LeafSystem::LoadLeafModel() {
		//printf("loading leaf model \n");
		std::ifstream inFile("models/leaf_simpleNTMesh.ewe", std::ifstream::binary);
		//inFile.open();
		assert(inFile.is_open() && "failed to open leaf model");
		//printf("before formatingg input file in mesh \n");
		//printf("before synchronizing \n");
		//binary_input_archive& fileData;
		ImportData::TemplateMeshData<VertexNT> importMesh{};

		uint32_t endianTest = 1;
		bool endian = (*((char*)&endianTest) == static_cast<char>(1));

		if (endian) {
			importMesh.ReadFromFile(inFile);
		}
		else {
			importMesh.ReadFromFileSwapEndian(inFile);
		}
		inFile.close();
		//printf("file read successfully \n");

		leafModel = Construct<EWEModel>({ importMesh.meshes[0].vertices.data(), importMesh.meshes[0].vertices.size(), importMesh.vertex_size, importMesh.meshes[0].indices});

#if DEBUG_NAMING
		leafModel->SetDebugNames("leafModel");
#endif
	}
	void LeafSystem::LoadLeafTexture() {
		const std::string fullLeafTexturePath = "textures/leaf.jpg";
		//Image::CreateImage(&leafImageInfo, fullLeafTexturePath, false);

		leafImgID = Image_Manager::GetCreateImageID(fullLeafTexturePath, false);

		const std::string leafTexturePath = "leaf.jpg";
		//printf("leaf model loaded \n");
	}

	void LeafSystem::CreateDescriptor() {
		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			EWEDescriptorWriter descWriter{leafEDSL, DescriptorPool_Global};
			DescriptorHandler::AddGlobalsToDescriptor(descWriter, i);
			//descWriter.WriteImage(leafImageInfo.GetDescriptorImageInfo());

			descWriter.WriteImage(leafImgID);
			descWriter.WriteBuffer(leafBuffer[i]->DescriptorInfo());
			leafDescriptor[i] = descWriter.Build();
		}
#if DEBUG_NAMING
		DebugNaming::SetObjectName(leafDescriptor[0], VK_OBJECT_TYPE_DESCRIPTOR_SET, "leaf descriptor[0]");
		DebugNaming::SetObjectName(leafDescriptor[1], VK_OBJECT_TYPE_DESCRIPTOR_SET, "leaf descriptor[1]");
#endif
	}


	void LeafSystem::Render() {
#if EWE_DEBUG
		currentPipe = myID;
#endif
		BindPipeline();
		BindDescriptor(0, &leafDescriptor[VK::Object->frameIndex]);

		leafModel->BindAndDrawInstanceNoBuffer(LEAF_COUNT);
	}
	void LeafSystem::CreatePipeline() {
		CreatePipeLayout();


		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.pipelineLayout = pipeLayout;
		pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<VertexNT>();
		pipelineConfig.attributeDescriptions = VertexNT::GetAttributeDescriptions();

		Pipeline_Helper_Functions::CreateShaderModule("leaf.vert.spv", &vertexShaderModule);
		Pipeline_Helper_Functions::CreateShaderModule("leaf.frag.spv", &fragmentShaderModule);

		pipe = std::make_unique<EWEPipeline>(vertexShaderModule, fragmentShaderModule, pipelineConfig);
	}
	void LeafSystem::CreatePipeLayout() {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		EWEDescriptorSetLayout::Builder dslBuilder{};
		leafEDSL = dslBuilder
		.AddGlobalBindings()
		.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.Build();

		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = leafEDSL->GetDescriptorSetLayout();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipeLayout);
	}
}