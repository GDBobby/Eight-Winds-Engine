#pragma once
#include "MenuModule.h"
#include "../SoundEngine.h"
#include "../SettingsJson.h"

namespace EWE {
	class AudioMM : public MenuModule {
	public:
		AudioMM(float screenWidth, float screenHeight, GLFWwindow* windowPtr);

		void processClick(double xpos, double ypos);

		std::shared_ptr<SoundEngine> soundEngine;

		void initSoundSettings();

		void initVolumes(float master, float music, float sfx, float screenWidth, float screenHeight) {
			sliders.emplace_back();
			sliders[0].Init(glm::vec2{ 0.f, -.4f }, screenHeight, screenWidth, master);
			sliders.emplace_back();
			sliders[1].Init(glm::vec2{ 0.f, -.4f + .25f }, screenHeight, screenWidth, sfx);
			sliders.emplace_back();
			sliders[2].Init(glm::vec2{ 0.f, -.4f + (.25f * 2.f) }, screenHeight, screenWidth, music);

		}
		void resetSounds(float master, float music, float sfx);
	private:
		static AudioMM* audioPtr;
		GLFWwindow* windowPtr;

		float screenWidth, screenHeight;
		int8_t currentGameState = 0; //0 for main menu, 1 for in game, both target and main

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