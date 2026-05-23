#pragma once
#include "MySqlDao.h"
#include "Singleton.h"
class MySqlMgr:public Singleton<MySqlMgr>
{
	friend class Singleton<MySqlMgr>;
public:
	int UserReg(const std::string& name, const std::string& email, const std::string& pwd);
	bool CheckEmail(const std::string& name, const std::string& email);
	bool UndatedPwd(const std::string& name, const std::string& pwd);
	bool CheckPwd(const std::string& name, const std::string& pwd,UserInfo& userInfo);
private:
	MySqlMgr();
	MySqlDao _dao;
};

