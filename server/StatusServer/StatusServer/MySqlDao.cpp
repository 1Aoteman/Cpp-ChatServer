#include "MySqlDao.h"
#include "ConfigMgr.h"
MySqlDao::MySqlDao()
{
	auto &conf = ConfigMgr::Inst();
	std::string host = conf["MySql"]["Host"];
	std::string port = conf["MySql"]["Port"];
	std::string user = conf["MySql"]["User"];
	std::string passwd = conf["MySql"]["PassWd"];
	std::string schema = conf["MySql"]["Schema"];
	_pool.reset(new MySqlConPool(5,host + ":" + port, user, passwd, schema));
}

MySqlDao::~MySqlDao()
{
	_pool->Close();
}
