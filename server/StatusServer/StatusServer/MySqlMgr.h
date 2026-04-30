#pragma once
#include "MySqlDao.h"
#include "SingleTon.h"
class MySqlMgr:public SingleTon<MySqlMgr>
{
	friend class SingleTon<MySqlMgr>;
public:
	~MySqlMgr();
private:
	MySqlMgr();
	MySqlDao _dao;
};

