#include <EWEngine/EightWindsEngine.h>
#include <EWEngine/Systems/ThreadPool.h>

#include "EWESample.h"

int main() {
	printf("current path : %s\n", std::filesystem::current_path().string().c_str());

	SettingsJSON::initializeSettings();

	EWE::ThreadPool::Construct();

	EWE::EightWindsEngine ewEngine{"Eight Winds Engine Sample"};
	EWE::EWESample* eweSample = nullptr;
	auto begin = std::chrono::high_resolution_clock::now();
	auto loadPart2 = [&]() {
		EWE::LoadingThreadTracker loadingThreadTracker{};
		ewEngine.FinishLoading();
		eweSample = Construct<EWE::EWESample>({ ewEngine, loadingThreadTracker });

		//i think the only way to guarantee this is called after i finish loading, if i add more threads, is to add flags for each loading segment.
		//the easiest way would be if i just counted the amount of requried threads, then required the completed thread count to equal that count before continuing
		while(!loadingThreadTracker.Finished()) { 
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}
		ewEngine.EndEngineLoadScreen();
	};
	EWE::ThreadPool::EnqueueVoid(loadPart2);

	//these threads are in a weird order
	//the loading screen won't finish until the threads are finished, but then we wait on threads. then we call the projects main loop,
	//and the main loop waits ont he loading screen
	ewEngine.LoadingScreen();

	printf("waiting for completion\n");
	EWE::ThreadPool::WaitForCompletion();
	printf("after wait completion\n");
	float duration = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - begin).count();
	printf("loading duration : %.5f\n", duration);

	eweSample->mainThread();
	Deconstruct(eweSample);
#if EWE_DEBUG
	system("pause");
#endif
	return EXIT_SUCCESS;
}