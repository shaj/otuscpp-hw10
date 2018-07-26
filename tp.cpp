
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <memory>
#include <functional>
// #include <iostream>

#include "tp.h"
#include "log.h"
#include "metr.h"




ThreadPool::ThreadPool(std::size_t tn)
{
	SPDLOG_TRACE(my::my_logger, "ThreadPool::ThreadPool");

	tpool.reserve(tn);
	tmetr.reserve(tn);

	for(int i=0; i<tn; i++)
	{
		auto m = std::make_shared<Metr>();
		tpool.emplace_back(std::thread {ThreadPool::worker, this, std::ref(msgs), m});
		tmetr.emplace_back(m);
	}

}

ThreadPool::~ThreadPool()
{
	SPDLOG_TRACE(my::my_logger, "ThreadPool::~ThreadPool");

	// quit = true;
	// cv.notify_all();
	// for(auto &it: tpool) it.join();
	join();
}

void ThreadPool::join()
{
	SPDLOG_TRACE(my::my_logger, "ThreadPool::join()");

	quit = true;
	cv.notify_all();
	for(auto &it: tpool) 
	{
		SPDLOG_TRACE(my::my_logger, "ThreadPool::join    {}", static_cast<void*>(&it));
		if(it.joinable()) it.join();
		SPDLOG_TRACE(my::my_logger, "ThreadPool::join    3");
	}
}

std::vector<std::shared_ptr<Metr>> ThreadPool::get_metr()
{
	SPDLOG_TRACE(my::my_logger, "ThreadPool::get_metr()");

	return tmetr;
}


void ThreadPool::worker(ThreadPool *t, std::queue<fn_type> &q, std::shared_ptr<Metr> metr)
{
	SPDLOG_TRACE(my::my_logger, "ThreadPool::worker");

	// std::cout << std::this_thread::get_id() << " waiting... " << std::endl;
	while(!(t->quit && q.empty()))
	{
		std::unique_lock<std::mutex> lk(t->cv_m);
		t->cv.wait(lk, [&q, t](){ return !q.empty() || t->quit; });
		if(!q.empty())
		{
			fn_type m = q.front();
			q.pop();
			lk.unlock();

			m(metr);
			metr->cnt++;
		}
	}
}


