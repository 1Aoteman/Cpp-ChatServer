#include "MysqlMgr.h"

MysqlMgr::~MysqlMgr()
{
}
std::shared_ptr<UserInfo> MysqlMgr::GetUser(int uid)
{
	return _dao.getUser(uid);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(std::string name)
{
	return _dao.getUser(name);
}

bool MysqlMgr::AddFriendApply(int uid, int touid)
{
	return _dao.addfriendApply(uid, touid);
}

bool MysqlMgr::GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& _apply_list,int begin, int limit)
{
	return _dao.getApplyList(touid,_apply_list,begin,limit);
}

bool MysqlMgr::AuthFriendApply(int from_uid, int to_uid)
{
	return _dao.authFriendApply(from_uid,to_uid);
}
bool MysqlMgr::GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list) {
	return _dao.getFriendList(self_id, user_list);
}
bool MysqlMgr::AddFriend(int from_uid, int to_uid, std::string& back_name, std::vector<std::shared_ptr<message::AddFriendMsg>>& chat_datas)
{
	return _dao.addFriend(from_uid,to_uid,back_name,chat_datas);
}
bool MysqlMgr::AddChatMsg(std::vector<std::shared_ptr<ChatMessage>>& chat_datas) {
	return _dao.AddChatMsg(chat_datas);
}

bool MysqlMgr::AddChatMsg(std::shared_ptr<ChatMessage> chat_data) {
	return _dao.AddChatMsg(chat_data);
}
bool MysqlMgr::GetChatThreads(int uid, 
	int last_thread_id, int page_size, 
	std::vector<std::shared_ptr<ChatThreadInfo>>& threads
	, bool& load_more, int64_t& next_last_id)
{
	return _dao.getChatThreads(uid, last_thread_id, page_size, threads, load_more, next_last_id);
}
bool MysqlMgr::CreatePrivateChat(int user1_id, int user2_id, int& thread_id)
{
	return _dao.CreatePrivateChat(user1_id, user2_id, thread_id);
}
std::shared_ptr<PageResult> MysqlMgr::LoadChatMsg(int threadId, int lastId, int pageSize)
{
	
	return _dao.LoadChatMsg(threadId, lastId, pageSize);
	
}
MysqlMgr::MysqlMgr() {

}
