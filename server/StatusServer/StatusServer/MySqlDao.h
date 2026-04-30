#pragma once
#include "const.h"
#include <thread>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>
#include <memory>
#include <queue>
#include <mutex>
#include <mutex>
class SqlConnection {
public:
	SqlConnection(sql::Connection *con,int64_t time):_con(con),_last_time(time) {}
	std::unique_ptr <sql::Connection>_con;
	int64_t _last_time;
};
class MySqlConPool {
public:
	MySqlConPool(size_t poolsize, std::string url,std::string user,std::string pwd,std::string schema) :
		_pool_size(poolsize),_url(url), _user(user), _pwd(pwd),_schema(schema), _b_stop(false)
	{
		try {
			for (int i = 0; i < poolsize; i++) {
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
				auto con = driver->connect(url, user, pwd);
				con->setSchema(_schema);
				// 获取当前时间戳
				auto currentTime = std::chrono::system_clock::now().time_since_epoch();
				// 将时间戳转换为秒
				long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
				_connections.push(std::make_unique<SqlConnection>(con, timestamp));
				std::cout << "mysql pool init success" << std::endl;
			}
			_check_thread = std::thread([this]() {
				//每隔一段时间就去检查
				CheckConnection();
				std::this_thread::sleep_for(std::chrono::seconds(600));//
				});
			_check_thread.detach();//与主线程分离
		}
		catch (sql::SQLException &e) {
			std::cout << "mqsql pool init failed"<<e.what()<< std::endl;
		}
		
	}
	void CheckConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		int poolsize = _connections.size();
		// 获取当前时间戳
		auto currentTime = std::chrono::system_clock::now().time_since_epoch();
		// 将时间戳转换为秒
		long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
		for (int i = 0; i < poolsize; i++) {
			auto con = std::move(_connections.front());
			
			_connections.pop();
			Defer defer([this, &con] {
				ReturnConnection(std::move(con));
				});
			if (timestamp- con->_last_time < 5)//如果上次使用的时间距离现在不超过5秒
			{
				continue;
			}
			try {
				std::unique_ptr <sql::Statement> pstmt(con->_con->createStatement());
				pstmt->executeQuery("SELECT 1");//执行查询操作，延长连接寿命
				//更新时间
				con->_last_time = timestamp;
			}
			catch (sql::SQLException& e) {
				//出现异常，重新设立连接
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
				auto* newcon = driver->connect(_url, _user, _pwd);
				newcon->setSchema(_schema);
				con->_con.reset(newcon);
				con->_last_time = timestamp;
			}
		}

	}
	std::unique_ptr<SqlConnection> GetConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this] {
			if (_b_stop) {
				return true;
			}
			return !_connections.empty();
		});
		if (_b_stop) {
			return nullptr;
		}
		std::unique_ptr<SqlConnection> con = std::move(_connections.front());
		_connections.pop();
		return con;
	}
	void ReturnConnection(std::unique_ptr<SqlConnection> con) {
		std::unique_lock<std::mutex> lock(_mutex);
		if (_b_stop) {
			return;
		}
		_connections.push(std::move(con));
		_cond.notify_one();
	}
	void Close() {
		_b_stop = true;
		_cond.notify_all();
	 }
	~MySqlConPool(){
		std::lock_guard<std::mutex> lock(_mutex);
		while (!_connections.empty()) {
			_connections.pop();
		}
		
	}
private:
	size_t _pool_size;
	std::atomic<bool> _b_stop;
	std::string _user;//数据库用户
	std::string _pwd;//密码
	std::string _url;//数据库地址
	std::string _schema;//数据库名称
	std::mutex _mutex;
	std::queue<std::unique_ptr<SqlConnection>> _connections;
	std::condition_variable _cond;
	std::thread _check_thread;
};
class MySqlDao
{
public:
	MySqlDao();
	~MySqlDao();
private:
	std::unique_ptr<MySqlConPool> _pool;
};

