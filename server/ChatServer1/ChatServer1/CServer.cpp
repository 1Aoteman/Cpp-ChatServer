#include "CServer.h"
#include "AsioIOServicePool.h"
#include "UserMgr.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
CServer::CServer(boost::asio::io_context& ioc, short port) :_ioc(ioc), _port(port),
_acceptor(ioc, tcp::endpoint(tcp::v4(), port)), _timer(ioc,std::chrono::seconds(60))
{
	_timer.async_wait([this](boost::system::error_code e) {
		On_timer(e);
		});
}
CServer::~CServer() {
	std::cout << "Server destruct listen on port : " << _port << std::endl;

}
void CServer::StartAccept() {
	//接受连接
	auto &_io_context = AsioIOServicePool::GetInstance()->GetIoService();
	auto newsession = std::make_shared<CSession>(_io_context,this);
	_acceptor.async_accept(newsession->GetSocket(), [newsession,this](const boost::system::error_code &err_code) {
		if (!err_code) {
			std::cout << "测试一下" << std::endl;
			//如果没有错误，Csession处理
			newsession->Start();
			_sessions.insert(std::make_pair(newsession->GetSessionId(), newsession));
			//循环调用
			StartAccept();

		}
		else {
			std::cout << "session accept failed:" << err_code.what()<<std::endl;
		}
	});
}
//提升锁的精度，
void CServer::On_timer(const boost::system::error_code& ec) {
	if (ec) {
		std::cout << "time error" << ec.message() << std::endl;
		return;
	}
	std::map<std::string, std::shared_ptr<CSession>> _sessions_copy;
	{
		std::lock_guard<std::mutex> _lock(_mutex);
		_sessions_copy = _sessions;
	}
	//使用vector保存过期的session
	std::vector<std::shared_ptr<CSession>> _expired_sessions;
	int session_count = 0;
	//遍历所有session，查找心跳过期的
	std::time_t now = std::time(nullptr);
	for (auto iter = _sessions_copy.begin(); iter != _sessions_copy.end();iter++) {
		auto b_expired = iter->second->IsHeartExpired(now);
		//如果过期
		if (b_expired) {
			//断开连接
			iter->second->Close();
			_expired_sessions.push_back(iter->second);
			continue;
		}
		session_count++;
	}
	//更新session
	auto& conf = ConfigMgr::Inst();
	auto self_name = conf["SelfServer"]["Name"];
	auto count_str = std::to_string(session_count);
	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, self_name, count_str);
	//删除过期的session
	for (auto& iden : _expired_sessions) {
		iden->DealExpiredSession();
	}
	//再次设置，下一个60s检测
	_timer.expires_after(std::chrono::seconds(60));
	_timer.async_wait([this](boost::system::error_code ec) {
		On_timer(ec);
		});
}
//void CServer::On_timer(const boost::system::error_code& ec)
//{
//	//使用vector保存过期的session
//	std::vector<std::shared_ptr<CSession>> _expired_sessions;
//	int session_count = 0;
//	//遍历所有session，查找心跳过期的
//	std::time_t now = std::time(nullptr);
//	for (auto iter = _sessions.begin(); iter != _sessions.end();) {
//		auto b_expired = iter->second->IsHeartExpired(now);
//		//如果过期
//		if (b_expired) {
//			//断开连接
//			iter->second->Close();
//			_expired_sessions.push_back(iter->second);
//			iter = _sessions.erase(iter);
//		}
//		else {
//			iter++;
//		}
//		session_count++;
//	}
//	//更新session
//	auto& conf = ConfigMgr::Inst();
//	auto self_name = conf["SelfServer"]["Name"];
//	auto count_str = std::to_string(session_count);
//	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, self_name, count_str);
//	//删除过期的session
//	for (auto& iden : _expired_sessions) {
//		iden->DealExpiredSession();
//	}
//	//再次设置，下一个60s检测
//	_timer.expires_after(std::chrono::seconds(60));
//	_timer.async_wait([this](boost::system::error_code ec) {
//		On_timer(ec);
//		});
//}
void CServer::ClearSession(std::string uuid)
{
	std::lock_guard<std::mutex> _lock(_mutex);
	if (_sessions.find(uuid) != _sessions.end())
	{
		UserMgr::GetInstance()->RevUserSession(_sessions[uuid]->GetUserId(),uuid);
	}
	_sessions.erase(uuid);
}