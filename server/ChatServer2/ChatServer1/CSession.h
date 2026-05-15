#pragma once
#include "boost/asio.hpp"
#include "const.h"
#include "MessageNode.h"
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <queue>
using boost::asio::ip::tcp;
class CServer;
class LogicSystem;

class CSession:public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context &ioc, CServer* server);
	tcp::socket& GetSocket();
	std::string& GetUuid();
	void Start();
	void  AsyncReadHead(int head_len);
	void AsyncReadBody(short msg_len);
	void AsyncReadFull(int max_len,std::function<void(boost::system::error_code err,size_t bytestransfered)>);
	void AsyncReadlen(int read_len, int total_len, std::function<void(boost::system::error_code err, size_t bytestransfered)>);
	void Close();
	void Send(std::string msg, int msg_id);//向客户端发送数据
	void HandleWrite(boost::system::error_code ec);
private:
	std::string _session_id;
	std::queue<std::shared_ptr<SendMsgNode>> _send_que;
	tcp::socket _socket;
	CServer* _cserver;
	//接收消息节点
	std::shared_ptr<RecvMsgNode> _recv_msg_node;
	std::shared_ptr<MessageNode> _recv_head_node;
	char _data[MAX_LENGTH];
	std::mutex _send_mutex;
};

class LogicNode {
	friend class LogicSystem;
public:
	LogicNode(std::shared_ptr<CSession> session,std::shared_ptr<RecvMsgNode> recvnoce);
	std::shared_ptr<RecvMsgNode> GetLogicNode() {
		return _recv_node;
	}
private:
	std::shared_ptr<CSession> _session;
	std::shared_ptr<RecvMsgNode> _recv_node;
};