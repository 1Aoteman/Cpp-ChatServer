#include "MysqlDao.h"
#include "ConfigMgr.h"



std::shared_ptr<UserInfo> MysqlDao::getUser(int uid)
{
	auto con = _pool->GetConnection();
	try {
		if (con == nullptr) {
			std::cout << "获取数据库连接失败" << std::endl;
			return nullptr;
		}
		Defer defer([this, &con]() {
			_pool->ReturnConnection(std::move(con));
			});
		std::unique_ptr<sql::PreparedStatement> pstmt(
			con->_con->prepareStatement("SELECT * FROM user WHERE uid = ?")
		);

		// 3. 把 uid 塞进第一个问号 (?) 里
		pstmt->setInt(1, uid);

		// 4. 执行查询，拿到结果集
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		// 5. 遍历结果（因为 uid 是主键，理论上最多只有一条结果，所以用 if 就行）
		if (res->next()) {
			// 在堆上创建一个 UserInfo 对象
			auto user = std::make_shared<UserInfo>();

			// 把数据库里的字段按列名取出来，塞进结构体
			user->uid = uid;
			user->name = res->getString("name");
			user->pwd = res->getString("pwd");
			user->email = res->getString("email");
			user->nick = res->getString("nick");
			user->desc = res->getString("desc");
			user->sex = res->getInt("sex");
			user->icon = res->getString("icon");

			// 成功拿到数据，返回这个共享指针
			return user;
		}
	}catch (sql::SQLException& e) {
		std::cout << "sql exception is" <<e.what() <<std::endl;
	}
}

std::shared_ptr<UserInfo> MysqlDao::getUser(std::string name)
{
	auto con = _pool->GetConnection();
	try {
		if (con == nullptr) {
			std::cout << "获取数据库连接失败" << std::endl;
			return nullptr;
		}
		Defer defer([this, &con]() {
			_pool->ReturnConnection(std::move(con));
			});
		std::unique_ptr<sql::PreparedStatement> pstmt(
			con->_con->prepareStatement("SELECT * FROM user WHERE name = ?")
		);

		// 3. 把 uid 塞进第一个问号 (?) 里
		pstmt->setString(1, name);

		// 4. 执行查询，拿到结果集
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		// 5. 遍历结果（因为 uid 是主键，理论上最多只有一条结果，所以用 if 就行）
		if (res->next()) {
			// 在堆上创建一个 UserInfo 对象
			auto user = std::make_shared<UserInfo>();

			// 把数据库里的字段按列名取出来，塞进结构体uid, QString name, QString nick, QString desc, int sex,QString icon
			user->uid = res->getInt("uid");
			user->name = name;
			user->pwd = res->getString("pwd");
			user->email = res->getString("email");
			user->nick = res->getString("nick");
			user->desc = res->getString("desc");
			user->sex = res->getInt("sex");
			user->icon = res->getString("icon");

			// 成功拿到数据，返回这个共享指针
			return user;
		}
	}
	catch (sql::SQLException& e) {
		std::cout << "sql exception is" << e.what() << std::endl;
	}
}

MysqlDao::MysqlDao()
{
	auto& conf = ConfigMgr::Inst();
	std::string host = conf["Mysql"]["Host"];
	std::string port = conf["Mysql"]["Port"];
	std::string user = conf["Mysql"]["User"];
	std::string passwd = conf["Mysql"]["Passwd"];
	std::string schema = conf["Mysql"]["Schema"];
	_pool.reset(new MysqlConPool(5, host + ":" + port, user, passwd, schema));
}
MysqlDao::~MysqlDao()
{
	_pool->Close();
}
