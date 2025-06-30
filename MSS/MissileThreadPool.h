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
	// ������ Ǯ �ʱ�ȭ
	void init(int threadCount);
	// �۾� �߰�
	template<typename F>
	void enqueue(F&& f);
	// ������ Ǯ ����
	void shutdown();
private:
	std::vector<std::thread> threads_;  // ������ ����
	std::queue<std::function<void()>> tasks_;  // �۾� ť
	std::mutex queueMutex_;  // �۾� ť ���ؽ�
	std::condition_variable condition_;  // ���� ����
	bool stop_ = false;  // ���� �÷���
};