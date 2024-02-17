#include <EWEngine/EightWindsEngine.h>

#include "EWESample.h"

int main() {
	SettingsJSON::initializeSettings();
	EWE::EightWindsEngine ewEngine{"Eight Winds Engine Sample"};
	EWE::EWESample* eweSample = nullptr;
	auto loadPart2 = [&]() {
		ewEngine.finishLoading();
		eweSample = new EWE::EWESample(ewEngine);
		};
	std::thread loadThread(loadPart2);
	ewEngine.loadingScreen();
	loadThread.join();

	try {
		eweSample->mainThread();
	
	}
	catch (const std::exception& e) {
		std::ofstream file;
		file.open("errorLog.log");
		if (file.is_open()) {
			file << e.what() << "\n";
			file.close();
		}
		else {
			printf("try catch error \n \n");
			printf("string error : %s", e.what());
			printf("\n \n");

			//just blasting it on all channels lol
			std::cerr << e.what() << '\n';
		}
		system("pause");
		return EXIT_FAILURE;
	}
	delete eweSample;

	system("pause");
	return EXIT_SUCCESS;
}