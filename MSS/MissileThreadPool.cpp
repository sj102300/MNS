#pragma once

#include "MissileThreadPool.h"

void MissileThreadPool::init(int threadCount) {
	stop_ = false;
	for (int i = 0; i < threadCount; ++i) {
		threads_.emplace_back([this]() {
			while (true) {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(queueMutex_);
					condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
					if (stop_ && tasks_.empty()) return;
					task = std::move(tasks_.front());
					tasks_.pop();
				}
				task();
			}
			});
	}
}

template<typename F>
void MissileThreadPool::enqueue(F&& f) {
	{
		std::unique_lock<std::mutex> lock(queueMutex_);
		tasks_.emplace(std::forward<F>(f));
	}
	condition_.notify_one();
}

void MissileThreadPool::shutdown() {
	{
		std::unique_lock<std::mutex> lock(queueMutex_);
		stop_ = true;
	}
	condition_.notify_all();
	for (std::thread& thread : threads_) {
		if (thread.joinable()) {
			thread.join();
		}
	}
	threads_.clear();
}

