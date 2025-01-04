#include "EWEngine/GUI/MenuModule.h"

#include "EWEngine/Systems/Rendering/Pipelines/Dimension2.h"
#include "EWEngine/Graphics/Texture/Image_Manager.h"

namespace EWE {

	MenuModule::UIImageStruct::UIImageStruct(ImageID imgID, Transform2D& transform) : imgID{ imgID }, transform{ transform }, descriptor{ Image_Manager::CreateSimpleTexture(imgID, VK_SHADER_STAGE_FRAGMENT_BIT) } {}

	EWEModel* MenuModule::model2D;

	void (*MenuModule::ChangeMenuStateFromMM)(uint8_t, uint8_t);
	std::function<void(SceneKey)> MenuModule::ChangeSceneFromMM;

	void MenuModule::initTextures() {

		model2D = Basic_Model::Quad2D();
	}

	std::pair<UIComponentTypes, int16_t> MenuModule::CheckClick(double xpos, double ypos) {
		std::pair<UIComponentTypes, int16_t> returnVal = { UIT_none, -1 };
		if (selectedComboBox >= 0) {
#if EWE_DEBUG
			printf("checking click for selected combobox : %d \n", selectedComboBox);
#endif
			//one combo box is unraveled
			for (int i = 0; i < comboBoxes[selectedComboBox].comboOptions.size(); i++) {
				if (comboBoxes[selectedComboBox].comboOptions[i].Clicked(xpos, ypos)) {
					comboBoxes[selectedComboBox].SetSelection(i);
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
#if EWE_DEBUG
				printf("clicked a combo box? :%d - xpos:ypos %.1f:%.1f \n", i, xpos, ypos);
#endif

				selectedComboBox = i;
				return { UIT_Combobox, -1 };
			}
		}
		for (int i = 0; i < dropBoxes.size(); i++) {
			int8_t clickBuffer = dropBoxes[i].Clicked(xpos, ypos);
			if (clickBuffer > -2) {
#if EWE_DEBUG
				printf("clicked a drop box? :%d:%d - xpos:ypos %.1f:%.1f \n", i, clickBuffer, xpos, ypos);
#endif
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

#if EWE_DEBUG
		printf("before checking menu bar click \n");
#endif
		for (int i = 0; i < menuBars.size(); i++) {
			int16_t ret = menuBars[i].Clicked(xpos, ypos);
			if (ret > -1) {
				return { UIT_MenuBar, ret };
			}
		}
#if EWE_DEBUG
		printf("after checking menu bar click \n");
#endif

		return returnVal;
	}

	void MenuModule::DrawNewObjects() {
		Dimension2::BindDefaultDesc();
		Array2DPushConstantData push{};
		if (checkBoxes.size() > 0) {
			push.color = glm::vec3{ 1.f };
			for (auto& object : checkBoxes) {
				if (object.isChecked) {
					push.textureID = MT_Checked;
					object.Render(push);
				}
			}
			for (auto& object : checkBoxes) {
				if (!object.isChecked) {
					push.textureID = MT_Unchecked;
					object.Render(push);
				}
			}
		}

		if (sliders.size() > 0) {
			push.color = glm::vec3(1.f);
			push.textureID = MT_Slider;

			for (auto& object : sliders) {
				object.Render(push, 0);
			}
			push.textureID = MT_BracketButton;
			for (auto& object : sliders) {
				object.Render(push, 1);
			}
			push.textureID = MT_Bracket;
			for (auto& object : sliders) {
				object.Render(push, 2);
			}
		}

		if (controlBoxes.size() > 0) {
			//printf("before draw objects control boxes \n");
			push.color = glm::vec3(1.f);
			push.textureID = MT_Button;
			for (auto& object : controlBoxes) {
				object.Render(push);
			}


			//printf("after control boxes \n");
		}
	}
	void MenuModule::DrawNewNine() {

		Dimension2::BindDefaultDesc();
		Array2DPushConstantData push{};
		push.textureID = MT_NineUI;
		if (comboBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			for (auto& object : comboBoxes) {
				object.Render(push);
			}
		}

		if (dropBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			for (auto& object : dropBoxes) {
				object.Render(push);
			}
		}

		if (clickText.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			for (auto& object : clickText) {
				object.Render(push);
			}
		}

		if (typeBoxes.size() > 0) {
			push.color = glm::vec3{ .5f, .35f, .25f };
			for (auto& object : typeBoxes) {
				object.Render(push);
			}
		}
		if (controlBoxes.size() > 0) {
			for (auto& object : controlBoxes) {
				object.Render(push);
			}
			//printf("after nine ui control boxes \n");
		}

		if (menuBars.size() > 0) {
			push.textureID = MT_NineUI;
			push.color = glm::vec3{ .5f, .35f, .25f };
			for (auto& object : menuBars) {
				object.Render(push, 0);
			}
			push.textureID = MT_NineFade;
			for (auto& object : menuBars) {
				object.Render(push, 1);
			}
		}

	}

	void MenuModule::ResizeWindow(glm::vec2 resizeRatio) {
		//i think its just clickboxes?
		for (int i = 0; i < sliders.size(); i++) {
			sliders[i].ResizeWindow(); //ok why is this given a pointer to window size
		}
		for (int i = 0; i < comboBoxes.size(); i++) {
			comboBoxes[i].ResizeWindow(resizeRatio);
		}
		for (int i = 0; i < checkBoxes.size(); i++) {
			checkBoxes[i].ResizeWindow(resizeRatio);
		}
		for (int i = 0; i < labels.size(); i++) {
			labels[i].x *= resizeRatio.x;
			labels[i].y *= resizeRatio.y;
		}
		for (int i = 0; i < clickText.size(); i++) {
			clickText[i].ResizeWindow(resizeRatio);
		}
		for (int i = 0; i < dropBoxes.size(); i++) {
			dropBoxes[i].ResizeWindow(resizeRatio);
		}
		for (int i = 0; i < typeBoxes.size(); i++) {
			typeBoxes[i].ResizeWindow(resizeRatio);
		}
		for (int i = 0; i < controlBoxes.size(); i++) {
			controlBoxes[i].ResizeWindow(resizeRatio);
		}
		//printf("initializing menu bars \n");
		for (int i = 0; i < menuBars.size(); i++) {
			menuBars[i].init();
		}
		//printf("after initializing menu bars \n");
	}
	void MenuModule::DrawImages() {
		Single2DPushConstantData push;
		if (images.size() > 0) {
			//not considering for texture ordering
			push.color = glm::vec3(1.f);
			for (int i = 0; i < images.size(); i++) {
				Dimension2::BindSingleDescriptor(&images[i].descriptor);
				push.transform = images[i].transform.MatrixNoRotation();
				
				Dimension2::PushAndDraw(push);
			}
		}
	}

	void MenuModule::DrawText() {
		for (int i = 0; i < comboBoxes.size(); i++) {
			TextOverlay::StaticAddText(comboBoxes[i].activeOption.textStruct);
			if (comboBoxes[i].currentlyDropped) {
				for (int j = 0; j < comboBoxes[i].comboOptions.size(); j++) {
					TextOverlay::StaticAddText(comboBoxes[i].comboOptions[j].textStruct);
				}
			}
		}
		for (int i = 0; i < dropBoxes.size(); i++) {
			TextOverlay::StaticAddText(dropBoxes[i].dropper.textStruct);
			if (dropBoxes[i].currentlyDropped) {
				for (int j = 0; j < dropBoxes[i].dropOptions.size(); j++) {
					TextOverlay::StaticAddText(dropBoxes[i].dropOptions[j]);
				}
			}
		}
		for (int i = 0; i < checkBoxes.size(); i++) {
			TextOverlay::StaticAddText(checkBoxes[i].label);
		}
		for (int i = 0; i < labels.size(); i++) {
			TextOverlay::StaticAddText(labels[i]);
		}
		for (int i = 0; i < clickText.size(); i++) {
			TextOverlay::StaticAddText(clickText[i].textStruct);
		}
		for (int i = 0; i < typeBoxes.size(); i++) {
			TextOverlay::StaticAddText(typeBoxes[i].textStruct);
		}
		for (int i = 0; i < controlBoxes.size(); i++) {
			TextOverlay::StaticAddText(controlBoxes[i].label);
			for (int j = 0; j < controlBoxes[i].variableControls.size(); j++) {
				for (int k = 0; k < controlBoxes[i].variableControls[j].typeBoxes.size(); k++) {
					TextOverlay::StaticAddText(controlBoxes[i].variableControls[j].typeBoxes[k].textStruct);
				}
				TextOverlay::StaticAddText(controlBoxes[i].variableControls[j].dataLabel);
			}
		}
		//printf("drawing menu bar text \n");
		for (int i = 0; i < menuBars.size(); i++) {
			for (int j = 0; j < menuBars[i].dropBoxes.size(); j++) {
				TextOverlay::StaticAddText(menuBars[i].dropBoxes[j].dropper.textStruct);
				if (menuBars[i].dropBoxes[j].currentlyDropped) {
					for (int k = 0; k < menuBars[i].dropBoxes[j].dropOptions.size(); k++) {
						TextOverlay::StaticAddText(menuBars[i].dropBoxes[j].dropOptions[k]);
					}
				}
			}
		}
		//printf("aftter drawing menu bar text \n");
	}

	std::string MenuModule::GetInputName(int keyCode) {
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