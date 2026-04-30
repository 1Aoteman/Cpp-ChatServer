#include "AsioIoServicePool.h"
AsioIoServicePool::AsioIoServicePool(size_t poolsize) {
	for (int i = 0; i < poolsize; i++) {
		_works.emplace_back(new Work(boost::asio::make_work_guard(_services[i].get_executor())));
	}
	for (int i = 0; i < poolsize; i++) {
		_threads.emplace_back([this, i] {
			_services[i].run();
			});
	}
}
boost::asio::io_context& AsioIoServicePool::GetIoService() {
	auto& service = _services[_next_ioservice++];
	if (_next_ioservice >= 2) {
		_next_ioservice = 0;
	}
	return service;
}
void AsioIoServicePool::Close() {
	for (auto& work : _works) {
		work.reset();
	}
	for (auto& service : _services) {
		service.stop();
	}
}
AsioIoServicePool::~AsioIoServicePool() {
	Close();
}