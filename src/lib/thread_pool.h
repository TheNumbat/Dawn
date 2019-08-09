
#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <assert.h>

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

template<class F, class... Args>
auto thread_pool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {

	using return_type = typename std::result_of<F(Args...)>::type;

	assert(!stop);

	auto task = std::make_shared<std::packaged_task<return_type()>>(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		
	std::future<return_type> res = task->get_future();
	
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		tasks.emplace([task](){ (*task)(); });
	}
	condition.notify_one();
	return res;
}
