#include "EWEngine/GUI/MenuModule.h"

#include "EWEngine/Systems/Rendering/Pipelines/Dimension2.h"
#include "EWEngine/Graphics/Texture/Texture_Manager.h"

namespace EWE {

	std::unique_ptr<EWEModel> MenuModule::model2D;
	std::unique_ptr<EWEModel> MenuModule::nineUIModel;

	std::map<MenuTextureEnum, TextureDesc> MenuModule::textures{};

	std::queue<uint16_t> MenuModule::clickReturns{};

	void (*MenuModule::changeMenuStateFromMM)(uint8_t, unsigned char);

	void MenuModule::initTextures() {
		//these textures are deleted in EWETexture, when the program is cleaning itself up on close

		textures.emplace(MT_NineUI, Texture_Builder::createSimpleTexture( "UI/NineUI.png", true, false, VK_SHADER_STAGE_FRAGMENT_BIT));
		textures.emplace(MT_NineFade, Texture_Builder::createSimpleTexture( "UI/NineFade.png", true, false, VK_SHADER_STAGE_FRAGMENT_BIT));
		textures.emplace(MT_Slider, Texture_Builder::createSimpleTexture( "UI/clickyBox.png", true, false, VK_SHADER_STAGE_FRAGMENT_BIT));
		textures.emplace(MT_BracketButton, Texture_Builder::createSimpleTexture( "UI/bracketButton.png", true, false, VK_SHADER_STAGE_FRAGMENT_BIT));
		textures.emplace(MT_Bracket, Texture_Builder::createSimpleTexture( "UI/bracketSlide.png", true, false, VK_SHADER_STAGE_FRAGMENT_BIT));
		textures.emplace(MT_Unchecked, Texture_Builder::createSimpleTexture( "UI/unchecked.png", true, false, VK_SHADER_STAGE_FRAGMENT_BIT));
		textures.emplace(MT_Checked, Texture_Builder::createSimpleTexture( "UI/checked.png", true, false, VK_SHADER_STAGE_FRAGMENT_BIT));
		//textureIDs.emplace(MT_background, Texture_Builder::createSimpleTexture( "UI/mainPageBG.png"));
		textures.emplace(MT_Button, Texture_Builder::createSimpleTexture( "UI/ButtonUp.png", true, false, VK_SHADER_STAGE_FRAGMENT_BIT));
		
		textures.emplace(MT_Base, Texture_Builder::createSimpleTexture("UI/menuBase.png", true, false, VK_SHADER_STAGE_FRAGMENT_BIT));
		

		model2D = Basic_Model::generate2DQuad();
		nineUIModel = Basic_Model::generateNineUIQuad();
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

	void MenuModule::drawNewObjects() {
		Simple2DPushConstantData push{};
		if (checkBoxes.size() > 0) {
			push.color = glm::vec3{ 1.f };
			for (auto& object : checkBoxes) {
				if (object.isChecked) {
					Dimension2::bindTexture2DUI(textures[MT_Checked]);
					object.render(push);
				}
			}
			for (auto& object : checkBoxes) {
				if (!object.isChecked) {
					Dimension2::bindTexture2DUI(textures[MT_Unchecked]);
					object.render(push);
				}
			}
		}

		if (sliders.size() > 0) {
			push.color = glm::vec3(1.f);
			Dimension2::bindTexture2DUI(textures[MT_Slider]);

			for (auto& object : sliders) {
				object.render(push, 0);
			}
			Dimension2::bindTexture2DUI(textures[MT_BracketButton]);
			for (auto& object : sliders) {
				object.render(push, 1);
			}
			Dimension2::bindTexture2DUI(textures[MT_Bracket]);
			for (auto& object : sliders) {
				object.render(push, 2);
			}
		}

		if (controlBoxes.size() > 0) {
			//printf("before draw objects control boxes \n");
			push.color = glm::vec3(1.f);
			Dimension2::bindTexture2DUI(textures[MT_Button]);
			for (auto& object : controlBoxes) {
				object.render(push);
			}


			//printf("after control boxes \n");
		}
		if (images.size() > 0) {
			//not considering for texture ordering
			push.color = glm::vec3(1.f);
			for (int i = 0; i < images.size(); i++) {
				Dimension2::bindTexture2DUI(images[i].texture);
				push.scaleOffset = glm::vec4(images[i].transform.scale, images[i].transform.translation);
				Dimension2::pushAndDraw(push);
			}
		}
	}
	void MenuModule::drawNewNine() {
		NineUIPushConstantData push{};
		if (comboBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			Dimension2::bindTexture9(textures[MT_NineUI]);
			for (auto& object : comboBoxes) {
				object.render(push);
			}
		}

		if (dropBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			Dimension2::bindTexture9(textures[MT_NineUI]);
			for (auto& object : dropBoxes) {
				object.render(push);
			}
		}

		if (clickText.size() > 0) {

			push.offset.z = 1.f;
			push.offset.w = 1.f;
			push.color = glm::vec3{ .5f, .35f, .25f };
			Dimension2::bindTexture9(textures[MT_NineUI]);
			for (auto& object : clickText) {
				object.render(push);
			}
		}

		if (typeBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			Dimension2::bindTexture9(textures[MT_NineUI]);
			for (auto& object : typeBoxes) {
				object.render(push);
			}
		}
		if (controlBoxes.size() > 0) {
			Dimension2::bindTexture9(textures[MT_NineUI]);
			for (auto& object : controlBoxes) {
				object.render(push);
			}
			//printf("after nine ui control boxes \n");
		}

		if (menuBars.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			Dimension2::bindTexture9(textures[MT_NineUI]);
			for (auto& object : menuBars) {
				object.render(push, 0);
			}

			Dimension2::bindTexture9(textures[MT_NineFade]);
			for (auto& object : menuBars) {
				object.render(push, 1);
			}
		}

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