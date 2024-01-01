#include "EWEngine/GUI/MenuModule.h"
#include "EWEngine/graphics/Dimension2/Dimension2.h"

namespace EWE {

	std::unique_ptr<EWEModel> MenuModule::model2D;
	std::unique_ptr<EWEModel> MenuModule::nineUIModel;

	std::map<MenuTextureEnum, TextureID> MenuModule::textureIDs{};

	std::queue<uint16_t> MenuModule::clickReturns{};

	void (*MenuModule::changeMenuStateFromMM)(uint8_t, unsigned char);

	void MenuModule::initTextures(EWEDevice& eweDevice) {
		//these textures are deleted in EWETexture, when the program is cleaning itself up on close

		textureIDs.emplace(MT_NineUI, EWETexture::addUITexture(eweDevice, "UI/NineUI.png"));
		textureIDs.emplace(MT_NineFade, EWETexture::addUITexture(eweDevice, "UI/NineFade.png"));
		textureIDs.emplace(MT_Slider, EWETexture::addUITexture(eweDevice, "UI/clickyBox.png"));
		textureIDs.emplace(MT_BracketButton, EWETexture::addUITexture(eweDevice, "UI/bracketButton.png"));
		textureIDs.emplace(MT_Bracket, EWETexture::addUITexture(eweDevice, "UI/bracketSlide.png"));
		textureIDs.emplace(MT_Unchecked, EWETexture::addUITexture(eweDevice, "UI/unchecked.png"));
		textureIDs.emplace(MT_Checked, EWETexture::addUITexture(eweDevice, "UI/checked.png"));
		//textureIDs.emplace(MT_background, EWETexture::addUITexture(eweDevice, "UI/mainPageBG.png"));
		textureIDs.emplace(MT_Button, EWETexture::addUITexture(eweDevice, "UI/ButtonUp.png"));
		{
			textureIDs.emplace(MT_Base, EWETexture::addUITexture(eweDevice, "UI/menuBase.png"));
		}

		model2D = Basic_Model::generate2DQuad(eweDevice);
		nineUIModel = Basic_Model::generateNineUIQuad(eweDevice);
	}

