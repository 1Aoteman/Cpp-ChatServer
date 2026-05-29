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
bool MysqlDao::addfriendApply(int from, int to) {
	auto con = _pool->GetConnection();
	if (con == nullptr) {
		return false;
	}
	Defer defer([this, &con]() {
		_pool->ReturnConnection(std::move(con));
		});
	try {
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("INSERT INTO friend_apply (from_uid, to_uid) values (?,?) "
			"ON DUPLICATE KEY UPDATE from_uid = from_uid, to_uid = to_uid "));
		pstmt->setInt(1, from);
		pstmt->setInt(2, to);
		//执行更新
		int rowAffected = pstmt->executeUpdate();
		if (rowAffected < 0) {
			return false;
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
	return true;
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
bool MysqlDao::getApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& _apply_list,int begin, int limit) {
	auto con = _pool->GetConnection();
	if (con == nullptr) {
		return false;
	}
	Defer defer([this,&con] {
		_pool->ReturnConnection(std::move(con));
		});
	try {
		// 准备SQL语句, 根据起始id和限制条数返回列表
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("select apply.from_uid, apply.status, user.name, "
			"user.nick, user.sex from friend_apply as apply join user on apply.from_uid = user.uid where apply.to_uid = ? "
			"and apply.id > ? order by apply.id ASC LIMIT ? "));
		pstmt->setInt(1, touid); // 将uid替换为你要查询的uid
		pstmt->setInt(2, begin); // 起始id
		pstmt->setInt(3, limit); //偏移量
		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		// 遍历结果集
		while (res->next()) {
			auto name = res->getString("name");
			auto uid = res->getInt("from_uid");
			auto status = res->getInt("status");
			auto nick = res->getString("nick");
			auto sex = res->getInt("sex");
			auto apply_ptr = std::make_shared<ApplyInfo>(uid, name, nick,"", sex,"",status);
			_apply_list.push_back(apply_ptr);
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}
bool MysqlDao::getFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list)
{
	auto con = _pool->GetConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		_pool->ReturnConnection(std::move(con));
		});


	try {
		// 准备SQL语句, 根据起始id和限制条数返回列表
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("select * from friend where self_id = ? "));

		pstmt->setInt(1, self_id); // 将uid替换为你要查询的uid

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		// 遍历结果集
		while (res->next()) {
			auto friend_id = res->getInt("friend_id");
			auto back = res->getString("back");
			//再一次查询friend_id对应的信息
			auto user_info = getUser(friend_id);
			if (user_info == nullptr) {
				continue;
			}

			user_info->back = user_info->name;
			user_list.push_back(user_info);
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}

	return true;
}
bool MysqlDao::authFriendApply(int from_uid, int to_uid) {
	auto con = _pool->GetConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		_pool->ReturnConnection(std::move(con));
		});

	try {
		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("UPDATE friend_apply SET status = 1 "
			"WHERE from_uid = ? AND to_uid = ?"));
		//反过来的申请时from，验证时to
		pstmt->setInt(1, to_uid); // from id
		pstmt->setInt(2, from_uid);
		// 执行更新
		int rowAffected = pstmt->executeUpdate();
		if (rowAffected < 0) {
			return false;
		}
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}


	return true;
}
bool MysqlDao::addFriend(int from_uid, int to_uid, std::string& back_name) {
	auto con = _pool->GetConnection();
	if (con == nullptr) {
		return false;
	}

	Defer defer([this, &con]() {
		_pool->ReturnConnection(std::move(con));
		});

	try {

		//开始事务
		con->_con->setAutoCommit(false);

		// 准备第一个SQL语句, 插入认证方好友数据
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "
		));
		//反过来的申请时from，验证时to
		pstmt->setInt(1, from_uid); // from id
		pstmt->setInt(2, to_uid);
		pstmt->setString(3, back_name);
		// 执行更新
		int rowAffected = pstmt->executeUpdate();
		if (rowAffected < 0) {
			con->_con->rollback();
			return false;
		}

		//准备第二个SQL语句，插入申请方好友数据
		std::unique_ptr<sql::PreparedStatement> pstmt2(con->_con->prepareStatement("INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "
		));
		//反过来的申请时from，验证时to
		pstmt2->setInt(1, to_uid); // from id
		pstmt2->setInt(2, from_uid);
		pstmt2->setString(3, "");
		// 执行更新
		int rowAffected2 = pstmt2->executeUpdate();
		if (rowAffected2 < 0) {
			con->_con->rollback();
			return false;
		}

		// 提交事务
		con->_con->commit();
		std::cout << "addfriend insert friends success" << std::endl;

		return true;
	}
	catch (sql::SQLException& e) {
		// 如果发生错误，回滚事务
		if (con) {
			con->_con->rollback();
		}
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}


	return true;
}
MysqlDao::~MysqlDao()
{
	_pool->Close();
}
