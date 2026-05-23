#pragma once
#include "const.h"
class CServer:public std::enable_shared_from_this<CServer>
{
public:
	CServer(net::io_context&ioc,short port);
	void Start();
private:
	tcp::socket _socket;
	boost::asio::ip::tcp::acceptor _acceptor;
	net::io_context& _ioc;
};

