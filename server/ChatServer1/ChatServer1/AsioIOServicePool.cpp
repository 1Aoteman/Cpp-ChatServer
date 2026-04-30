#include "AsioIOServicePool.h"



AsioIOServicePool::AsioIOServicePool(size_t poolsize) :_ioservices(poolsize), _works(poolsize)
{
	for (int i = 0; i < poolsize; i++) {
		_works.emplace_back(new Work(boost::asio::make_work_guard(_ioservices[i].get_executor())));
	}
	for (int i = 0; i < poolsize; i++) {
		_threads.emplace_back([this,i]{
			_ioservices[i].run();
		});
	}
}
AsioIOServicePool::~AsioIOServicePool() {
	Stop();
}
boost::asio::io_context& AsioIOServicePool::GetIoService()
{
	std::lock_guard<std::mutex> lock(_mutex);
	auto& ioc = _ioservices[_nextioservice++];
	if (_nextioservice >=_ioservices.size()) {
		_nextioservice = 0;
	}
	return ioc;
}

void AsioIOServicePool::Stop()
{
	for (auto& ioc : _ioservices) {
		ioc.stop();
	}
	for (auto& work : _works) {
		
		work.reset();
	}
	for (auto& thread : _threads) {
		thread.join();//主线程等带子线程结束之后再结束
	}
}
