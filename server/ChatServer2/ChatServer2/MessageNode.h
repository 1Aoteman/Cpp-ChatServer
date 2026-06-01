#pragma once
#include <iostream>
#include "const.h"
#include "boost/asio.hpp"
class MessageNode
{
public:
	MessageNode(short max_len) :_total_len(max_len), _cur_len(0) {
		_data = new char[max_len + 1]();//char*的字符串必须以/0结束
		_data[_total_len] = '/0';
	}
	~MessageNode() {
		std::cout << "消息节点析构" << std::endl;
		delete[] _data;
	}
	void Clear() {
		memset(_data, 0, _total_len);
		_cur_len = 0;
	}
	short _cur_len;//已经处理的长度
	short _total_len;
	char* _data;
};
class RecvMsgNode:public MessageNode {
public:
	RecvMsgNode(short maxlen,short msgid);
	short GetMsgid() {
		return _msg_id;
	}
private:
	short _msg_id;
};
class SendMsgNode :public MessageNode {
public:
	SendMsgNode(const char* msg,short maxlen, short msgid);
private:
	short GetMsgid() {
		return _msg_id;
	}
	short _msg_id;
};
