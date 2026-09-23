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
bool MysqlDao::addFriend(int from, int to, std::string& back_name, 
	std::vector<std::shared_ptr<message::AddFriendMsg>>& chat_datas) {
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
		std::string reverse_back;
		std::string apply_desc;

		{
			// 1. 锁定并读取
			std::unique_ptr<sql::PreparedStatement> selStmt(con->_con->prepareStatement(
				"SELECT back_name, descs "
				"FROM friend_apply "
				"WHERE from_uid = ? AND to_uid = ? "
				"FOR UPDATE"
			));
			selStmt->setInt(1, to);
			selStmt->setInt(2, from);

			std::unique_ptr<sql::ResultSet> rsSel(selStmt->executeQuery());

			if (rsSel->next()) {
				reverse_back = rsSel->getString("back_name");
				apply_desc = rsSel->getString("descs");
			}
			else {
				// 没有对应的申请记录，直接 rollback 并返回失败
				con->_con->rollback();
				return false;
			}
		}

		{
			// 2. 执行真正的更新
			std::unique_ptr<sql::PreparedStatement> updStmt(con->_con->prepareStatement(
				"UPDATE friend_apply "
				"SET status = 1 "
				"WHERE from_uid = ? AND to_uid = ?"
			));

			updStmt->setInt(1, to);
			updStmt->setInt(2, from);

			if (updStmt->executeUpdate() != 1) {
				// 更新行数不对，回滚
				con->_con->rollback();
				return false;
			}
		}

		{
			// 3. 准备第一个SQL语句, 插入认证方好友数据
			std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("INSERT IGNORE INTO friend(self_id, friend_id, back) "
				"VALUES (?, ?, ?) "
			));
			//反过来的申请时from，验证时to
			pstmt->setInt(1, from); // from id
			pstmt->setInt(2, to);
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
			pstmt2->setInt(1, to); // from id
			pstmt2->setInt(2, from);
			pstmt2->setString(3, reverse_back);
			// 执行更新
			int rowAffected2 = pstmt2->executeUpdate();
			if (rowAffected2 < 0) {
				con->_con->rollback();
				return false;
			}
		}



		// 4. 创建 chat_thread
		long long threadId = 0;
		{
			std::unique_ptr<sql::PreparedStatement> threadStmt(con->_con->prepareStatement(
				"INSERT INTO chat_thread (type, created_at) VALUES ('private', NOW());"
			));

			threadStmt->executeUpdate();

			std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
			std::unique_ptr<sql::ResultSet> rs(
				stmt->executeQuery("SELECT LAST_INSERT_ID()")
			);

			if (rs->next()) {
				threadId = rs->getInt64(1);
			}
			else {
				return false;
			}
		}

		// 5. 插入 private_chat
		{
			std::unique_ptr<sql::PreparedStatement> pcStmt(con->_con->prepareStatement(
				"INSERT INTO private_chat(thread_id, user1_id, user2_id) VALUES (?, ?, ?)"
			));

			pcStmt->setInt64(1, threadId);
			pcStmt->setInt(2, from);
			pcStmt->setInt(3, to);
			if (pcStmt->executeUpdate() < 0) return false;
		}

		// 6. 可选：插入初始消息到 chat_message
		if (apply_desc.empty() == false)
		{
			std::unique_ptr<sql::PreparedStatement> msgStmt(con->_con->prepareStatement(
				"INSERT INTO chat_message(thread_id, sender_id, recv_id, content,created_at, updated_at, status) VALUES (?, ?, ?, ?,NOW(),NOW(),?)"
			));

			msgStmt->setInt64(1, threadId);
			msgStmt->setInt(2, to);
			msgStmt->setInt(3, from);
			msgStmt->setString(4, apply_desc);
			msgStmt->setInt(5, 0);
			if (msgStmt->executeUpdate() < 0) { return false; }

			std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
			std::unique_ptr<sql::ResultSet> rs(
				stmt->executeQuery("SELECT LAST_INSERT_ID()")
			);
			if (rs->next()) {
				auto messageId = rs->getInt64(1);
				auto tx_data = std::make_shared<message::AddFriendMsg>();
				tx_data->set_sender_id(to);
				tx_data->set_msg_id(messageId);
				tx_data->set_msgcontent(apply_desc);
				tx_data->set_thread_id(threadId);
				tx_data->set_unique_id("");
				std::cout << "addfriend insert message success" << std::endl;
				chat_datas.push_back(tx_data);
			}
			else {
				return false;
			}
		}

		{
			std::unique_ptr<sql::PreparedStatement> msgStmt(con->_con->prepareStatement(
				"INSERT INTO chat_message(thread_id, sender_id, recv_id, content, created_at, updated_at, status) VALUES (?, ?, ?, ?,NOW(),NOW(),?)"
			));

			msgStmt->setInt64(1, threadId);
			msgStmt->setInt(2, from);
			msgStmt->setInt(3, to);
			msgStmt->setString(4, "We are friends now!");

			msgStmt->setInt(5, 0);

			if (msgStmt->executeUpdate() < 0) { return false; }

			std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
			std::unique_ptr<sql::ResultSet> rs(
				stmt->executeQuery("SELECT LAST_INSERT_ID()")
			);
			if (rs->next()) {
				auto messageId = rs->getInt64(1);
				auto tx_data = std::make_shared<message::AddFriendMsg>();
				tx_data->set_sender_id(from);
				tx_data->set_msg_id(messageId);
				tx_data->set_msgcontent("We are friends now!");
				tx_data->set_thread_id(threadId);
				tx_data->set_unique_id("");
				chat_datas.push_back(tx_data);
			}
			else {
				return false;
			}
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
bool MysqlDao::getChatThreads(
	int64_t userId,
	int64_t lastId,
	int      pageSize,
	std::vector<std::shared_ptr<ChatThreadInfo>>& threads,
	bool& loadMore,
	int64_t& nextLastId)
{
	//设置参数的初始状态
	loadMore = false;
	nextLastId = lastId;
	threads.clear();
	auto con = _pool->GetConnection();
	if (con == nullptr) {
		return false;
	}
	Defer defer([&con,this] {
		_pool->ReturnConnection(std::move(con));
		});
	auto& _conn = con->_con;
	try {
		std::string sql=
			"WITH all_threads AS ( "
			"  SELECT thread_id, 'private' AS type, user1_id, user2_id "
			"    FROM private_chat "
			"   WHERE (user1_id = ? OR user2_id = ?) "
			"     AND thread_id > ? "
			"  UNION ALL "
			"  SELECT thread_id, 'group'   AS type, 0 AS user1_id, 0 AS user2_id "
			"    FROM group_chat_member "
			"   WHERE user_id   = ? "
			"     AND thread_id > ? "
			") "
			"SELECT thread_id, type, user1_id, user2_id "
			"  FROM all_threads "
			" ORDER BY thread_id "
			" LIMIT ?;";
		std::unique_ptr<sql::PreparedStatement> pstmt(_conn->prepareStatement(sql));
		//占位
		int idx = 1;
		pstmt->setInt64(idx++, userId);              // private.user1_id
		pstmt->setInt64(idx++, userId);              // private.user2_id
		pstmt->setInt64(idx++, lastId);              // private.thread_id > lastId
		pstmt->setInt64(idx++, userId);              // group.user_id
		pstmt->setInt64(idx++, lastId);              // group.thread_id > lastId
		pstmt->setInt(idx++, pageSize + 1);          // LIMIT pageSize+1
		//执行
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::vector<std::shared_ptr<ChatThreadInfo>> temp;
		while (res->next()) {
			auto cti = std::make_shared<ChatThreadInfo>();
			cti->thread_id = res->getInt64("thread_id");
			cti->thread_type = res->getString("type");
			cti->user1_id = res->getInt64("user1_id");
			cti->user2_id = res->getInt64("user2_id");
			temp.push_back(cti);
		}
		//判断是否多取到一条
		if (int(temp.size())>pageSize){
			loadMore =true;
			temp.pop_back();		//丢掉多的一条
		}
		// 如果还有数据，更新 nextLastId 为最后一条的 thread_id
		if (!temp.empty()) {
			nextLastId = temp.back()->thread_id;
		}
		threads = std::move(temp);
	}catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what()
			<< " (MySQL error code: " << e.getErrorCode()
			<< ", SQLState: " << e.getSQLState() << ")\n";
		return false;
	}
	return true;
}
bool MysqlDao::CreatePrivateChat(int user1_id, int user2_id, int& thread_id)
{
	auto con = _pool->GetConnection();
	if (!con) {
		return false;
	}
	Defer defer([this, &con]() {
		_pool->ReturnConnection(std::move(con));
		});
	auto& conn = con->_con;
	try {
		// 开启事务
		conn->setAutoCommit(false);
		// 1. 查询是否已存在私聊并加行级锁
		int uid1 = std::min(user1_id, user2_id);
		int uid2 = std::max(user1_id, user2_id);
		std::string check_sql =
			"SELECT thread_id FROM private_chat "
			"WHERE (user1_id = ? AND user2_id = ?) "
			"FOR UPDATE;";

		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(check_sql));
		pstmt->setInt64(1, uid1);
		pstmt->setInt64(2, uid2);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		if (res->next()) {
			// 如果已存在，返回该 thread_id
			thread_id = res->getInt("thread_id");
			conn->commit();  // 提交事务
			return true;
		}

		// 2. 如果未找到，创建新的 chat_thread 和 private_chat 记录
		// 在 chat_thread 表插入新记录
		std::string insert_chat_thread_sql =
			"INSERT INTO chat_thread (type, created_at) VALUES ('private', NOW());";

		std::unique_ptr<sql::PreparedStatement> pstmt_insert_thread(conn->prepareStatement(insert_chat_thread_sql));
		pstmt_insert_thread->executeUpdate();

		// 获取新插入的 thread_id
		std::string get_last_insert_id_sql = "SELECT LAST_INSERT_ID();";
		std::unique_ptr<sql::PreparedStatement> pstmt_last_insert_id(conn->prepareStatement(get_last_insert_id_sql));
		std::unique_ptr<sql::ResultSet> res_last_id(pstmt_last_insert_id->executeQuery());
		res_last_id->next();
		thread_id = res_last_id->getInt(1);

		// 3. 在 private_chat 表插入新记录
		std::string insert_private_chat_sql =
			"INSERT INTO private_chat (thread_id, user1_id, user2_id, created_at) "
			"VALUES (?, ?, ?, NOW());";


		std::unique_ptr<sql::PreparedStatement> pstmt_insert_private(conn->prepareStatement(insert_private_chat_sql));
		pstmt_insert_private->setInt64(1, thread_id);
		pstmt_insert_private->setInt64(2, uid1);
		pstmt_insert_private->setInt64(3, uid2);
		pstmt_insert_private->executeUpdate();

		// 提交事务
		conn->commit();
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		conn->rollback();
		return false;
	}
	return false;
}
std::shared_ptr<PageResult> MysqlDao::LoadChatMsg(int thread_id, int last_message_id, int page_size)
{
	auto con = _pool->GetConnection();
	if (!con) {
		return nullptr;
	}
	Defer defer([this, &con]() {
		_pool->ReturnConnection(std::move(con));
		});
	auto& conn = con->_con;


	try {
		auto page_res = std::make_shared<PageResult>();
		page_res->load_more = false;
		page_res->next_cursor = last_message_id;

		// SQL：多取一条，用于判断是否还有更多
		const std::string sql = R"(
        SELECT message_id, thread_id, sender_id, recv_id, content,
        created_at, updated_at, status,msg_type
        FROM chat_message
        WHERE thread_id = ?
        AND message_id > ?
        ORDER BY message_id ASC
        LIMIT ?
        )";

		uint32_t fetch_limit = page_size + 1;
		auto pstmt = std::unique_ptr<sql::PreparedStatement>(
			conn->prepareStatement(sql)
		);
		pstmt->setInt(1, thread_id);
		pstmt->setInt(2, last_message_id);
		pstmt->setInt(3, fetch_limit);

		auto rs = std::unique_ptr<sql::ResultSet>(pstmt->executeQuery());

		// 读取 fetch_limit 条记录
		while (rs->next()) {
			ChatMessage msg;
			msg.message_id = rs->getUInt64("message_id");
			msg.thread_id = rs->getUInt64("thread_id");
			msg.sender_id = rs->getUInt64("sender_id");
			msg.recv_id = rs->getUInt64("recv_id");
			msg.content = rs->getString("content");
			msg.chat_time = rs->getString("created_at");
			msg.status = rs->getInt("status");
			msg.msg_type = rs->getInt("msg_type");
			page_res->messages.push_back(std::move(msg));
		}
		if (page_res->messages.size() > page_size) {
			page_res->messages.pop_back();
			page_res->load_more = true;
		}
		if (!page_res->messages.empty()) {
			page_res->next_cursor = page_res->messages.back().message_id;
		}
		else {
			page_res->next_cursor = last_message_id; // 或者设置为 0
		}
		return page_res;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		conn->rollback();
		return nullptr;
	}
	return nullptr;

}

