#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"

#include <vector>
#include <queue>
#include <functional>
#include <mutex>

namespace EWE {
	struct TransitionData {
		std::function<void()> callback{};
		VkSemaphore waitSemaphore{ VK_NULL_HANDLE };
		VkSemaphore signalSemaphore{ VK_NULL_HANDLE };
		TransitionData() {}
		TransitionData(std::function<void()> callback, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore) :
			callback{ callback },
			waitSemaphore{ waitSemaphore }
		{}
	};

	class TransitionManager {
	private:
		std::queue<TransitionData> transitions{};
		std::mutex mut{};
	public:
		void AddTransition(std::function<void()> callback, VkSemaphore waitSemaphore) {
			mut.lock();
			transitions.emplace(callback, waitSemaphore);
			mut.unlock();
		}
		bool NotEmpty() {
			mut.lock();
			bool ret = transitions.size() != 0;
			mut.unlock();
			return ret;
		}
		TransitionData PullTransition() {
			mut.lock();
			TransitionData ret = transitions.front();
			transitions.pop();
			mut.unlock();
			return ret;
		}
	};
}