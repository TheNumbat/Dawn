
#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>

class thread_pool {

	bool stop = true;
	std::mutex queue_mutex;
	std::condition_variable condition;
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

public:
	thread_pool();
	~thread_pool();
	
	void start(size_t);
	void finish();
	void kill();

	template<class F, class... Args>
	auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
};
 