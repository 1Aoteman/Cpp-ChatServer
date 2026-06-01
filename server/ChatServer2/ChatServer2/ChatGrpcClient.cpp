#include "ChatGrpcClient.h"

ChatGrpcClient::ChatGrpcClient()
{
	auto& conf = ConfigMgr::Inst();
	auto server_list = conf["PeerServer"]["Servers"];
	std::vector<std::string> words;
	std::stringstream ss(server_list);
	std::string word;
	while (std::getline(ss, word, ',')) {
		words.push_back(word);
	}
	for (auto w : words) {
		if (conf[w]["Name"].empty()) {
			continue;
		}
		_pools[conf[w]["Name"]] = std::make_unique<ChatConPool>(5, conf[w]["Host"], conf[w]["Port"]);
	}
}
ChatGrpcClient::~ChatGrpcClient() {

}
AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req) {
	AddFriendRsp rsp;
	//使用defer保证
	Defer defer([&rsp,&req] {
		rsp.set_error(ErrorCodes::Success);
		rsp.set_applyuid(req.applyuid());
		rsp.set_touid(req.touid());
		});
	//多台服务器要去map来查找连接池
	auto find_iter = _pools.find(server_ip);
	if (find_iter == _pools.end()) {
		return rsp;
	}
	//取出对应的连接池
	auto& con = find_iter->second;
	ClientContext context;
	auto stub = con->GetConnection();
	//使用grpc来放请求
	grpc::Status status = stub->NotifyAddFriend(&context, req, &rsp);
	//这个defer用来归还连接
	Defer defercon([&con,&stub] {
		con->Returnconn(std::move(stub));
		});
	//判断是否出错
	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}
	return rsp;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req)
{
	AuthFriendRsp rsp;
	rsp.set_error(ErrorCodes::Success);
	Defer defer([&req, &rsp] {
		rsp.set_fromuid(req.fromuid());
		rsp.set_touid(req.touid());
		});
	auto find_iter = _pools.find(server_ip);
	if (find_iter == _pools.end()) {
		return rsp;
	}
	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->GetConnection();
	Status status = stub->NotifyAuthFriend(&context,req,&rsp);
	Defer rdefer([&pool, &stub] {
		pool->Returnconn(std::move(stub));
		});
	//判断状态是否正常
	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}
	return rsp;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue)
{
	TextChatMsgRsp rsp;
	rsp.set_error(ErrorCodes::Success);
	Defer defer([&rsp,&req] {
		rsp.set_fromuid(req.fromuid());
		rsp.set_touid(req.touid());
		for (const auto& obj : req.textmsgs()) {
			TextChatData* chatdata = rsp.add_textmsgs();
			chatdata->set_msgid(obj.msgid());
			chatdata->set_msgcontent(obj.msgcontent());
		}
		});
	//找到对应服务器的连接池；
	auto find_iter = _pools.find(server_ip);
	if (find_iter == _pools.end()) {
		std::cout << "没有找到对应的服务器 " << server_ip << " 的连接池" << std::endl;
		return rsp;
	}
	std::cout << "找到对应的服务器 " << server_ip << " 的连接池" << std::endl;
	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->GetConnection();
	Status status = stub->NotifyTextChatMsg(&context, req, &rsp);
	std::cout << "发送了grpc请求" << std::endl;
	Defer defercon([&stub, this, &pool]() {
		pool->Returnconn(std::move(stub));
		});
	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}
	return rsp;
}
