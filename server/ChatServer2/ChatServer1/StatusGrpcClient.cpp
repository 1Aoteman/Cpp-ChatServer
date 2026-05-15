#include "StatusGrpcClient.h"

LoginRsp StatusGrpcClient::Login(int id, std::string token)
{
    ClientContext context;
    LoginReq request;
    LoginRsp reply;
    request.set_uid(id);
    request.set_token(token);
    Status status = _stub->Login(&context, request, &reply);
    if (status.ok()) {
        return reply;
    }
    else {
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}
StatusGrpcClient::StatusGrpcClient() {

}
