#pragma once
#include "grpcpp/grpcpp.h"
#include "const.h"
#include "message.grpc.pb.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class GrpcconPool
{
public:
	GrpcconPool(std::size_t poolszie,std::string host,std::string port);
	~GrpcconPool();
	std::unique_ptr<VarifyService::Stub> GetConnection();
	void ReturnConnection(std::unique_ptr<VarifyService::Stub> context);
	void Close();
private:
	std::size_t _poolsize;
	std::atomic<bool> _b_stop;
	std::queue<std::unique_ptr<VarifyService::Stub>> _connections;
	std::string _host;
	std::string _port;
	std::mutex _mutex;
	std::condition_variable _con;

};

class VarifyGrpcClient:public Singleton<VarifyGrpcClient>
{
	friend class Singleton<VarifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(std::string email) {
		ClientContext context;
		GetVarifyReq request;
		GetVarifyRsp reply;
		request.set_email(email);
		auto stub = pool_->GetConnection();
		Status status = stub->GetVarifyCode(&context, request, &reply);

		if (status.ok()) {
			pool_->ReturnConnection(std::move(stub));//还回连接
			return reply;
		}
		else {
			pool_->ReturnConnection(std::move(stub));
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}
private:
	VarifyGrpcClient();
	std::unique_ptr<GrpcconPool> pool_;
};

