#include "EWEngine/MainWindow.h"

#include <stdexcept>

namespace EWE {
	//HWND MainWindow::hWnd;

	MainWindow::MainWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name }{
		//printf("main window constructor \n");
		initWindow();
	}
	MainWindow::MainWindow(std::string name) : windowName{ name } {
		std::pair<uint32_t, uint32_t> tempDim = SettingsJSON::settingsData.getDimensions();

		width = tempDim.first;
		height = tempDim.second;

		screenDimensions = SettingsJSON::settingsData.screenDimensions;
		windowMode = SettingsJSON::settingsData.windowMode;


		initWindow();
	}

	MainWindow::~MainWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	void MainWindow::initWindow() {
		glfwInit();

		int count;
		GLFWmonitor** monitors = glfwGetMonitors(&count);
#if EWE_DEBUG
		printf("monitor count : %d \n", count);
#endif
		//for (int i = 0; i < count; i++) {
		//	std::string monName = glfwGetMonitorName(monitors[i]);

		//	const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
		//	

		//	printf("monitor name:size - %s:(%.3f:%.3f) \n", monName.c_str(), mode->width, mode->height);
		//}

		if (screenDimensions == SettingsInfo::SD_size) {
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			if (mode != nullptr) {
				width = mode->width;
				height = mode->height;
				//printf("primary monitor - %d:%d \n", width, height);
				SettingsJSON::settingsData.setDimensions(width, height);
				SettingsJSON::tempSettings.setDimensions(width, height);
				SettingsJSON::saveToJsonFile();
			}
			else {
				printf("failed to find primary monitor \n");
				std::pair<uint32_t, uint32_t> tempDim = SettingsInfo::getScreenDimensions(SettingsInfo::SD_1280W);
				width = tempDim.first;
				height = tempDim.second;
				SettingsJSON::settingsData.screenDimensions = SettingsInfo::SD_1280W;
				SettingsJSON::tempSettings.screenDimensions = SettingsInfo::SD_1280W;
			}
		}



		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		
		if (SettingsJSON::settingsData.windowMode == SettingsInfo::WT_windowed) {
			glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		}
		else {
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		}

		printf("creating window with width:height  - %d:%d \n", width, height);
		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
		//hWnd = glfwGetWin32Window(window);
	}

	void MainWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface, bool gpuLogging) {
		//this->width = width;
		//this->height = height;
		//i think glfwcreatewindow does this, and chatGPT says it does so...
		/*
#if defined (_WIN32)
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = glfwGetWin32Window(window);
		createInfo.hinstance = GetModuleHandle(nullptr);
		if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create win32 surface");
		}
		return;
#endif
*/
		//glfwSetWindowSize(window, width, height);


		EWE_VK(glfwCreateWindowSurface, instance, window, nullptr, surface);
	}

	void MainWindow::frameBufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto mainWindow = reinterpret_cast<MainWindow*>(glfwGetWindowUserPointer(window));
		mainWindow->frameBufferResized = true;
		mainWindow->width = width;
		mainWindow->height = height;
	}

	void MainWindow::updateSettings() {
		if (screenDimensions != SettingsJSON::settingsData.screenDimensions) {
			printf("local - SettingsJSON - %d:%d \n", screenDimensions, SettingsJSON::settingsData.screenDimensions);
			screenDimensions = SettingsJSON::settingsData.screenDimensions;
			std::pair<uint32_t, uint32_t> tempDim = SettingsInfo::getScreenDimensions(screenDimensions);
			width = tempDim.first;
			height = tempDim.second;

			printf("updating dimensions in mainWindow - %d:%d\n", width, height);
			glfwSetWindowSize(window, width, height);
			frameBufferResized = true;
		}
		if (windowMode != SettingsJSON::settingsData.windowMode) {
			windowMode = SettingsJSON::settingsData.windowMode;
			printf("updating window mode in mainWindow : %d \n", windowMode);
			if (SettingsJSON::settingsData.windowMode == SettingsInfo::WT_windowed) {
				glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
				glfwGetWindowSize(window, &width, &height);
			}
			else {
				glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
				GLFWmonitor* currentMonitor = getCurrentMonitor(window);
				std::pair<uint32_t, uint32_t> tempDim = SettingsInfo::getScreenDimensions(screenDimensions);
				width = tempDim.first;
				height = tempDim.second;

				printf("updating dimensions in mainWindow - %d:%d\n", width, height);
				glfwSetWindowSize(window, width, height);

				int xPos, yPos;
				glfwGetMonitorPos(currentMonitor, &xPos, &yPos);
				glfwSetWindowPos(window, xPos, yPos);
			}
			frameBufferResized = true;
		}
	}

}