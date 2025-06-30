#pragma once

#include <vector>
#include <queue>
#include<mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <string>

class MissileThreadPool {

public:
	MissileThreadPool() = default;
	~MissileThreadPool() = default;
	// 스레드 풀 초기화
	void init(int threadCount);
	// 작업 추가
	template<typename F>
	void enqueue(F&& f);
	// 스레드 풀 종료
	void shutdown();
private:
	std::vector<std::thread> threads_;  // 스레드 벡터
	std::queue<std::function<void()>> tasks_;  // 작업 큐
	std::mutex queueMutex_;  // 작업 큐 뮤텍스
	std::condition_variable condition_;  // 조건 변수
	bool stop_ = false;  // 종료 플래그
};