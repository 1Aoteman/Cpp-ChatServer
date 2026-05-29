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
bool MysqlMgr::AddFriend(int from_uid, int to_uid, std::string& back_name)
{
	return _dao.addFriend(from_uid,to_uid,back_name);
}

MysqlMgr::MysqlMgr() {

}
