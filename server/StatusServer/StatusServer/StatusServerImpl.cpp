#include "StatusServerImpl.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "const.h"
std::string generate_unique_string() {
	// 创建UUID对象
	boost::uuids::uuid uuid = boost::uuids::random_generator()();

	// 将UUID转换为字符串
	std::string unique_string = to_string(uuid);

	return unique_string;
}
StatusServerImpl::StatusServerImpl()
{
	auto& cfg = ConfigMgr::Inst();
	auto server_list = cfg["chatservers"]["Name"];

	std::vector<std::string> words;

	std::stringstream ss(server_list);
	std::string word;

	while (std::getline(ss, word, ',')) {
		words.push_back(word);
	}

	for (auto& word : words) {
		if (cfg[word]["Name"].empty()) {
			continue;
		}

		ChatServer server;
		server._port = cfg[word]["Port"];
		server._host = cfg[word]["Host"];
		server._name = cfg[word]["Name"];
		_servers[server._name] = server;
	}
}

Status StatusServerImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
	std::string prefix("llfc status server has received :  ");
	const auto& server = getChatServer();
	reply->set_host(server._host);
	reply->set_port(server._port);
	reply->set_error(ErrorCodes::Success);
	reply->set_token(generate_unique_string());
	std::cout << reply << std::endl;
	InsertToken(request->uid(), reply->token());
	return Status::OK;
}

Status StatusServerImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
{
	auto uid = request->uid();
	auto token = request->token();
	
	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	std::string token_value = "";
	bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
	if (!success) {
		reply->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}

	if (token_value != token) {
		reply->set_error(ErrorCodes::TokenInvalid);
		return Status::OK;
	}
	reply->set_error(ErrorCodes::Success);
	reply->set_uid(uid);
	reply->set_token(token);
	return Status::OK;
}

void StatusServerImpl::InsertToken(int uid, std::string token)
{
	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	RedisMgr::GetInstance()->Set(token_key, token);
}

ChatServer StatusServerImpl::getChatServer()
{
	std::lock_guard<std::mutex> guard(_mutex);
	auto minServer = _servers.begin()->second;

	auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer._name);
	if (count_str.empty()) {
		//不存在则默认设置为最大
		minServer._count = INT_MAX;
	}
	else {
		minServer._count = std::stoi(count_str);
	}


	 //使用范围基于for循环
	for ( auto& server : _servers) {
		
		if (server.second._name == minServer._name) {
			continue;
		}

		auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second._name);
		if (count_str.empty()) {
			server.second._count = INT_MAX;
		}
		else {
			server.second._count = std::stoi(count_str);
		}

		if (server.second._count < minServer._count) {
			minServer = server.second;
		}
	}

	return minServer;
}
