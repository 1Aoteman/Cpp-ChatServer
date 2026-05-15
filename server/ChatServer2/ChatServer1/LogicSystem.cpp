#include "LogicSystem.h"
#include "StatusGrpcClient.h"
#include "MysqlMgr.h"
#include "data.h"
LogicSystem::LogicSystem():_b_stop(false)
{
	InitCallBack();
	
}

void LogicSystem::InitCallBack()
{
	_fun_callbacks[MSG_IDS::MSG_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}
void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> logicnode)
{
	//为了防止锁竞争，放开锁之后在唤醒
	bool shoud_notice = false;
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_msg_que.push(logicnode);
		//仅仅当队列大小有零变为1是唤醒一个
		if (_msg_que.size() == 1) {
			shoud_notice=true;
		}
	}
	if (shoud_notice) {
		_cond.notify_one();
	}

}
void LogicSystem::DealMsg()
{
	while (1) {
		std::unique_lock<std::mutex> lock(_mutex);
		//如果队列为空，并且服务器不停止，线程挂起等待消息放入队列中
		while (_msg_que.empty() && !_b_stop) {
			_cond.wait(lock);
		}
		//如果服务器要关闭，要讲对立中剩下的数据全部处理完之后在关闭
		if (_b_stop) {
			while (!_msg_que.empty()) {
				auto msg_node = _msg_que.front();
				std::cout<<"msg id is"<<msg_node->GetLogicNode()->GetMsgid()<<std::endl;
				auto it_call = _fun_callbacks.find(msg_node->GetLogicNode()->GetMsgid());
				if (it_call == _fun_callbacks.end()) {
					_msg_que.pop();
					return;
				}
				//注意string和char的区别。char数组必须以/0结束，读不到/0不会结束，为了防止读过界，传入总长度
				it_call->second(msg_node->_session, msg_node->GetLogicNode()->GetMsgid(),
					std::string(msg_node->GetLogicNode()->_data, msg_node->GetLogicNode()->_cur_len));
				_msg_que.pop();
			}
			break;
		}
		//按照队列顺序处理数据
		auto msg_node = _msg_que.front();
		std::cout << "msg id is" << msg_node->GetLogicNode()->GetMsgid() << std::endl;
		auto it_call = _fun_callbacks.find(msg_node->GetLogicNode()->GetMsgid());
		if (it_call == _fun_callbacks.end()) {
			_msg_que.pop();
			return;
		}
		//注意string和char的区别。char数组必须以/0结束，读不到/0不会结束，为了防止读过界，传入总长度
		it_call->second(msg_node->_session, msg_node->GetLogicNode()->GetMsgid(),
			std::string(msg_node->GetLogicNode()->_data, msg_node->GetLogicNode()->_cur_len));
		_msg_que.pop();
	}

}
void LogicSystem::LoginHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data) 
{
	Json::Value root;
	Json::Reader reader;
	//讲消息转为json格式
	reader.parse(msg_data, root);
	std::cout << "user login uid is  " << root["uid"].asInt() << " user token  is "
		<< root["token"].asString() << std::endl;
	int uid = root["uid"].asInt();
	std::string token = root["token"].asString();
	std::string return_str = root.toStyledString();
	LoginRsp rsp = StatusGrpcClient::GetInstance()->Login(uid, token);
	//session->Send(return_str, msg_id);
	Json::Value rtvalue;
	rtvalue["error"] = rsp.error();
	//如果回复不成功
	if (rtvalue["error"] != ErrorCodes::Success) {
		return;
	}

	auto find_iter = _users.find(uid);
	std::shared_ptr<UserInfo> user_info = nullptr;
	if (find_iter == _users.end()) {
		//查询数据库
		user_info = MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			rtvalue["error"] = ErrorCodes::UidInvalid;
			return;
		}
		_users[uid] = user_info;
	}
	else {
		user_info = find_iter->second;
	}
	rtvalue["uid"] = uid;
	rtvalue["token"] = rsp.token();
	rtvalue["name"] = user_info->name;
}
