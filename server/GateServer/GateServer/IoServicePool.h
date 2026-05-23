#pragma once
#include "Singleton.h"
#include <memory>
#include "boost/asio.hpp"


class IoServicePool:public Singleton<IoServicePool>
{
	friend class Singleton<IoServicePool>;
public:
	using IoService = boost::asio::io_context;
	using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
	using work_ptr = std::unique_ptr<Work>;
	~IoServicePool();
	IoServicePool(const IoServicePool&) = delete;
	IoServicePool& operator=(const IoServicePool&) = delete;
	boost::asio::io_context& GetIoService();
	void Stop();
private:
	IoServicePool(std::size_t size=2);//参数是线程总数
	std::vector<IoService> _ioservice;
	std::vector<std::thread>_threads;
	std::vector<work_ptr>_work;
	std::size_t _nextioservice;


};

