#pragma once
#include "boost/asio.hpp"

#include "CSession.h"
#include <iostream>
#include <mutex>
#include <map>
#include <memory>
using boost::asio::ip::tcp;
class CServer
{
public:
	
	CServer(boost::asio::io_context& ioc, short port);
	~CServer();
	void ClearSession(std::string uuid);
	void Stop();
	void StartAccept();
private:
	std::mutex _mutex;
	boost::asio::io_context& _ioc;
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
	short _port;
};