	std::pair<UIComponentTypes, int16_t> MenuModule::checkClick(double xpos, double ypos) {
		std::pair<UIComponentTypes, int16_t> returnVal = { UIT_none, -1 };
		if (selectedComboBox >= 0) {
			printf("checking click for selected combobox : %d \n", selectedComboBox);
			//one combo box is unraveled
			for (int i = 0; i < comboBoxes[selectedComboBox].comboOptions.size(); i++) {
				if (comboBoxes[selectedComboBox].comboOptions[i].Clicked(xpos, ypos)) {
					comboBoxes[selectedComboBox].setSelection(i);
					//UIComp::TextToTransform(comboBoxes[selectedComboBox].activeOption.transform, comboBoxes[selectedComboBox].activeOption.textStruct, comboBoxes[selectedComboBox].activeOption.clickBox, screenWidth, screenHeight);
					return { UIT_Combobox, i };
				}
			}
			comboBoxes[selectedComboBox].currentlyDropped = false;
			selectedComboBox = -1;
			return returnVal;
		}
		for (int i = 0; i < comboBoxes.size(); i++) {
			//int16_t comboClick = comboBoxes[i]; //not finished yet
			//return comboClick + sliders.size() * 3;
			if (comboBoxes[i].Clicked(xpos, ypos)) {
				printf("clicked a combo box? :%d - xpos:ypos %.1f:%.1f \n", i, xpos, ypos);

				selectedComboBox = i;
				return { UIT_Combobox, -1 };
			}
		}
		for (int i = 0; i < dropBoxes.size(); i++) {
			int8_t clickBuffer = dropBoxes[i].Clicked(xpos, ypos);
			if (clickBuffer > -2) {
				printf("clicked a drop box? :%d:%d - xpos:ypos %.1f:%.1f \n", i, clickBuffer, xpos, ypos);
				selectedDropBox = i;
				return { UIT_Dropbox, -1 };
			}
		}

		for (int i = 0; i < sliders.size(); i++) {
			int8_t sliderClick = sliders[i].Clicked(xpos, ypos);
			//printf("checking slider %d:%d \n", i, sliderClick);
			//printf("clickboxes, 0 - x(%d:%d) : y(%d:%d) \n", sliders[i].click[0].x, sliders[i].click[0].z, sliders[i].click[0].y, sliders[i].click[0].w);
			//printf("\t  1 - x(%d:%d) : y(%d:%d) \n", sliders[i].click[1].x, sliders[i].click[1].z, sliders[i].click[1].y, sliders[i].click[1].w);
			//printf("\t  2 - x(%d:%d) : y(%d:%d) \n", sliders[i].click[2].x, sliders[i].click[2].z, sliders[i].click[2].y, sliders[i].click[2].w);
			if (sliderClick != -1) {
				return { UIT_Slider, i * 3 + sliderClick };
			}
		}
		for (int i = 0; i < checkBoxes.size(); i++) {
			//printf("checking check box %d \n", i);
			//printf("click bounds - x(%d:%d) : y(%d:%d) \n",
				//checkBoxes[i].clickBox.x, checkBoxes[i].clickBox.z,
				//checkBoxes[i].clickBox.y, checkBoxes[i].clickBox.w
			//);
			if (checkBoxes[i].Clicked(xpos, ypos)) {
				return { UIT_Checkbox, i };
			}
		}
		for (int i = 0; i < clickText.size(); i++) {
			if (clickText[i].Clicked(xpos, ypos)) {
				return { UIT_ClickTextBox, i };
			}
		}
		for (int i = 0; i < typeBoxes.size(); i++) {
			if (typeBoxes[i].Clicked(xpos, ypos)) {
				return { UIT_TypeBox, i };
			}
		}
		for (int i = 0; i < controlBoxes.size(); i++) {

			if (controlBoxes[i].Clicked(xpos, ypos)) { //come back to this
				return { UIT_VariableTypeBox, i };
			}
		}

		printf("before checking menu bar click \n");
		for (int i = 0; i < menuBars.size(); i++) {
			int16_t ret = menuBars[i].Clicked(xpos, ypos);
			if (ret > -1) {
				return { UIT_MenuBar, ret };
			}
		}
		printf("after checking menu bar click \n");

		return returnVal;
	}
	void MenuModule::drawObjects(FrameInfo2D& frameInfo) {

		//printf("drawing objects in menu module \n");
		/*
		* go front to back
			Combo,
			Slider,
			BracketButton,
			Bracket,
			ClickBox,
			unChecked,
			CheckedBox,
		*/
		Simple2DPushConstantData push{};

		auto& pl2d = PipelineManager::pipeLayouts[PL_2d];
		model2D->bind(frameInfo.cmdIndexPair.first);

		if (checkBoxes.size() > 0) {
			push.color = glm::vec3{ 1.f };
			for (int i = 0; i < checkBoxes.size(); i++) {
				if (checkBoxes[i].isChecked) {
					if (frameInfo.currentlyBindedTexture != textureIDs[MT_Checked]) {
						vkCmdBindDescriptorSets(
							frameInfo.cmdIndexPair.first,
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							pl2d,
							0, 1,
							//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
							EWETexture::getUIDescriptorSets(textureIDs[MT_Checked], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
							0, nullptr
						);
						frameInfo.currentlyBindedTexture = textureIDs[MT_Checked];
					}
					push.scaleOffset = glm::vec4(checkBoxes[i].button.transform.scale, checkBoxes[i].button.transform.translation);
					vkCmdPushConstants(frameInfo.cmdIndexPair.first, pl2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
					model2D->draw(frameInfo.cmdIndexPair.first);
				}
			}
			for (int i = 0; i < checkBoxes.size(); i++) {
				if (!checkBoxes[i].isChecked) {
					if (frameInfo.currentlyBindedTexture != textureIDs[MT_Unchecked]) {
						vkCmdBindDescriptorSets(
							frameInfo.cmdIndexPair.first,
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							pl2d,
							0, 1,
							//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
							EWETexture::getUIDescriptorSets(textureIDs[MT_Unchecked], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
							0, nullptr
						);
						frameInfo.currentlyBindedTexture = textureIDs[MT_Unchecked];
					}
					push.scaleOffset = glm::vec4(checkBoxes[i].button.transform.scale, checkBoxes[i].button.transform.translation);
					vkCmdPushConstants(frameInfo.cmdIndexPair.first, pl2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
					model2D->draw(frameInfo.cmdIndexPair.first);
				}
			}
		}

		if (sliders.size() > 0) {
			push.color = glm::vec3(1.f);
			if (frameInfo.currentlyBindedTexture != textureIDs[MT_Slider]) {
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					pl2d,
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_Slider], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				frameInfo.currentlyBindedTexture = textureIDs[MT_Slider];
			}

			for (int i = 0; i < sliders.size(); i++) {
				push.scaleOffset = glm::vec4(sliders[i].slider.mat2(), sliders[i].slider.translation);
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, pl2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
				model2D->draw(frameInfo.cmdIndexPair.first);
			}
			if (frameInfo.currentlyBindedTexture != textureIDs[MT_BracketButton]) {
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					pl2d,
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_BracketButton], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				frameInfo.currentlyBindedTexture = textureIDs[MT_BracketButton];
			}
			for (int i = 0; i < sliders.size(); i++) {
				push.scaleOffset = glm::vec4(sliders[i].bracketButtons.first.mat2(), sliders[i].bracketButtons.first.translation);
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, pl2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
				model2D->draw(frameInfo.cmdIndexPair.first);
				push.scaleOffset = glm::vec4(sliders[i].bracketButtons.second.mat2(), sliders[i].bracketButtons.second.translation);
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, pl2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
				model2D->draw(frameInfo.cmdIndexPair.first);
			}
			if (frameInfo.currentlyBindedTexture != textureIDs[MT_Bracket]) {
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					pl2d,
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_Bracket], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				frameInfo.currentlyBindedTexture = textureIDs[MT_Bracket];
			}
			for (int i = 0; i < sliders.size(); i++) {
				push.scaleOffset = glm::vec4(sliders[i].bracket.mat2(), sliders[i].bracket.translation);
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, pl2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
				model2D->draw(frameInfo.cmdIndexPair.first);
			}
		}

		if (controlBoxes.size() > 0) {
			//printf("before draw objects control boxes \n");
			push.color = glm::vec3(1.f);
			if (frameInfo.currentlyBindedTexture != textureIDs[MT_Button]) {
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					pl2d,
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_Button], frameInfo.cmdIndexPair.second), //need to change this texture to a button texture, + -
					0, nullptr
				);
				frameInfo.currentlyBindedTexture = textureIDs[MT_Button];
			}
			for (int i = 0; i < controlBoxes.size(); i++) {
				for (int j = 0; j < controlBoxes[i].variableControls.size(); j++) {
					//printf("variable controls size, j - %d:%d \n", controlBoxes[i].variableControls.size(), j);
					for (int k = 0; k < controlBoxes[i].variableControls[j].buttons.size(); k++) {
						push.scaleOffset = glm::vec4(controlBoxes[i].variableControls[j].buttons[k].first.transform.scale, controlBoxes[i].variableControls[j].buttons[k].first.transform.translation);
						vkCmdPushConstants(frameInfo.cmdIndexPair.first, pl2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
						model2D->draw(frameInfo.cmdIndexPair.first);
						push.scaleOffset = glm::vec4(controlBoxes[i].variableControls[j].buttons[k].second.transform.scale, controlBoxes[i].variableControls[j].buttons[k].second.transform.translation);
						vkCmdPushConstants(frameInfo.cmdIndexPair.first, pl2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
						model2D->draw(frameInfo.cmdIndexPair.first);
					}
				}
			}
			//printf("after control boxes \n");
		}

		if (images.size() > 0) {
			//not considering for texture ordering
			push.color = glm::vec3(1.f);
			for (int i = 0; i < images.size(); i++) {
				if (frameInfo.currentlyBindedTexture != images[i].textureID) {
					vkCmdBindDescriptorSets(
						frameInfo.cmdIndexPair.first,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						pl2d,
						0, 1,
						//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
						EWETexture::getUIDescriptorSets(images[i].textureID, frameInfo.cmdIndexPair.second), //need to change this texture to a button texture, + -
						0, nullptr
					);
					frameInfo.currentlyBindedTexture = images[i].textureID;
				}
				push.scaleOffset = glm::vec4(images[i].transform.scale, images[i].transform.translation);
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, pl2d, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Simple2DPushConstantData), &push);
				model2D->draw(frameInfo.cmdIndexPair.first);
			}
		}
	}

	void MenuModule::drawNewObjects() {
		Simple2DPushConstantData push{};
		if (checkBoxes.size() > 0) {
			push.color = glm::vec3{ 1.f };
			for (auto& object : checkBoxes) {
				if (object.isChecked) {
					Dimension2::bindTexture2DUI(textureIDs[MT_Checked]);
					object.render(push);
				}
			}
			for (auto& object : checkBoxes) {
				if (!object.isChecked) {
					Dimension2::bindTexture2DUI(textureIDs[MT_Unchecked]);
					object.render(push);
				}
			}
		}

		if (sliders.size() > 0) {
			push.color = glm::vec3(1.f);
			Dimension2::bindTexture2DUI(textureIDs[MT_Slider]);

			for (auto& object : sliders) {
				object.render(push, 0);
			}
			Dimension2::bindTexture2DUI(textureIDs[MT_BracketButton]);
			for (auto& object : sliders) {
				object.render(push, 1);
			}
			Dimension2::bindTexture2DUI(textureIDs[MT_Bracket]);
			for (auto& object : sliders) {
				object.render(push, 2);
			}
		}

		if (controlBoxes.size() > 0) {
			//printf("before draw objects control boxes \n");
			push.color = glm::vec3(1.f);
			Dimension2::bindTexture2DUI(textureIDs[MT_Button]);
			for (auto& object : controlBoxes) {
				object.render(push);
			}


			//printf("after control boxes \n");
		}
		if (images.size() > 0) {
			//not considering for texture ordering
			push.color = glm::vec3(1.f);
			for (int i = 0; i < images.size(); i++) {
				Dimension2::bindTexture2DUI(images[i].textureID);
				push.scaleOffset = glm::vec4(images[i].transform.scale, images[i].transform.translation);
				Dimension2::pushAndDraw(push);
			}
		}
	}
	void MenuModule::drawNewNine() {
		NineUIPushConstantData push{};
		if (comboBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			Dimension2::bindTexture9(textureIDs[MT_NineUI]);
			for (auto& object : comboBoxes) {
				object.render(push);
			}
		}

		if (dropBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			Dimension2::bindTexture9(textureIDs[MT_NineUI]);
			for (auto& object : dropBoxes) {
				object.render(push);
			}
		}

		if (clickText.size() > 0) {

			push.offset.z = 1.f;
			push.offset.w = 1.f;
			push.color = glm::vec3{ .5f, .35f, .25f };
			Dimension2::bindTexture9(textureIDs[MT_NineUI]);
			for (auto& object : clickText) {
				object.render(push);
			}
		}

		if (typeBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			Dimension2::bindTexture9(textureIDs[MT_NineUI]);
			for (auto& object : typeBoxes) {
				object.render(push);
			}
		}
		if (controlBoxes.size() > 0) {
			Dimension2::bindTexture9(textureIDs[MT_NineUI]);
			for (auto& object : controlBoxes) {
				object.render(push);
			}
			//printf("after nine ui control boxes \n");
		}

		if (menuBars.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			Dimension2::bindTexture9(textureIDs[MT_NineUI]);
			for (auto& object : menuBars) {
				object.render(push, 0);
			}

			Dimension2::bindTexture9(textureIDs[MT_NineFade]);
			for (auto& object : menuBars) {
				object.render(push, 1);
			}
		}

	}
	void MenuModule::drawNineUI(FrameInfo2D& frameInfo) {
		//printf("beginning nine ui \n");
		nineUIModel->bind(frameInfo.cmdIndexPair.first);
		NineUIPushConstantData push{};
		if (comboBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			if (frameInfo.currentlyBindedTexture != MT_NineUI) {
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineUI], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				frameInfo.currentlyBindedTexture = MT_NineUI;
			}
			for (int i = 0; i < comboBoxes.size(); i++) {
				push.offset = glm::vec4(comboBoxes[i].activeOption.transform.translation, 1.f, 1.f);
				//need color array
				push.scale = comboBoxes[i].activeOption.transform.scale;
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(frameInfo.cmdIndexPair.first);
				if (comboBoxes[i].currentlyDropped) {
					for (int j = 0; j < comboBoxes[i].comboOptions.size(); j++) {
						push.offset = glm::vec4(comboBoxes[i].comboOptions[j].transform.translation, 1.f, 1.f);
						//need color array
						push.scale = comboBoxes[i].comboOptions[j].transform.scale;
						if (j == comboBoxes[i].currentlySelected) {
							push.color = glm::vec3{ .4f, .4f, 1.f };
							vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
							nineUIModel->draw(frameInfo.cmdIndexPair.first);
							push.color = glm::vec3{ .5f, .35f, .25f };
						}
						else {
							vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
							nineUIModel->draw(frameInfo.cmdIndexPair.first);
						}
					}
				}
			}
		}
		if (dropBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			if (frameInfo.currentlyBindedTexture != MT_NineUI) {
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineUI], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				frameInfo.currentlyBindedTexture = MT_NineUI;
			}
			for (int i = 0; i < dropBoxes.size(); i++) {
				push.offset = glm::vec4(dropBoxes[i].dropper.transform.translation, 1.f, 1.f);
				//need color array
				if (dropBoxes[i].currentlyDropped) {
					push.color = glm::vec3{ .75f, .35f, .25f };
				}
				push.scale = dropBoxes[i].dropper.transform.scale;
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(frameInfo.cmdIndexPair.first);
				push.color = glm::vec3{ .5f, .35f, .25f };
				if (dropBoxes[i].currentlyDropped) {
					push.offset = glm::vec4(dropBoxes[i].dropBackground.translation, 0.5f, 1.f);
					push.scale = dropBoxes[i].dropBackground.scale;
					vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
					nineUIModel->draw(frameInfo.cmdIndexPair.first);
				}
			}
		}

		if (clickText.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			if (frameInfo.currentlyBindedTexture != textureIDs[MT_NineUI]) {
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineUI], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				frameInfo.currentlyBindedTexture = textureIDs[MT_NineUI];
			}
			for (int i = 0; i < clickText.size(); i++) {
				push.offset = glm::vec4(clickText[i].transform.translation, 1.f, 1.f);
				//need color array
				push.scale = clickText[i].transform.scale;
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(frameInfo.cmdIndexPair.first);
				//printf("drawing click text \n");
			}
		}

		if (typeBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			if (frameInfo.currentlyBindedTexture != textureIDs[MT_NineUI]) {
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineUI], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				frameInfo.currentlyBindedTexture = textureIDs[MT_NineUI];
			}
			for (int i = 0; i < typeBoxes.size(); i++) {
				push.offset = glm::vec4(typeBoxes[i].transform.translation, 1.f, 1.f);
				//need color array
				push.scale = typeBoxes[i].transform.scale;
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(frameInfo.cmdIndexPair.first);
				//printf("drawing click text \n");
			}
		}

		//printf("before nineui control box \n");
		if (controlBoxes.size() > 0) {
			if (frameInfo.currentlyBindedTexture != textureIDs[MT_NineUI]) {
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineUI], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				frameInfo.currentlyBindedTexture = textureIDs[MT_NineUI];
			}
			for (int i = 0; i < controlBoxes.size(); i++) {
				for (int j = 0; j < controlBoxes[i].variableControls.size(); j++) {
					for (int k = 0; k < controlBoxes[i].variableControls[j].typeBoxes.size(); k++) {
						if (controlBoxes[i].variableControls[j].isSelected(k)) {
							push.color = glm::vec3{ .6f, .5f, .4f };
						}
						else {
							push.color = glm::vec3{ .5f, .35f, .25f };
						}
						push.offset = glm::vec4(controlBoxes[i].variableControls[j].typeBoxes[k].transform.translation, 1.f, 1.f);
						push.scale = controlBoxes[i].variableControls[j].typeBoxes[k].transform.scale;
						vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
						nineUIModel->draw(frameInfo.cmdIndexPair.first);
					}
				}
				push.color = glm::vec3{ .3f, .25f, .15f };
				push.offset = glm::vec4(controlBoxes[i].transform.translation, 1.f, 1.f);
				push.scale = controlBoxes[i].transform.scale;
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(frameInfo.cmdIndexPair.first);
			}
			//printf("after nine ui control boxes \n");
		}
		//printf("before drawing menu bars \n");
		if (menuBars.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			if (frameInfo.currentlyBindedTexture != textureIDs[MT_NineUI]) {
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineUI], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				frameInfo.currentlyBindedTexture = textureIDs[MT_NineUI];
			}
			for (int i = 0; i < menuBars.size(); i++) {

				if (menuBars[i].dropBoxes.size() > 0) { //drawing these here instead of tumblingg these with the earlier drop boxes because i dont want to draw the dropper box
					push.color = glm::vec3{ .5f, .35f, .25f };
					for (int j = 0; j < menuBars[i].dropBoxes.size(); j++) {
						push.color = glm::vec3{ .5f, .35f, .25f };
						if (menuBars[i].dropBoxes[j].currentlyDropped) {
							push.offset = glm::vec4(menuBars[i].dropBoxes[j].dropBackground.translation, 0.5f, 1.f);
							push.scale = menuBars[i].dropBoxes[j].dropBackground.scale;
							vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
							nineUIModel->draw(frameInfo.cmdIndexPair.first);
						}
					}
				}
			}

			if (frameInfo.currentlyBindedTexture != textureIDs[MT_NineFade]) {
				vkCmdBindDescriptorSets(
					frameInfo.cmdIndexPair.first,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					PipelineManager::pipeLayouts[PL_nineUI],
					0, 1,
					//&EWETexture::globalDescriptorSets[frameInfo.frameInfo.cmdIndexPair.second + 2],
					EWETexture::getUIDescriptorSets(textureIDs[MT_NineFade], frameInfo.cmdIndexPair.second), //i % 4 only works currently because 4 is same as 0, and the rest are lined up 
					0, nullptr
				);
				frameInfo.currentlyBindedTexture = textureIDs[MT_NineFade];
			}
			for (int i = 0; i < menuBars.size(); i++) {

				push.color = glm::vec3{ .86f, .5f, .5f };
				push.offset = glm::vec4(menuBars[i].transform.translation, 0.1f, 1.f);
				push.scale = menuBars[i].transform.scale;
				vkCmdPushConstants(frameInfo.cmdIndexPair.first, PipelineManager::pipeLayouts[PL_nineUI], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(NineUIPushConstantData), &push);
				nineUIModel->draw(frameInfo.cmdIndexPair.first);
			}
		}
		//printf("After drawing menu bars \n");
		frameInfo.currentlyBindedTexture = -1;
	}

	void MenuModule::resizeWindow(float rszWidth, float oldWidth, float rszHeight, float oldHeight) {
		//i think its just clickboxes?
		for (int i = 0; i < sliders.size(); i++) {
			sliders[i].resizeWindow(rszWidth, rszHeight); //ok why is this given a pointer to window size
		}
		for (int i = 0; i < comboBoxes.size(); i++) {
			comboBoxes[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		for (int i = 0; i < checkBoxes.size(); i++) {
			checkBoxes[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		for (int i = 0; i < labels.size(); i++) {
			labels[i].x *= rszWidth / oldWidth;
			labels[i].y *= rszHeight / oldHeight;
		}
		for (int i = 0; i < clickText.size(); i++) {
			clickText[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		for (int i = 0; i < dropBoxes.size(); i++) {
			dropBoxes[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		for (int i = 0; i < typeBoxes.size(); i++) {
			typeBoxes[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		for (int i = 0; i < controlBoxes.size(); i++) {
			controlBoxes[i].resizeWindow(rszWidth, oldWidth, rszHeight, oldHeight);
		}
		//printf("initializing menu bars \n");
		for (int i = 0; i < menuBars.size(); i++) {
			menuBars[i].init(rszWidth, rszHeight);
		}
		//printf("after initializing menu bars \n");
	}

	void MenuModule::drawText(TextOverlay* textOverlay) {
		for (int i = 0; i < comboBoxes.size(); i++) {
			textOverlay->addText(comboBoxes[i].activeOption.textStruct);
			if (comboBoxes[i].currentlyDropped) {
				for (int j = 0; j < comboBoxes[i].comboOptions.size(); j++) {
					textOverlay->addText(comboBoxes[i].comboOptions[j].textStruct);
				}
			}
		}
		for (int i = 0; i < dropBoxes.size(); i++) {
			textOverlay->addText(dropBoxes[i].dropper.textStruct);
			if (dropBoxes[i].currentlyDropped) {
				for (int j = 0; j < dropBoxes[i].dropOptions.size(); j++) {
					textOverlay->addText(dropBoxes[i].dropOptions[j]);
				}
			}
		}
		for (int i = 0; i < checkBoxes.size(); i++) {
			textOverlay->addText(checkBoxes[i].label);
		}
		for (int i = 0; i < labels.size(); i++) {
			textOverlay->addText(labels[i]);
		}
		for (int i = 0; i < clickText.size(); i++) {
			textOverlay->addText(clickText[i].textStruct);
		}
		for (int i = 0; i < typeBoxes.size(); i++) {
			textOverlay->addText(typeBoxes[i].textStruct);
		}
		for (int i = 0; i < controlBoxes.size(); i++) {
			textOverlay->addText(controlBoxes[i].label);
			for (int j = 0; j < controlBoxes[i].variableControls.size(); j++) {
				for (int k = 0; k < controlBoxes[i].variableControls[j].typeBoxes.size(); k++) {
					textOverlay->addText(controlBoxes[i].variableControls[j].typeBoxes[k].textStruct);
				}
				textOverlay->addText(controlBoxes[i].variableControls[j].dataLabel);
			}
		}
		//printf("drawing menu bar text \n");
		for (int i = 0; i < menuBars.size(); i++) {
			for (int j = 0; j < menuBars[i].dropBoxes.size(); j++) {
				textOverlay->addText(menuBars[i].dropBoxes[j].dropper.textStruct);
				if (menuBars[i].dropBoxes[j].currentlyDropped) {
					for (int k = 0; k < menuBars[i].dropBoxes[j].dropOptions.size(); k++) {
						textOverlay->addText(menuBars[i].dropBoxes[j].dropOptions[k]);
					}
				}
			}
		}
		//printf("aftter drawing menu bar text \n");
	}

	std::string MenuModule::getInputName(int keyCode) {
		//std::cout << "get mouse? : " << keyCode << std::endl;
		//std::cout << "get mouse key : " << keyCode << std::endl;
		if (keyCode == 0) {
			return "LMB";
		}
		else if (keyCode == 1) {
			return "RMB";
		}
		else if (keyCode == 2) {
			return "MMB";
		}
		else if (keyCode <= 10) {
			std::string tempString = "MB";
			tempString += std::to_string(keyCode - 2);
			// std::cout << "other mouse key : " << tempString << std::endl;
			printf("other mouse key : %s \n", tempString.c_str());
			return tempString;
		}
		else if (keyCode == GLFW_KEY_SPACE) {
			return "SPACE";
		}
		else if (keyCode == GLFW_KEY_LEFT_SHIFT) {
			return "L Shift";
		}
		else if (keyCode == GLFW_KEY_LEFT_CONTROL) {
			return "L Ctrl";
		}
		else if (keyCode == GLFW_KEY_RIGHT_CONTROL) {
			return "R Ctrl";
		}
		else if (keyCode == GLFW_KEY_RIGHT_SHIFT) {
			return "R Shift";
		}
		else if (keyCode == GLFW_KEY_TAB) {
			return "Tab";
		}
		std::string tempString;
		if (((keyCode >= GLFW_KEY_0) && (keyCode <= GLFW_KEY_9)) || ((keyCode >= GLFW_KEY_A) && (keyCode <= GLFW_KEY_Z))) {
			tempString = glfwGetKeyName(keyCode, 0);
		}
		else {
			tempString = "unknown:";
			tempString += std::to_string(keyCode);
		}
		//std::cout << "standard key?" << std::endl;
		//char tempChar = glfwGetKeyName(keyCode, 0);
		//std::cout << "before getkeyname" << std::endl;

		//std::cout << "end of get mouse" << std::endl;
		return tempString;
	};
}