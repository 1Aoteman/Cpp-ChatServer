#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
LoginRsp StatusGrpcClient::Login(int id, std::string token)
{
    ClientContext context;
    LoginReq request;
    LoginRsp reply;
    request.set_uid(id);
    request.set_token(token);
    auto _stub = _pool->GetGrpcConnect();
    Status status = _stub->Login(&context, request, &reply);
    //自动归还连接
    Defer defer([&_stub, this] {
        _pool->ReturnGrpcCon(std::move(_stub));
        });
    if (status.ok()) {
        return reply;
    }
    else {
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}
StatusGrpcClient::StatusGrpcClient() {
    auto& gCfgMgr = ConfigMgr::Inst();
    std::string host = gCfgMgr["StatusServer"]["Host"];
    std::string port = gCfgMgr["StatusServer"]["Port"];
    _pool.reset(new StatusConPool(5, host, port));
}
