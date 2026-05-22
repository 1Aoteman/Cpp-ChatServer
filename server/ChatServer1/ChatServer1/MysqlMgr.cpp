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
	
}

MysqlMgr::MysqlMgr() {

}
