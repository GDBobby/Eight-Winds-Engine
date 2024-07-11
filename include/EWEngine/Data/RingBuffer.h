#pragma once

#include <memory>

//i removed this before and im not quite sure why, but it's not the correct solution for the issue im having anyways

namespace EWE {
	template <class T>
	class Ring_Buffer {
	public:
		explicit Ring_Buffer(size_t size) : buff{ new T[size] }, max_size_{ size } {
			// empty 
		}
		~Ring_Buffer() {
			delete[] buff;
		}

		void Put(T item) {
			std::lock_guard<std::mutex> lock(mut);

			buff[head_] = item;

			if (full) {
				tail = (tail + 1) % max_size;
			}

			head = (head + 1) % max_size;

			full = head == tail;
		}
		T Get() {
			std::lock_guard<std::mutex> lock(mutex_);

			if (empty()) {
				return T();
			}

			//Read data and advance the tail (we now have a free space)
			auto val = buf_[tail_];
			full_ = false;
			tail_ = (tail_ + 1) % max_size_;

			return val;
		}
		void Reset() {
			std::lock_guard<std::mutex> lock(mut);
			head = tail;
			full = false;
		}
		bool Empty() const {
			return (!full && (head == tail));
		}
		bool Full()  const {
			return full;
		}
		size_t Capacity() const {
			return max_size;
		}
		size_t Size() const {
			if (full) {
				return max_size;
			}
			if (head >= tail) {
				return head - tail;
			}
			return max_size + head - tail;
		}

	private:
		std::mutex mut;
		T* buff;
		size_t head = 0;
		size_t tail = 0;
		const size_t max_size;
		bool full = false;
	};
} //namespace EWE