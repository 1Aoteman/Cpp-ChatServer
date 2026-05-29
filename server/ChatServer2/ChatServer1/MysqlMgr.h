#pragma once
#include "Singleton.h"
#include "MysqlDao.h"
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
	bool AddFriend(int from_uid, int to_uid, std::string& back_name);
private:
	MysqlMgr();
	MysqlDao _dao;
};

