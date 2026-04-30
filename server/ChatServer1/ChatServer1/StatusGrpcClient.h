#pragma once
#include <iostream>
#include "Singleton.h"
#include "const.h"
#include "grpcpp/grpcpp.h"
#include "message.grpc.pb.h"
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using message::LoginReq;
using message::LoginRsp;
using message::StatusService;

class StatusGrpcClient :public Singleton<StatusGrpcClient>
{
	friend class StatusGrpcClient;
public:
	
	LoginRsp Login(int id, std::string token);
private:
	StatusGrpcClient();
	std::unique_ptr<StatusService::Stub> _stub;

};

