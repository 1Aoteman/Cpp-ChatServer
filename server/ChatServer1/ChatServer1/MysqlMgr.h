#pragma once
#include "Singleton.h"
#include "MysqlDao.h"
class MysqlMgr:public Singleton<MysqlMgr>
{
	friend class Singleton<MysqlMgr>;

public:
	~MysqlMgr();
	std::shared_ptr<UserInfo> GetUser(int uid);
private:
	MysqlMgr();
	MysqlDao _dao;
};

