#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "Singleton.h"
using grpc::Channel;//连接
using grpc::Status;
using grpc::ClientContext;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;
//创建池
class StatusConPool {
public:
	StatusConPool(size_t poolsize, std::string host, std::string port) :_pool_size(poolsize),
		_host(host), _port(port), _b_stop(false) {
		for (int i = 0; i < poolsize; i++) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
				grpc::InsecureChannelCredentials());
			_connections.push(StatusService::NewStub(channel));
		}
	}

	~StatusConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		Close();
		while(!_connections.empty()) {
			_connections.pop();
		}
	}
	std::unique_ptr<StatusService::Stub> GetConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this] {
			if (_b_stop) {
				return true;
			}
			return !_connections.empty();
			});
		if (_b_stop) {
			return nullptr;
		}
		auto con = std::move(_connections.front());
		_connections.pop();
		return con;
	}
	void Close() {
		_b_stop = true;
		_cond.notify_all();
	}
	void ReturnConnection(std::unique_ptr<StatusService::Stub> conn) {
		std::unique_lock<std::mutex> lock(_mutex);
		if (_b_stop) {
			return;
		}
		_connections.push(std::move(conn));
		_cond.notify_one();//归还了一个连接，唤醒一个；
	}
private:
	size_t _pool_size;
	std::string _host;
	std::string _port;
	std::atomic<bool> _b_stop;
	std::queue<std::unique_ptr<StatusService::Stub>> _connections;
	std::condition_variable _cond;
	std::mutex _mutex;
};

class StatusGrpcClient:public Singleton<StatusGrpcClient>
{
	friend class Singleton<StatusGrpcClient>;
public:
	~StatusGrpcClient() {

	}
	GetChatServerRsp GetChatServer(int uid);
private:
	StatusGrpcClient();
	std::unique_ptr<StatusConPool> _pool;
};

