
#include "thread_pool.h"
#include <assert.h>

thread_pool::thread_pool() {
	stop = true;
}

thread_pool::~thread_pool() {
	finish();
}

void thread_pool::start(size_t threads) {
	stop = false;
	for(size_t i = 0;i<threads;++i)
		workers.emplace_back(
			[this]
			{
				for(;;)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock,
							[this]{ return this->stop || !this->tasks.empty(); });
						if(this->stop)
							return;
						task = std::move(this->tasks.front());
						this->tasks.pop();
					}

					task();
				}
			}
		);
}

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

void thread_pool::finish() {
	
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	
	condition.notify_all();
	for(std::thread &worker: workers) {
		worker.join();
	}
	workers.clear();
	
	std::queue<std::function<void()>> empty;
	std::swap(tasks, empty);
}

void thread_pool::kill() {

	// we call this at shutdown and let the threads crash
	// because apparently there's no way to just hard-kill
	// a std::thread. This is highly annoying because the
	// debugger now breaks whenever we shut down with threads
	// still going.
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}

	condition.notify_all();
	for (std::thread& worker : workers) {
		worker.detach();
	}
	workers.clear();

	std::queue<std::function<void()>> empty;
	std::swap(tasks, empty);
}