bool MysqlDao::AddChatMsg(std::vector<std::shared_ptr<ChatMessage>>& chat_datas) {
	auto con = _pool->GetConnection();
	if (!con) {
		return false;
	}
	Defer defer([this, &con]() {
		_pool->ReturnConnection(std::move(con));
		});
	auto& conn = con->_con;


	try {
		//关闭自动提交，以手动管理事务
		conn->setAutoCommit(false);
		auto pstmt = std::unique_ptr<sql::PreparedStatement>(
			conn->prepareStatement(
				"INSERT INTO chat_message "
				"(thread_id, sender_id, recv_id, content, created_at, updated_at, status,msg_type) "
				"VALUES (?, ?, ?, ?, ?, ?, ?,?)"
			)
		);

		for (auto& msg : chat_datas) {
			// 普通字段
			pstmt->setUInt64(1, msg->thread_id);
			pstmt->setUInt64(2, msg->sender_id);
			pstmt->setUInt64(3, msg->recv_id);
			pstmt->setString(4, msg->content);

			pstmt->setString(5, msg->chat_time);  // created_at
			pstmt->setString(6, msg->chat_time);  // updated_at

			pstmt->setInt(7, msg->status);
			pstmt->setInt(8, msg->msg_type);
			pstmt->executeUpdate();

			// 2. 取 LAST_INSERT_ID()
			std::unique_ptr<sql::Statement> keyStmt(
				conn->createStatement()
			);
			std::unique_ptr<sql::ResultSet> rs(
				keyStmt->executeQuery("SELECT LAST_INSERT_ID()")
			);
			if (rs->next()) {
				msg->message_id = rs->getUInt64(1);
			}
			else {
				continue;
			}
		}

		conn->commit();
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		conn->rollback();
		return false;
	}
	return true;

}

