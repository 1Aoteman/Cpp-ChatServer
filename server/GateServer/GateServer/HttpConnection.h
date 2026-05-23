#pragma once
#include "const.h"


class HttpConnection:public std::enable_shared_from_this<HttpConnection>
{
	friend class LogicSystem;
public:
	HttpConnection(boost::asio::io_context& ioc);
	void Start();
	tcp::socket& GetSocket()//socket不能被拷贝
	{
		return _socket;
	}
private:
	void CheckDeadLine();//检查是否超时
	void WriteResponse();//回应
	void HandleReq();//处理请求
	void PreParseGetParam();
	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
	tcp::socket _socket;
	beast::flat_buffer _buffer{ 8192 };
	http::request < http::dynamic_body > _request;
	http::response < http::dynamic_body > _response;
	//超时检测，
	net::steady_timer deadline_{
		_socket.get_executor(), std::chrono::seconds(60) };
};

