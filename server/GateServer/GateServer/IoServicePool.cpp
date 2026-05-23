#include "IoServicePool.h"
IoServicePool::IoServicePool(std::size_t size ) :_ioservice(size),
_work(size),_nextioservice(0)
{
	for (int i = 0; i < size; i++)
	{
		_work.emplace_back(new Work(boost::asio::make_work_guard(_ioservice[i].get_executor())));
	}
	for (int i = 0; i < size; i++)
	{
		_threads.emplace_back([this,i]() {
			_ioservice[i].run();
			});
	}
}
boost::asio::io_context& IoServicePool::GetIoService()
{
	auto& service = _ioservice[_nextioservice++];
	if (_nextioservice == 2)
	{
		_nextioservice = 0;
	}
	return service;
}
IoServicePool::~IoServicePool()
{
	Stop();
}
void IoServicePool::Stop()
{
	//ÏÈÈ¥µôwork
	for (auto& work:_work)
	{
		work.reset();
	}
	for (auto& thread:_threads) {
		thread.join();
	}
}