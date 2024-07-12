#include <EWEngine/EightWindsEngine.h>
#include <EWEngine/Systems/ThreadPool.h>

#include "EWESample.h"

int main() {
	SettingsJSON::initializeSettings();

	EWE::ThreadPool::Construct();

	EWE::EightWindsEngine ewEngine{"Eight Winds Engine Sample"};
	EWE::EWESample* eweSample = nullptr;
	auto loadPart2 = [&]() {
		ewEngine.FinishLoading();

		eweSample = new EWE::EWESample(ewEngine);
		};
	//EWE::ThreadPool::EnqueueVoid(loadPart2);
	ewEngine.LoadingScreen();
	EWE::ThreadPool::WaitForCompletion();

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