#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
{
    ClientContext context;
    GetChatServerReq request;
    GetChatServerRsp reply;
    request.set_uid(uid);
    auto stub = _pool->GetConnection();
    Status status = stub->GetChatServer(&context, request, &reply);
    //可以使用defer自动归还连接
    Defer defer([&stub,this]() {
        _pool->ReturnConnection(std::move(stub));//归还连接
        });
    if (status.ok()) {
        return reply;
    }
    else {
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
    
}
StatusGrpcClient::StatusGrpcClient(){
    auto& conf=ConfigMgr::Inst();//单例只能有一个，禁止拷贝
    auto host = conf["StatusServer"]["Host"];
    auto port = conf["StatusServer"]["Port"];
    _pool.reset(new StatusConPool(5, host, port));

}
