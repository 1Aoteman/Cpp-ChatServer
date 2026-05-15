#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include "const.h"
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>
#include "data.h"
class SqlConnection {
public:
	SqlConnection(sql::Connection* con,int64_t last_time):_con(con),_last_time(last_time){}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_time;
};
class MysqlConPool {
public:
	MysqlConPool(size_t poolsize, std::string url, std::string user, std::string pass, std::string schema) :
		_pool_size(poolsize), _user(user), _pass(pass), _schema(schema), _b_stop(false)
	{
		try {
			for (int i = 0; i < poolsize; i++) {
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
				auto* con(driver->connect(_url, _user, _pass));
				con->setSchema(_schema);
				// 获取当前时间戳
				auto currentTime = std::chrono::system_clock::now().time_since_epoch();
				// 将时间戳转换为秒
				long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
				_conn.push(std::make_unique<SqlConnection>(con, timestamp));
				std::cout << "mysql connect success" << std::endl;
			}
			//一个线程用来验证连接是否会被回收,每60秒去检查一次连接
			_check_thread = std::thread([this] {
				CheckConnection();
				std::this_thread::sleep_for(std::chrono::seconds(600));
				});
			_check_thread.detach();//与主线程分离
		}
		catch (sql::SQLException& e) {
			std::cout << "sql eccepyion is" << e.what() << std::endl;
		}
		
	}
	void CheckConnection() {
		size_t poolsize = _pool_size;
		// 获取当前时间戳
		auto currentTime = std::chrono::system_clock::now().time_since_epoch();
		// 将时间戳转换为秒
		long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
		for (int i = 0; i < poolsize; i++) {
			auto con=std::move(_conn.front());
			_conn.pop();
			Defer defer([&con,this] {
				_conn.push(std::move(con));
				});
			if (timestamp - con->_last_time <5) {
				continue;
			}
			try {
				std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
				stmt->executeQuery("select 1 ");//执行一个查询来延长
				con->_last_time = timestamp;
			}
			catch (sql::SQLException& e) {
				//如果出现异常，//因为原连接已经被销毁，重新建立一个连接加入队列
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
				auto* newcon(driver->connect(_url, _user, _pass));
				newcon->setSchema(_schema);
				con->_con.reset(newcon);
				con->_last_time = timestamp;
			}
		}
	}
	std::unique_ptr<SqlConnection> GetConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [&] {
			if (_b_stop) {
				return true;
			}
			return !_conn.empty();
			});
		//防止队列为空时内存越界
		if (_b_stop) {
			return nullptr;
		}
		std::unique_ptr<SqlConnection> con=std::move(_conn.front());
		_conn.pop();
		return con;
	}
	void ReturnConnection(std::unique_ptr<SqlConnection> con) {
		std::unique_lock<std::mutex> lock(_mutex);
		if (_b_stop) {
			return;
		}
		_conn.push(std::move(con));
		_cond.notify_one();//归还了一个连接，唤醒一个进程去取；
	}
	void Close() {
		_b_stop = true;
		_cond.notify_all();
	}
	~MysqlConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		while (!_conn.empty()) {
			_conn.pop();
		}
	}
private:
	size_t _pool_size;
	std::atomic<bool> _b_stop;
	std::queue<std::unique_ptr<SqlConnection>> _conn;//使用队列来存放连接
	std::mutex _mutex;
	std::thread _check_thread;
	std::string _url;
	std::string _user;
	std::string _pass;
	std::string _schema;//数据库名
	std::condition_variable _cond;
};
class MysqlDao
{
public:
	std::shared_ptr<UserInfo> getUser(int uid);
	MysqlDao();
	~MysqlDao();
private:
	std::unique_ptr<MysqlConPool> _pool;
};

