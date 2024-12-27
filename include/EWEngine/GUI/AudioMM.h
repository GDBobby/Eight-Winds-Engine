#pragma once
#include "MenuModule.h"
#include "EWEngine/Sound_Engine.h"
#include "EWEngine/SettingsJson.h"

namespace EWE {
	class AudioMM : public MenuModule {
	public:
		AudioMM(GLFWwindow* windowPtr);

		void ProcessClick(double xpos, double ypos) final;

		std::shared_ptr<SoundEngine> soundEngine;

		void initSoundSettings();

		void initVolumes(float master, float music, float sfx) {
			sliders.emplace_back();
			sliders[0].Init(glm::vec2{ 0.f, -.4f }, master);
			sliders.emplace_back();
			sliders[1].Init(glm::vec2{ 0.f, -.4f + .25f }, sfx);
			sliders.emplace_back();
			sliders[2].Init(glm::vec2{ 0.f, -.4f + (.25f * 2.f) }, music);

		}
		void resetSounds(float master, float music, float sfx);
	private:
		static AudioMM* audioPtr;
		GLFWwindow* windowPtr;

		double mousePosX = 0;
		double mousePosY = 0;


		static void AudioPosCallback(GLFWwindow* window, double xpos, double ypos);

		void audioSelected() {
			glfwSetCursorPosCallback(windowPtr, AudioPosCallback);
		}
		/*
			if (menuManagerPtr->currentMenuState == menu_audio_settings) {
				//menuManagerPtr->soundEngine->setVolume(menuManagerPtr->grabbedSlider - 1, menuManagerPtr->sliderVector[menuManagerPtr->grabbedSlider].slidePosition);
				printf("before setting volume on slider release, grabbed slider : %d  \n", menuManagerPtr->callbackSlider);
				SettingsJSON::tempSettings.setVolume(menuManagerPtr->callbackSlider, menuManagerPtr->menuMap[menu_audio_settings]->sliders[menuManagerPtr->callbackSlider].slidePosition);
				printf("after setting volume on slider rlease \n");
			}
		*/
	};
}