#include "MySqlMgr.h"
MySqlMgr::MySqlMgr() {
	
}
int MySqlMgr::UserReg(const std::string& name, const std::string& email, const std::string& pwd) {
	return _dao.UserReg(name, email, pwd);
}

bool MySqlMgr::CheckEmail(const std::string& name, const std::string& email)
{
	return _dao.Checkemail(name,email);
}

bool MySqlMgr::UndatedPwd(const std::string& name, const std::string& pwd)
{
	return _dao.Undatedpwd(name,pwd);
}

bool MySqlMgr::CheckPwd(const std::string& name, const std::string& pwd,UserInfo& userInfo)
{
	return _dao.Checkpwd(name,pwd,userInfo);
}

