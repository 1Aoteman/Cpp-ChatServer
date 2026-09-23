#pragma once
#include "Singleton.h"
#include "MysqlDao.h"
#include "message.pb.h"
class MysqlMgr:public Singleton<MysqlMgr>
{
	friend class Singleton<MysqlMgr>;

public:
	~MysqlMgr();
	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUser(std::string name);
	bool AddFriendApply(int uid,int touid);
	bool GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& _apply_list,int begin,int limit);
	bool GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list);
	bool AuthFriendApply(int from_uid, int to_uid);
	bool AddFriend(int from_uid, int to_uid, std::string& back_name, 
		std::vector<std::shared_ptr<message::AddFriendMsg>>& chat_datas);
	bool GetChatThreads(int uid, int last_thread_id, 
		int page_size, std::vector<std::shared_ptr<ChatThreadInfo>>& threads
		, bool& load_more, int64_t& next_last_id);
	bool CreatePrivateChat(int user1_id, int user2_id, int& thread_id);
	std::shared_ptr<PageResult> LoadChatMsg(int threadId, int lastId, int pageSize);
	bool AddChatMsg(std::vector<std::shared_ptr<ChatMessage>>& chat_datas);
	bool AddChatMsg(std::shared_ptr<ChatMessage> chat_data);
private:
	MysqlMgr();
	MysqlDao _dao;
};

