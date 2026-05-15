#pragma once
#include "Singleton.h"
#include "const.h"
#include "CSession.h"
#include "json/json.h"
#include <map>
#include <queue>
#include "data.h"

typedef std::function<void(std::shared_ptr<CSession>, const short& msg_id, const std::string& msg_data)> FunCallBack;
class LogicSystem:public Singleton<LogicSystem>
{
public:
	LogicSystem();
	void LoginHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	void InitCallBack();
	void PostMsgToQue(std::shared_ptr<LogicNode> logicnode);
	void DealMsg();
private:
	std::map<short, FunCallBack> _fun_callbacks;
	std::atomic<bool> _b_stop;
	std::thread _work_thread;//工作线程。处理接受的消息
	std::condition_variable _cond;
	std::mutex _mutex;
	std::queue<std::shared_ptr<LogicNode>> _msg_que;
};

