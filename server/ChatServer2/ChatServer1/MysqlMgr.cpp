#include "MysqlMgr.h"

MysqlMgr::~MysqlMgr()
{
}
std::shared_ptr<UserInfo> MysqlMgr::GetUser(int uid)
{
	return _dao.getUser(uid);
}

MysqlMgr::MysqlMgr() {

}
