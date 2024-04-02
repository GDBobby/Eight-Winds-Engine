/*
#include "LevelBuilder.h"
namespace EWE {

	void LevelBuilder::LBKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureKeyboard) {
			//printf("imgui wants mouse capture \n");
			return;
		}
	}
	void LevelBuilder::LBMouseCallback(GLFWwindow* window, int button, int action, int mods) {
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse) {
			//printf("imgui wants mouse capture \n");
			return;
		}
	}


	LevelBuilder::LevelBuilder(ImGUIHandler* ImGuiHandler, GLFWwindow* window, ObjectManager* objMan, EWEGameObject* cameraObj, EWECamera* EWECamera, LightBufferObject* lbo, bool* shouldRenderPoints) : cameraControl{ window } {
		printf("constructing level builderr \n");
		imguiHandler = ImGuiHandler;
		cameraObject = cameraObj;
		camera = EWECamera;
		floorGridModel = EWEModel::generateQuad(device, { 100.f, 100.f });
		objectManager = objMan;
		uint32_t idChecker = -1;
		idChecker = EWETexture::addModeTexture(device, "floorGridTexture.png");
		if (idChecker > 0) {
			objectManager->lBuilderObjects[0] = EWEGameObject{};
			objectManager->lBuilderObjects[0].model = floorGridModel;
			objectManager->lBuilderObjects[0].textureID = idChecker;
			objectManager->lBuilderObjects[0].transform.scale = { 100.f, 1.f, 100.f };
		}
		idChecker = -1;
		idChecker = EWETexture::addModeTexture(device, "portalFloor.png", EWETexture::tType_sprite);
		if (idChecker > 0) {
			objectManager->spriteBuildObjects[0] = EWEGameObject{};//i dont think this line is necessary
			objectManager->spriteBuildObjects[0].model = EWEModel::generateQuad(device);
			printf("before sprite load \n");

			objectManager->spriteBuildObjects[0].textureID = idChecker;
			printf("after sprite load \n");
		}
		else {
			printf("couldn't find texture ID : %s \n", "portalFloor.png");
		}
		this->shouldRenderPoints = shouldRenderPoints;
		this->lbo = lbo;
		sunColor[0] = lbo->sunlightColor.x;
		sunColor[1] = lbo->sunlightColor.y;
		sunColor[2] = lbo->sunlightColor.z;
		printf("end of level building constructor \n");

		//lastTextureID = EWETexture::addModeTexture(eweDevice, "hazard.png");
		lastTextureID = EWETexture::addSmartModeTexture(eweDevice, "hazard.png", EWETexture::tType_smart);
		printf("after smart mode texture \n");
		materialHandler = RigidRenderingSystem::getRigidRSInstance();
		printf("after material handler \n");
	}
	
	void LevelBuilder::render(float dt) {
		//main controls
		cameraControl.moveInPlaneXZ(*cameraObject);
		cameraControl.rotateCam(*cameraObject);
		camera->SetViewYXZ(cameraObject->transform.translation, cameraObject->transform.rotation);
		cameraControl.zoom(cameraObject);
		addMainControls();
	}


	void LevelBuilder::addMainControls() {
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New", "")) {
					newLevelPrompt = !newLevelPrompt;
				}
				if (ImGui::MenuItem("Save", "Ctrl + S")) {
					saveLevelPrompt = !saveLevelPrompt;
					//LevelManager::saveLevel()
				}
				if (ImGui::MenuItem("Load", "Ctrl + L")) {
					loadLevelPrompt = !loadLevelPrompt;
				}
				if (ImGui::MenuItem("Return To Main", "Ctrl + R")) {
					returnToMain = true;
				}
				if (ImGui::MenuItem("Exit", "")) {
					closePlease = true;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Set Target Path")) {
					targetControlScheme = !targetControlScheme;
				}
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::EndMenu();
			}
			//printf("before ?? \n");
			if (ImGui::BeginMenu("View")) {
				if (ImGui::MenuItem("Add Quad", "F3", showAddQuad)) { showAddQuad = !showAddQuad; }
				if (ImGui::MenuItem("Object List", "F5", showObjectList)) { showObjectList = !showObjectList; }
				if (ImGui::MenuItem("Light Buffer Object", "F6", showLBOControls)) { showLBOControls = !showLBOControls; }
				ImGui::EndMenu();
			}
			//printf("after ?? \n");
			ImGui::EndMainMenuBar();
		}

		//ImGui::ShowDemoWindow();
		if (showLBOControls) {
			ImGui::Begin("sunlight variables");
			ImGui::Checkbox("render points?", shouldRenderPoints);

			ImGui::SliderFloat("sunlight intensity", &lbo->sunlightColor.w, 0.0, 1.0, "%.2f");

			ImGui::Text("sunlight color");

			ImGui::ColorEdit3("sun color", sunColor);
			lbo->sunlightColor.x = sunColor[0];
			lbo->sunlightColor.y = sunColor[1];
			lbo->sunlightColor.z = sunColor[2];

			ImGui::Text("sunlight direction");
			ImGui::InputFloat("sunlight dir.x", &lbo->sunlightDirection.x);
			ImGui::InputFloat("sunlight dir.y", &lbo->sunlightDirection.y);
			ImGui::InputFloat("sunlight dir.z", &lbo->sunlightDirection.z);

			if(ImGui::SmallButton("normalize")) {
				glm::vec3 tempDir{ lbo->sunlightDirection.x, lbo->sunlightDirection.y, lbo->sunlightDirection.z };
				tempDir = glm::normalize(tempDir);
				lbo->sunlightDirection.x = tempDir.x;
				lbo->sunlightDirection.y = tempDir.y;
				lbo->sunlightDirection.z = tempDir.z;
			}
			ImGui::End();
		}

		if (showAddQuad) {
			ImGui::Begin("Add Quad");
			ImGui::InputText("Texture  Location", materialLocation, MAX_LB_PATH);

			ImGui::Text("Translation");
			ImGui::InputFloat("translation.x", &placeTransform.translation.x, 0.1f, 1.f, "%.2f");
			ImGui::InputFloat("translation.y", &placeTransform.translation.y, 0.1f, 1.f, "%.2f");
			ImGui::InputFloat("translation.z", &placeTransform.translation.z, 0.1f, 1.f, "%.2f");

			ImGui::Text("Rotation");
			ImGui::InputFloat("rotation.x", &placeTransform.rotation.x, oneDegree, tenDegrees, "%.2f");
			ImGui::InputFloat("rotation.y", &placeTransform.rotation.y, oneDegree, tenDegrees, "%.2f");
			ImGui::InputFloat("rotation.z", &placeTransform.rotation.z, oneDegree, tenDegrees, "%.2f");

			ImGui::Text("Scale");
			ImGui::InputFloat("scale.x", &placeTransform.scale.x, 0.1f, 1.f, "%.2f");
			ImGui::InputFloat("scale.y", &placeTransform.scale.y, 0.1f, 1.f, "%.2f");
			ImGui::InputFloat("scale.z", &placeTransform.scale.z, 0.1f, 1.f, "%.2f");

			if (ImGui::Button("Add Quad")) {
				printf("add quad \n");

				*
					//objectManager->texturedBuildObjects[selectedObject] = std::move(objectManager->builderObjects[selectedObject]);
					//objectManager->builderObjects.erase(selectedObject);
					//objectList[selectedObject] = &objectManager->texturedBuildObjects;
				*
				std::string textureStringBuffer = materialLocation;
				lastTextureID = EWETexture::addSmartModeTexture(eweDevice, textureStringBuffer, EWETexture::tType_smart);
				if ((lastTextureID.first >= 0) && (lastTextureID.second >= 0)) {
					objectManager->dynamicBuildObjects[objectCounter] = EWEGameObject{};
					if (lastTextureID.first & MaterialF_hasNormal) {
						objectManager->dynamicBuildObjects[objectCounter].model = BuilderModel::generateTangentQuad(EWEDevice);
					}
					else {
						objectManager->dynamicBuildObjects[objectCounter].model = BuilderModel::generateNTQuad(EWEDevice);
					}

					//printf("vertex size? %d \n", ((BuilderModel*)objectManager->builderObjects[objectCounter].model.get())->vertices.size());
					objectManager->dynamicBuildObjects[objectCounter].transform = placeTransform;


					objectManager->dynamicBuildObjects[objectCounter].textureID = lastTextureID.second;

					objectList[objectCounter] = &objectManager->dynamicBuildObjects;
					materialHandler->addMaterialObject(lastTextureID.first, MOI_none, &objectManager->dynamicBuildObjects[objectCounter].transform, objectManager->dynamicBuildObjects[objectCounter].model.get(), lastTextureID.second, &objectManager->dynamicBuildObjects[objectCounter].drawable);

					selectedObject = objectCounter;
					showObjectControl = true;

					objectCounter++;

					quadWasAdded = true;
				}
				else {
					printf("adding invalid smart texture \n");
				}
			}
			ImGui::End();
		}

		if (showObjectList) {
			//printf("yo? \n");
			ImGui::Begin("Object List");
			if (ImGui::BeginTabBar("Objects")) {
				if (ImGui::BeginTabItem("Map Items")) {
					if (ImGui::BeginListBox("objects")) {
						for (auto iter = objectList.begin(); iter != objectList.end(); iter++) {
							//ImGui::Selectable(std::to_string(iter->first).c_str());
							const bool isSelected = (selectedObject == iter->first);
							if (ImGui::Selectable(std::to_string(iter->first).c_str(), isSelected)) {
								if (iter->first != selectedObject) {
									printf("selection changed \n");
									newUVFocus = true;
									selectedObject = iter->first;
								}
							}
							if (isSelected) { ImGui::SetItemDefaultFocus(); }
						}
						ImGui::EndListBox();
					}

					if (selectedObject < 6942000) {
						if (ImGui::SmallButton("edit")) {
							printf("selection? \n");
							showObjectControl = true;
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("Destroy")) {
							showObjectControl = false;
							((BuilderModel*)objectList[selectedObject]->at(selectedObject).model.get())->initiateDeletion();
							wantsToDestroy = true;

						}
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Global Items")) {
					if (ImGui::BeginListBox("global objects")) {
						const bool isSelected = selectedObject == spawnObjectID;
						if (ImGui::Selectable("spawn portal", isSelected)) {
							selectedObject = spawnObjectID;
						}
						if (ImGui::Selectable("grid", isSelected)) {
							selectedObject = gridObjectID;
						}
						if (isSelected) { ImGui::SetItemDefaultFocus(); }
						ImGui::EndListBox();
					}
					if (selectedObject == spawnObjectID || selectedObject == gridObjectID) {
						if (ImGui::SmallButton("edit_")) {
							printf("selection? \n");
							showObjectControl = true;
						}
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::End();
		}
		if (showObjectControl) {
			

			ImGui::Begin("Object Control");
			if ((selectedObject != spawnObjectID) && (selectedObject != gridObjectID)) {
				sprintf(genericBuffer, "Object %d", selectedObject);
			}
			else {
				if (selectedObject == spawnObjectID) {
					sprintf(genericBuffer, "Spawn Control");
				}
				else if (selectedObject == gridObjectID) {
					sprintf(genericBuffer, "Grid Control");
				}
			}
			ImGui::Text(genericBuffer);
			if (ImGui::BeginTabBar("ControlTabBar")) {
				if (selectedObject < 6942000) {
					if (ImGui::BeginTabItem("Transform")) {
						ImGui::Text("Translation");
						ImGui::InputFloat("translation.x", &objectList[selectedObject]->at(selectedObject).transform.translation.x, 0.1f, 1.f, "%.2f");
						ImGui::InputFloat("translation.y", &objectList[selectedObject]->at(selectedObject).transform.translation.y, 0.1f, 1.f, "%.2f");
						ImGui::InputFloat("translation.z", &objectList[selectedObject]->at(selectedObject).transform.translation.z, 0.1f, 1.f, "%.2f");
						//ImGui::InputFloat3("translation", placeTranslation);

						ImGui::Text("Rotation");
						ImGui::InputFloat("rotation.x", &objectList[selectedObject]->at(selectedObject).transform.rotation.x, oneDegree, tenDegrees, "%.2f");
						ImGui::InputFloat("rotation.y", &objectList[selectedObject]->at(selectedObject).transform.rotation.y, oneDegree, tenDegrees, "%.2f");
						ImGui::InputFloat("rotation.z", &objectList[selectedObject]->at(selectedObject).transform.rotation.z, oneDegree, tenDegrees, "%.2f");

						ImGui::Text("Scale");
						ImGui::InputFloat("scale.x", &objectList[selectedObject]->at(selectedObject).transform.scale.x, 0.1f, 1.f, "%.2f");
						ImGui::InputFloat("scale.y", &objectList[selectedObject]->at(selectedObject).transform.scale.y, 0.1f, 1.f, "%.2f");
						ImGui::InputFloat("scale.z", &objectList[selectedObject]->at(selectedObject).transform.scale.z, 0.1f, 1.f, "%.2f");
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Texture")) {
						if (objectList[selectedObject] == &objectManager->dynamicBuildObjects) {
							ImGui::InputText("texture location", materialLocation, MAX_LB_PATH);
							if (ImGui::SmallButton("Change texture")) {
								auto tempTextureID = EWETexture::addSmartModeTexture(eweDevice, materialLocation, EWETexture::tType_smart);
								if ((lastTextureID.first >= 0) && (lastTextureID.second)) {
									//objectManager->texturedBuildObjects[selectedObject] = std::move(objectManager->builderObjects[selectedObject]);
									//objectManager->builderObjects.erase(selectedObject);
									//objectList[selectedObject] = &objectManager->texturedBuildObjects;
									//std::string tempString = texLocation;

									materialHandler->removeByTransform(objectManager->dynamicBuildObjects[selectedObject].textureID, &objectManager->dynamicBuildObjects[selectedObject].transform);
									objectManager->dynamicBuildObjects[selectedObject].textureID = tempTextureID.second;

									if ((objectManager->dynamicBuildObjects[selectedObject].textureFlags & MaterialF_hasNormal) && ((tempTextureID.first & MaterialF_hasNormal) == 0)) {
										printf("before removing tangents \n");
										((BuilderModel*)objectManager->dynamicBuildObjects[selectedObject].model.get())->removeTangent();
										printf("after removing tangents \n");
									}
									else if (((objectManager->dynamicBuildObjects[selectedObject].textureFlags & MaterialF_hasNormal) == 0) && (tempTextureID.first & MaterialF_hasNormal)) {
										printf("before adding tangents \n");
										((BuilderModel*)objectManager->dynamicBuildObjects[selectedObject].model.get())->addTangent();
										printf("After adding tangents \n");
									}

									objectManager->dynamicBuildObjects[selectedObject].textureFlags = tempTextureID.first;
									materialHandler->addMaterialObject(tempTextureID.first, MOI_none, &objectManager->dynamicBuildObjects[selectedObject].transform, objectManager->dynamicBuildObjects[selectedObject].model.get(), tempTextureID.second, &objectManager->dynamicBuildObjects[selectedObject].drawable);

									quadWasAdded = true;

									lastTextureID = tempTextureID;
								}
								printf("adding texture to simple \n");
							}
							*
							ImGui::InputText("material location", materialLocation, texLocSize);
							if (ImGui::SmallButton("Add material")) {
								int32_t tempTextureID = EWETexture::addModeTexture(eweDevice, materialLocation, EWETexture::tType_material);
								if (tempTextureID >= 0) {
									objectManager->materialBuildObjects[selectedObject] = std::move(objectManager->builderObjects[selectedObject]);
									objectManager->builderObjects.erase(selectedObject);
									objectList[selectedObject] = &objectManager->materialBuildObjects;
									((BuilderModel*)objectManager->materialBuildObjects[selectedObject].model.get())->addTangent();

									objectManager->materialBuildObjects[selectedObject].textureID = tempTextureID;
									printf("adding material to simple \n");
								}


							}
							*
						}
						*
						else if (objectList[selectedObject] == &objectManager->texturedBuildObjects) {
							ImGui::InputText("changed texture location", texLocation, texLocSize);
							if (ImGui::SmallButton("Change Texture")) {
								objectManager->texturedBuildObjects[selectedObject].textureID = EWETexture::addModeTexture(eweDevice, texLocation);
							}
							ImGui::InputText("normal map location", materialLocation, texLocSize);

							if (ImGui::SmallButton("Add Normal Map")) {
								printf("beginning add normal map \n");
								int32_t tempTextureID = EWETexture::addModeTexture(eweDevice, materialLocation, EWETexture::tType_material);
								if (tempTextureID >= 0) {
									objectManager->materialBuildObjects[selectedObject] = std::move(objectManager->texturedBuildObjects[selectedObject]);
									objectManager->texturedBuildObjects.erase(selectedObject);
									objectList[selectedObject] = &objectManager->materialBuildObjects;
									printf("upscale vertices \n");
									((BuilderModel*)objectManager->materialBuildObjects[selectedObject].model.get())->addTangent();
									printf("after upscale vertices \n");

									objectManager->materialBuildObjects[selectedObject].textureID = tempTextureID;
								}
								//objectManager->materialBuildObjects[selectedObject].textureID = EWETexture::addModeTexture(eweDevice, materialLocation, EWETexture::tType_material);
								//EWETexture::createMaterialDescriptorAtBack();
								printf("adding material to textured \n");
							}
							*
							if (ImGui::SmallButton("Remove Texture")) {
								objectManager->builderObjects[selectedObject] = std::move(objectManager->texturedBuildObjects[selectedObject]);
								objectManager->texturedBuildObjects.erase(selectedObject);
								objectList[selectedObject] = &objectManager->builderObjects;
								objectManager->builderObjects[selectedObject].textureID = -1;
								printf("strippign texture \n");
							}
							*
						}
						else if (objectList[selectedObject] == &objectManager->materialBuildObjects) {
							ImGui::InputText("Change Material Location", materialLocation, texLocSize);
							if (ImGui::SmallButton("Change Material")) {
								int32_t tempTextureID = EWETexture::addModeTexture(eweDevice, materialLocation, EWETexture::tType_material);
								if (tempTextureID >= 0) {
									objectList[selectedObject]->at(selectedObject).textureID = tempTextureID;
								}
								else {
									printf("invalid change material location texture \n");
									//create an error box
								}
							}
							*
							if (ImGui::SmallButton("Remove Material")) {
								objectManager->builderObjects[selectedObject] = std::move(objectManager->materialBuildObjects[selectedObject]);
								objectManager->materialBuildObjects.erase(selectedObject);
								objectList[selectedObject] = &objectManager->builderObjects;
								objectManager->builderObjects[selectedObject].textureID = -1;
								((BuilderModel*)objectManager->builderObjects[selectedObject].model.get())->removeTangent();
								printf("stripping material \n");
							}
							
							if (ImGui::SmallButton("Add Transparency")) {
								objectManager->transparentBuildObjects[selectedObject] = std::move(objectManager->materialBuildObjects[selectedObject]);
								objectManager->materialBuildObjects.erase(selectedObject);
								objectList[selectedObject] = &objectManager->transparentBuildObjects;
								printf("adding transparency to material \n");
							}
							*
						}
						else if (objectList[selectedObject] == &objectManager->transparentBuildObjects) {
							ImGui::InputText("Change Material Location", materialLocation, texLocSize);
							if (ImGui::SmallButton("Change Material ")) {
								int32_t tempTextureID = EWETexture::addModeTexture(eweDevice, materialLocation, EWETexture::tType_material);
								if (tempTextureID >= 0) {
									objectList[selectedObject]->at(selectedObject).textureID = tempTextureID;
								}
								else {
									printf("invalid change material location texture \n");
									//create an error box
								}
							}
							if (ImGui::SmallButton("Remove Transparency")) {
								objectManager->materialBuildObjects[selectedObject] = std::move(objectManager->transparentBuildObjects[selectedObject]);
								objectManager->transparentBuildObjects.erase(selectedObject);
								objectList[selectedObject] = &objectManager->materialBuildObjects;
							}
						}
						*
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Texture UVs")) {
						if (newUVFocus) {
							changingSimpleVertices.clear();
							changingMateriaEWErtices.clear();
							changingVertexType = ((BuilderModel*)objectList[selectedObject]->at(selectedObject).model.get())->vType;
							if (changingVertexType == BuilderModel::vT_NT) {
								changingSimpleVertices = ((BuilderModel*)objectList[selectedObject]->at(selectedObject).model.get())->verticesNT;
							}
							else if (changingVertexType == BuilderModel::vT_tangent) {
								//printf("equal here \n");
								changingMateriaEWErtices = ((BuilderModel*)objectList[selectedObject]->at(selectedObject).model.get())->verticesTangent;
							}
							else {
								printf("vertex type not supported??? %d \n", changingVertexType);
							}
							//printf(" \n \n \n \n new uv focus %d \n \n \n \n", changingVertexType);
							newUVFocus = false;
						}
						ImGui::Text("Vertex UV Control");
						ImGui::InputFloat("Scale UV x", &uvScaleX, 1.f, .1f, "% .3f");
						ImGui::InputFloat("Scale UV y", &uvScaleY, 1.f, .1f, "% .3f");
						if (ImGui::SmallButton("Apply UV scale")) {
							for (int i = 0; i < changingSimpleVertices.size(); i++) {
								changingSimpleVertices[i].uv.x = uvScaleX * (changingSimpleVertices[i].uv.x > 0.f);
								changingSimpleVertices[i].uv.y = uvScaleY * (changingSimpleVertices[i].uv.y > 0.f);
							}
							for (int i = 0; i < changingMateriaEWErtices.size(); i++) {
								changingMateriaEWErtices[i].uv.x = uvScaleX * (changingMateriaEWErtices[i].uv.x > 0.f);
								changingMateriaEWErtices[i].uv.y = uvScaleY * (changingMateriaEWErtices[i].uv.y > 0.f);
							}
							//uvScaleX = 1.f;
							//uvScaleY = 1.f;
						}

						for (int i = 0; i < changingSimpleVertices.size(); i++) {
							sprintf(genericBuffer, "Vertex:%d", i);
							ImGui::Text(genericBuffer);
							sprintf(genericBuffer, "%d uv.x", i);
							ImGui::InputFloat(genericBuffer, &changingSimpleVertices[i].uv.x, 0.1f, 1.f, "% .2f");
							sprintf(genericBuffer, "%d uv.y", i);
							ImGui::InputFloat(genericBuffer, &changingSimpleVertices[i].uv.y, 0.1f, 1.f, "% .2f");
						}
						//ImGui::Text("Vertex UV Control");
						for (int i = 0; i < changingMateriaEWErtices.size(); i++) {
							sprintf(genericBuffer, "Vertex:%d", i);
							ImGui::Text(genericBuffer);
							sprintf(genericBuffer, "%d uv.x", i);
							ImGui::InputFloat(genericBuffer, &changingMateriaEWErtices[i].uv.x, 0.1f, 1.f, "% .2f");
							sprintf(genericBuffer, "%d uv.y", i);
							ImGui::InputFloat(genericBuffer, &changingMateriaEWErtices[i].uv.y, 0.1f, 1.f, "% .2f");


						}
						for (int i = 0; i < changingSimpleVertices.size(); i++) {
							if (!(changingSimpleVertices[i] == ((BuilderModel*)objectList[selectedObject]->at(selectedObject).model.get())->verticesNT[i])) {
								((BuilderModel*)objectList[selectedObject]->at(selectedObject).model.get())->replaceQuad(changingSimpleVertices);
								break;
							}
						}
						for (int i = 0; i < changingMateriaEWErtices.size(); i++) {
							if (!(changingMateriaEWErtices[i] == ((BuilderModel*)objectList[selectedObject]->at(selectedObject).model.get())->verticesTangent[i])) {
								((BuilderModel*)objectList[selectedObject]->at(selectedObject).model.get())->replaceQuad(changingMateriaEWErtices);
								break;
							}
						}
						//ImGui::InputFloat("vertex 1 UVs", &objectList[selectedObject]->at(selectedObject).
						//for(int i = 0; i < objectList[selectedObject]->at(selectedObject).model)
						//ImGui::InputFloat("")
						//BuilderModel::generateQuad(EWEDevice);
						ImGui::EndTabItem();
					}
					else {
						changingSimpleVertices.clear();
						changingMateriaEWErtices.clear();
						newUVFocus = true;
					}
				}
				else if (selectedObject == spawnObjectID) {
					if (ImGui::BeginTabItem("Transform_")) {
						ImGui::Text("Translation ");
						ImGui::InputFloat("translation.x ", &objectManager->spriteBuildObjects[0].transform.translation.x, 0.1f, 1.f, "%.2f");
						ImGui::InputFloat("translation.y ", &objectManager->spriteBuildObjects[0].transform.translation.y, 0.1f, 1.f, "%.2f");
						ImGui::InputFloat("translation.z ", &objectManager->spriteBuildObjects[0].transform.translation.z, 0.1f, 1.f, "%.2f");

						ImGui::Text("Rotation ");
						ImGui::InputFloat("rotation.x ", &objectManager->spriteBuildObjects[0].transform.rotation.x, oneDegree, tenDegrees, "%.2f");
						ImGui::InputFloat("rotation.y ", &objectManager->spriteBuildObjects[0].transform.rotation.y, oneDegree, tenDegrees, "%.2f");
						ImGui::InputFloat("rotation.z ", &objectManager->spriteBuildObjects[0].transform.rotation.z, oneDegree, tenDegrees, "%.2f");

						ImGui::EndTabItem();
					}
				}
				else if (selectedObject == gridObjectID) {
					if (ImGui::BeginTabItem("Transform__")) {
						ImGui::Text("Translation  ");
						ImGui::InputFloat("translation.x ", &objectManager->lBuilderObjects[0].transform.translation.x, 0.1f, 1.f, "%.2f");
						ImGui::InputFloat("translation.y ", &objectManager->lBuilderObjects[0].transform.translation.y, 0.1f, 1.f, "%.2f");
						ImGui::InputFloat("translation.z ", &objectManager->lBuilderObjects[0].transform.translation.z, 0.1f, 1.f, "%.2f");
						ImGui::EndTabItem();
					}
				}
				ImGui::EndTabBar();
			}
			ImGui::End();
		}
		
		if (saveLevelPrompt) {
			//printf("save level prompt true? \n");
			ImGui::Begin("Save Level Prompt \n");
			ImGui::InputText("Save Level Location", saveLocation, 128);
			if (ImGui::Button("Save Level")) {
				printf("Level Saved \n");
				float sunDir[3] = { lbo->sunlightDirection.x , lbo->sunlightDirection.y, lbo->sunlightDirection.z };
				float sunCol[3] = { lbo->sunlightColor.x, lbo->sunlightColor.y, lbo->sunlightColor.z };
				printf("before save? \n");
				LevelManager::saveLevel(saveLocation, objectManager, objectList, { lbo->sunlightDirection.x , lbo->sunlightDirection.y, lbo->sunlightDirection.z, lbo->sunlightColor.x, lbo->sunlightColor.y, lbo->sunlightColor.z, lbo->sunlightColor.w }, targetLocation, objectManager->spriteBuildObjects[0].transform, targetTimer);
				saveLevelPrompt = false;
				printf("after save \n");
				printf("saved object count? %d \n", objectCounter);
			}
			ImGui::End();
		}
		if (loadLevelPrompt) {
			ImGui::Begin("Load Level Prompt");
			ImGui::InputText("Load Level Location", saveLocation, 128);

			if (ImGui::Button("Load Level")) {
				wantsToLoadLevel = true;
				loadLevelPrompt = false;
			}

			ImGui::End();
		}
		if (targetControlScheme) {
			ImGui::Begin("Target Course Data");
			ImGui::Text("Target Count : %d", targetCount);

			ImGui::InputScalar("Target Timer", ImGuiDataType_U32, &targetTimer, &targetStepSmall, &targetStepLarge, "%u");
			//ImGui::InputInt("Timer", &targetTimer, 1, 10);
			//ImGui::InputInt
			ImGui::InputText("Target Texture Path", targetLocation, 128);
			if(ImGui::Button("Recalculate Targets")) {
				std::string bufferString = targetLocation;
				auto tempTargetID = EWETexture::addSmartModeTexture(eweDevice, bufferString, EWETexture::tType_smart);
				targetCount = 0;
				for (auto iter = objectList.begin(); iter != objectList.end(); iter++) {
					printf("target count %d \n", targetCount);
					if (iter->second->at(iter->first).textureID == tempTargetID.second) {
						targetCount++;
						iter->second->at(iter->first).isTarget = true;
					}
					else {
						iter->second->at(iter->first).isTarget = false;
					}
				}
			}
			ImGui::End();
		}

		ImGui::Begin("Camera Pos");
		ImGui::Text("x:%.2f y:%.2f z:%.2f", cameraObject->transform.translation.x, cameraObject->transform.translation.y, cameraObject->transform.translation.z);
		ImGui::SameLine();
		if(ImGui::SmallButton("Reset Camera Pos")) {
			cameraObject->transform.translation = glm::vec3{ 0.f };
		}
		ImGui::Text("x:%.2f y:%.2f z:%.2f", cameraObject->transform.rotation.x, cameraObject->transform.rotation.y, cameraObject->transform.rotation.z);
		ImGui::SameLine();
		if (ImGui::SmallButton("Reset Camera Rotation")) {
			cameraObject->transform.rotation = glm::vec3{ 0.f };
		}
		ImGui::End();
		bool hideGridOpen = true;
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoBackground;

		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkSize.x - 160, main_viewport->WorkSize.y - 20), ImGuiCond_None);
		ImGui::Begin("hide grid", &hideGridOpen, window_flags);
		//main_viewport->WorkSize.x -= 100;
		ImGui::Checkbox("grid visible", &objectManager->lBuilderObjects[0].drawable);
		ImGui::End();


	}

	void LevelBuilder::CleanUp() {
		objectManager->clearBuilders();
		objectList.clear();
	}
}
*/