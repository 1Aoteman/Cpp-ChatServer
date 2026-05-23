#include "CServer.h"
#include "HttpConnection.h"
#include "IoServicePool.h"
CServer::CServer(net::io_context& ioc, short port):_ioc(ioc),_socket(ioc),
_acceptor(ioc,tcp::endpoint(tcp::v4(),port))
{

}
void CServer::Start()
{
	auto self = shared_from_this();
	auto& io_con = IoServicePool::GetInstance()->GetIoService();
	std::shared_ptr<HttpConnection> new_conn = std::make_shared<HttpConnection>(io_con);
	//与客户端进行链接
	_acceptor.async_accept(new_conn->GetSocket(), [self, new_conn](beast::error_code ec) {
		try {
			if (ec)//如果出错，放弃这个链接
			{
				self->Start();
				return;
			}
			new_conn->Start();
			self->Start();
		}
		catch (std::exception& e)
		{
			std::cout << "与客户端连接异常" << e.what() << std::endl;
			self->Start();
		}
	});
}
