#pragma once
#include "const.h"
#include"SingleTon.h"
class AsioIoServicePool:public SingleTon<AsioIoServicePool>
{
	friend class SingleTon<AsioIoServicePool>;
public:
	using IoService = boost::asio::io_context;
	using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
	using work_ptr = std::unique_ptr<Work>;
	~AsioIoServicePool();
	AsioIoServicePool(const AsioIoServicePool&) = delete;
	AsioIoServicePool& operator=(const AsioIoServicePool&) = delete;
	boost::asio::io_context& GetIoService();
	void Close();
private:
	AsioIoServicePool(size_t poolsize =2);
	std::vector<work_ptr> _works;
	std::vector<std::thread> _threads;
	std::vector<IoService> _services;
	size_t _next_ioservice;
};

