#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <mutex>
#include "data.h"
#include "json/json.h"
#include "CServer.h"
#include <memory>
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::ChatService;
using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;
using message::KickUserReq;
using message::KickUserRsp;

class ChatServiceImpl final:public ChatService::Service
{
public:
	ChatServiceImpl();
	Status NotifyAddFriend(ServerContext* context, const AddFriendReq* request, AddFriendRsp* reply) override;
	Status NotifyAuthFriend(ServerContext* context, const AuthFriendReq* request, AuthFriendRsp* reply) override;
private:
	bool GetBaseInfo(std::string user_base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
};

