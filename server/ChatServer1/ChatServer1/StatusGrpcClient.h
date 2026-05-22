#pragma once
#include <iostream>
#include "Singleton.h"
#include "const.h"
#include "grpcpp/grpcpp.h"
#include "message.grpc.pb.h"
#include "queue"
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using message::LoginReq;
using message::LoginRsp;
using message::StatusService;
class StatusConPool {
public:
	StatusConPool(size_t pool_size, std::string host, std::string port) :_pool_size(pool_size),
		_host(host), _port(port), _b_stop(false)
	{
		for (int i = 0; i < pool_size; i++) {
			std::shared_ptr channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
			_connections.push(StatusService::NewStub(channel));
		}
	}
	void Close() {
		_b_stop = true;
		_cond.notify_all();
	}
	std::unique_ptr<StatusService::Stub> GetGrpcConnect() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this] {
			if (!_b_stop) {
				return true;
			}
			return !_connections.empty();
			});
		if (_b_stop) {
			return nullptr;
		}
		auto con = std::move(_connections.front());//unique_ptr不支持拷贝
		_connections.pop();
		return con;
	}
	void ReturnGrpcCon(std::unique_ptr<StatusService::Stub> con) {
		std::unique_lock<std::mutex> lock(_mutex);
		if (_b_stop) {
			return;
		}
		_connections.push(std::move(con));
		_cond.notify_one();
	}
	~StatusConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		//注意要先close
		Close();
		while (!_connections.empty()) {
			_connections.pop();
		}
	
	}
private:
	size_t _pool_size;
	std::string _host;
	std::string _port;
	std::queue<std::unique_ptr<StatusService::Stub>> _connections;
	std::atomic<bool> _b_stop;
	std::condition_variable _cond;
	std::mutex _mutex;
};
class StatusGrpcClient :public Singleton<StatusGrpcClient>
{
	friend class Singleton<StatusGrpcClient>;
public:
	
	LoginRsp Login(int id, std::string token);
private:
	StatusGrpcClient();
	std::unique_ptr<StatusConPool> _pool;

};

