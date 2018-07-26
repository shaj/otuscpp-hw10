#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>

#include "metr.h"

// using namespace std::placeholders;

class ThreadPool
{
public:
	using fn_type = std::function<void(std::shared_ptr<Metr>)>;

private:
	std::condition_variable cv;
	std::mutex cv_m;
	std::queue<fn_type> msgs;
	bool quit = false;

	std::vector<std::thread> tpool;
	std::vector<std::shared_ptr<Metr>> tmetr;

	ThreadPool() = delete;

public:

	ThreadPool(std::size_t tn);
	~ThreadPool();

	static void worker(ThreadPool *t, std::queue<fn_type> &q, std::shared_ptr<Metr> metr);

	template<class _FN, class... _ARGS>
	void msgs_put(_FN _fn, _ARGS... _args)
	{
		std::lock_guard<std::mutex> lk(cv_m);
		msgs.push(std::bind(_fn, _args..., std::placeholders::_1));
		cv.notify_one();
	}

	void join();

	std::vector<std::shared_ptr<Metr>> get_metr();

};

