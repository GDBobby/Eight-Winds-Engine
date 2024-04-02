#include "EWEngine/GUI/UIHandler.h"


#define USING_IMGUI false
#if USING_IMGUI
#include "EWEngine/Graphics/imGuiHandler.h"
#endif

//#include <sstream>
//#include <iomanip>


namespace EWE {

	UIHandler* UIHandler::uiPointer;
	//GLFWwindow* UIHandler::windowPtr:

	UIHandler::UIHandler(std::pair<uint32_t, uint32_t> dimensions, GLFWwindow* window, TextOverlay* txtOverlay)
		: screenWidth{ static_cast<float>(dimensions.first) }, screenHeight{ static_cast<float>(dimensions.second) }, windowPtr{ window }, textOverlay{ std::shared_ptr<TextOverlay>(txtOverlay) } {
		printf("beg uiHandler construction, dimensions - %.1f:%.1f \n", screenWidth, screenHeight);

		//textOverlay = std::make_unique<TextOverlay>(eweDevice, EWESwapChain->getFrameBuffers(), EWESwapChain->width(), EWESwapChain->height(), EWESwapChain->getRenderPass(), 1.f);


		//MenuModule::changeMenuStateFromMM = changeMenuStateFromMM;
		printf("before sound engine \n");
		soundEngine = SoundEngine::GetSoundEngineInstance();
		printf("after sound engine \n");

		//printf("after imagers in ui handler \n");

		uiPointer = this;
		windowPtr = window;

		backgroundObject.transform2d.scale = glm::vec2{ 2.f };
		backgroundObject.color = { 0.01f, 0.01f, 0.01f };

	
		printf("end of ui construction \n");
	}

#if BENCHMARKING
	void UIHandler::Benchmarking(double time, double peakTime, double averageTime, double highTime, double avgLogic, bool benchmarkingGPU, float elapsedGPUMS, float averageGPU) {
		textOverlay->addDefaultText(time, peakTime, averageTime, highTime);
		if (avgLogic > 0.0f) {
			std::stringstream ss;
			ss.str("");
			ss << std::fixed << std::setprecision(4);
			ss << "average Logic Time: " << avgLogic;
			TextStruct passer{ ss.str(), 0.f, screenHeight - (100.f * textOverlay->scale), TA_left, 1.f };
			textOverlay->addText(passer);
			//addText(TextStruct{ ss.str(), 0.f, frameBufferHeight - (100.f * scale), TA_left, 1.f });
		}
		if (benchmarkingGPU) {
			std::stringstream ss;
			ss.str("");
			ss << std::fixed << std::setprecision(4);
			ss << "last GPU Time: " << elapsedGPUMS;
			TextStruct passer{ ss.str(), 0.f, screenHeight - (120.f * textOverlay->scale), TA_left, 1.f };
			textOverlay->addText(passer);
			//addText(TextStruct{ ss.str(), 0.f, frameBufferHeight - (100.f * scale), TA_left, 1.f });

			if (averageGPU > 0.f) {
				std::stringstream ss;
				ss.str("");
				ss << std::fixed << std::setprecision(4);
				ss << "average GPU Time: " << averageGPU;
				TextStruct passer{ ss.str(), 0.f, screenHeight - (140.f * textOverlay->scale), TA_left, 1.f };
				textOverlay->addText(passer);
			}
		}
	}
#endif


	void UIHandler::drawOverlayText(VkCommandBuffer commandBuffer, bool displayingRenderInfo) {

		if (overlay) {
			overlay->drawText();
		}
		//printf("returning draw menu main \n");
	}

	void UIHandler::type(int keyCode) {
		//std::cout << "typing? " << std::endl;
		if (keyCode == GLFW_KEY_ENTER) {
			selectedKey = -1;
			textSent = true;
		}
		else if (keyCode != GLFW_KEY_BACKSPACE) { typedString += glfwGetKeyName(keyCode, 0); }
		else if (typedString.size() > 0) {
			typedString.pop_back();
		}
	}

}