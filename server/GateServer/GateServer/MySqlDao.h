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
class SqlConnection {
public:
	SqlConnection(sql::Connection* con, int64_t lasttime) :_con(con),_last_oper_time(lasttime){}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};
class MysqlPool {
public:
	MysqlPool(const size_t poolsize, const std::string& url, const std::string& user, const std::string& pass, const std::string schema) :
		_poolsize(poolsize), _url(url), _user(user), _pass(pass), _schema(schema),_b_stop(false){
		try {
			for (int i = 0; i < poolsize; i++) {
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
				auto* con = driver->connect(url, user, pass);
				con->setSchema(_schema);
				// 获取当前时间戳
				auto currentTime = std::chrono::system_clock::now().time_since_epoch();
				// 将时间戳转换为秒
				long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
				_pool.push(std::make_unique<SqlConnection>(con, timestamp));
				std::cout << "mysql pool init success" << std::endl;
			}
			_check_thread = std::thread([this] {
				while (!_b_stop) {
					CheckConnection();
					std::this_thread::sleep_for(std::chrono::seconds(600));//每十分钟查询一下连接
				}
				});
			_check_thread.detach();//与主线程分离
		}
		catch (sql::SQLException & e) {
			std::cout << "mysql pool init failed, error is " << e.what() << std::endl;
		}
		

	}
	void CheckConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		int poolsize = _pool.size();
		// 获取当前时间戳
		auto currentTime = std::chrono::system_clock::now().time_since_epoch();
		// 将时间戳转换为秒
		long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
		
		for (int i = 0; i < poolsize; i++) {
		//队列中并没有迭代器，智能取出来进行检查
			auto con=std::move(_pool.front());
			Defer defer([this, &con] {
				_pool.push(std::move(con));//unique_ptr没有拷贝构造
				});
			_pool.pop();
			//如果与最后一次操作仅仅相差五秒，认为该连接仍是积极的，
			if (timestamp - con->_last_oper_time < 5) {
				continue;
			}
			try {
				std::unique_ptr<sql::Statement> stmt (con->_con->createStatement());
				stmt->executeQuery("select 1");
				//更新时间
				con->_last_oper_time = timestamp;
			}
			catch (sql::SQLException& e) {
				//出现异常，重新设立连接
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
				auto* newcon = driver->connect(_url, _user, _pass);
				newcon->setSchema(_schema);
				con->_con.reset(newcon);
				con->_last_oper_time = timestamp;
			}
		}

	}
	std::unique_ptr<SqlConnection> Getconnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this] {
			if (_b_stop) {
				return true;
			}
			return !_pool.empty();
			});
		std::unique_ptr<SqlConnection> conn(std::move(_pool.front()));//unique_ptr是不支持拷贝和赋值的
		_pool.pop();
		return conn;
	}
	void returnconnection(std::unique_ptr<SqlConnection> con) {
		std::unique_lock<std::mutex> lock(_mutex);
		if (_b_stop) {
			return;
		}
		_pool.push(std::move(con));//加入队列
		_cond.notify_one();//唤醒一个进程去取连接
	}
	void Close() {
		_b_stop = true;
		_cond.notify_all();
	}
	~MysqlPool() {
		//先枷锁
		std::lock_guard <std::mutex> lock(_mutex);
		//队列里非空时
		while (!_pool.empty()) {
			_pool.pop();
		}
	}
private:
	size_t _poolsize;
	std::string _url;//mysql连接地址
	std::string _user;//用户
	std::string _pass;//密码
	std::string _schema;//数据库
	std::queue<std::unique_ptr<SqlConnection>> _pool;
	std::condition_variable _cond;
	std::mutex _mutex;
	std::atomic<bool> _b_stop;
	std::thread _check_thread;
};
struct UserInfo {
	std::string name;
	std::string pwd;
	int uid;
	std::string email;
};
class MySqlDao
{
public:
	MySqlDao();
	~MySqlDao();
	int UserReg(const std::string &name, const std::string &email, const std::string& pwd);
	bool Checkemail(const std::string& name, const std::string& email);
	bool Undatedpwd(const std::string& name, const std::string& pwd);
	bool Checkpwd(const std::string& name, const std::string& pwd,UserInfo& userInfo);
private:
	std::unique_ptr<MysqlPool> _pool;
};
