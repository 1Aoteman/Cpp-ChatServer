#pragma once
#include <queue>
#include "const.h"
#include "Singleton.h"
#include "ConfigMgr.h"
#include <grpcpp/grpcpp.h> 
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "data.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::GetChatServerRsp;
using message::LoginRsp;
using message::LoginReq;
using message::ChatService;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;

using message::KickUserReq;
using message::KickUserRsp;

class ChatConPool {
public:
	ChatConPool(size_t poolsize, std::string host, std::string port) :_pool_size(poolsize), _host(host), _port(port),
		_b_stop(false)
	{
		for (int i = 0; i < poolsize; i++) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
			_connections.push(ChatService::NewStub(channel));
		}
	}
	std::unique_ptr<ChatService::Stub> GetConnection() {
		std::unique_lock<std::mutex> lock(_chatgrpc_mutex);
		_cond.wait(lock, [this]() {
			//如果停止，还在等候的全部释放；
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
	void Returnconn(std::unique_ptr<ChatService::Stub> con) {
		std::unique_lock<std::mutex> lock(_chatgrpc_mutex);
		if (_b_stop) {
			return;
		}
		_connections.push(std::move(con));
		_cond.notify_one();
	}
	void Close() {
		_b_stop = true;
		_cond.notify_all();
	}
	~ChatConPool() {
		std::lock_guard<std::mutex> lock(_chatgrpc_mutex);
		Close();
		while (!_connections.empty()) {
			_connections.pop();
		}
	}
private:
	std::atomic<bool> _b_stop;
	std::mutex _chatgrpc_mutex;
	size_t _pool_size;
	std::string _host;
	std::string _port;
	std::queue<std::unique_ptr<ChatService::Stub>> _connections;
	std::condition_variable _cond;

};
//设计为单例类，
class ChatGrpcClient:public Singleton<ChatGrpcClient>
{
public:
	~ChatGrpcClient();
	AddFriendRsp NotifyAddFriend(std::string server_ip, const AddFriendReq& req);
private:
	ChatGrpcClient();
	//使用map来管理//分布式有两个聊天服务齐，可能会向不同的服务器发送消息;
	std::unordered_map<std::string, std::unique_ptr<ChatConPool>> _pools;
};

