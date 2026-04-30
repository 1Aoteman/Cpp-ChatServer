#include "CServer.h"
#include "AsioIOServicePool.h"
CServer::CServer(boost::asio::io_context& ioc, short port) :_ioc(ioc),_port(port),
_acceptor(ioc,tcp::endpoint(tcp::v4(),port))
{

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
			//如果没有错误，Csession处理
			newsession->Start();
			_sessions.insert(std::make_pair(newsession->GetUuid(), newsession));

		}
		else {
			std::cout << "session accept failed:" << err_code.what()<<std::endl;
		}
	});
}
void CServer::ClearSession(std::string uuid)
{
	_sessions.erase(uuid);
}