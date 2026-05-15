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
		if (conf[word]["Name"].empty()) {
			continue;
		}
		_pools[conf[word]["Name"]] = std::make_unique<ChatConPool>(5, conf[word]["Host"], conf[word]["Port"]);
	}
}
ChatGrpcClient::~ChatGrpcClient() {

}
AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req) {
	AddFriendRsp rsp;
	return rsp;
}