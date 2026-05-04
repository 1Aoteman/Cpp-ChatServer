#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
using grpc::Status;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;
using message::LoginReq;
using message::LoginRsp;
struct ChatServer {
public:
	ChatServer() :_host(""), _port(""), _name(""), _count(0) {}
	ChatServer(const ChatServer& cs) :_host(cs._host), _port(cs._port), _name(cs._name), _count(cs._count) {

	}
	ChatServer& operator = (const ChatServer& cs) {
		_host = cs._host;
		_port = cs._port;
		_name = cs._name;
		_count = cs._count;
		return *this;
	}
	std::string _host;
	std::string _port;
	std::string _name;
	int _count;
};
class StatusServerImpl final:public StatusService::Service
{
public:
	StatusServerImpl();
	Status GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply) override;
	Status Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)override;
private:
	void InsertToken(int uid,std::string token);
	ChatServer getChatServer();
	std::unordered_map<std::string, ChatServer> _servers;
	int _server_index;
	std::mutex _mutex;
};

