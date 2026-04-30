#pragma once
#include "const.h"
#include "Singleton.h"
#include "boost/asio.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <mutex>
#include "Singleton.h"
class AsioIOServicePool:public Singleton<AsioIOServicePool>
{
	friend class Singleton<AsioIOServicePool>;
public:
	using IoService = boost::asio::io_context;
	using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;//防止没有任务ioc停止
	using WorkPtr = std::unique_ptr<Work>;
	AsioIOServicePool(const AsioIOServicePool&) = delete;//参数必须使用引用
	AsioIOServicePool& AsioIoServicePool(const AsioIOServicePool&) = delete;
	~AsioIOServicePool();
	boost::asio::io_context& GetIoService();
	void Stop();
private:
	AsioIOServicePool(size_t poolsize = std::thread::hardware_concurrency());//根据核心数创建吃的大小
	std::vector<IoService> _ioservices;
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;
	std::mutex _mutex;
	size_t _nextioservice;
};

