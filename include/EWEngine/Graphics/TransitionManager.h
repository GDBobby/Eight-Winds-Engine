#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"

#include <vector>
#include <queue>
#include <functional>
#include <mutex>

namespace EWE {
	struct TransitionData {
		std::function<void()> callback{};
		SemaphoreData* waitSemaphore{nullptr};
		TransitionData() {}
		TransitionData(std::function<void()> callback, SemaphoreData* waitSemaphore) :
			callback{ callback },
			waitSemaphore{ waitSemaphore }
		{}
	};

	class TransitionManager {
	private:
		std::queue<TransitionData> transitions{};
		std::mutex mut{};
	public:
		void Add(std::function<void()> callback, SemaphoreData* waitSemaphore) {
			mut.lock();
			transitions.emplace(callback, waitSemaphore);
			mut.unlock();
		}
		bool Empty() {
			mut.lock();
			bool ret = transitions.size() == 0;
			mut.unlock();
			return ret;
		}
		TransitionData Pull() {
			mut.lock();
			TransitionData ret = transitions.front();
			transitions.pop();
			mut.unlock();
			return ret;
		}
	};
}