bool MysqlDao::AddChatMsg(std::shared_ptr<ChatMessage> chat_data) {
	auto con = _pool->GetConnection();
	if (!con) {
		return false;
	}
	Defer defer([this, &con]() {
		_pool->ReturnConnection(std::move(con));
		});
	auto& conn = con->_con;

	try {
		//关闭自动提交，以手动管理事务
		conn->setAutoCommit(false);
		auto pstmt = std::unique_ptr<sql::PreparedStatement>(
			conn->prepareStatement(
				"INSERT INTO chat_message "
				"(thread_id, sender_id, recv_id, content, created_at, updated_at, status,msg_type) "
				"VALUES (?, ?, ?, ?, ?, ?, ?,?)"
			)
		);

		// 绑定参数
		pstmt->setUInt64(1, chat_data->thread_id);
		pstmt->setUInt64(2, chat_data->sender_id);
		pstmt->setUInt64(3, chat_data->recv_id);
		pstmt->setString(4, chat_data->content);
		pstmt->setString(5, chat_data->chat_time);  // created_at
		pstmt->setString(6, chat_data->chat_time);  // updated_at
		pstmt->setInt(7, chat_data->status);
		pstmt->setInt(8, chat_data->msg_type);
		pstmt->executeUpdate();

		// 获取自增主键
		std::unique_ptr<sql::Statement> keyStmt(
			conn->createStatement()
		);
		std::unique_ptr<sql::ResultSet> rs(
			keyStmt->executeQuery("SELECT LAST_INSERT_ID()")
		);
		if (rs->next()) {
			chat_data->message_id = rs->getUInt64(1);
		}

		conn->commit();
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what() << std::endl;
		conn->rollback();
		return false;
	}
}
MysqlDao::~MysqlDao()
{
	_pool->Close();
}
