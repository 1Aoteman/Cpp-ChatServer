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
	void DealHeartBeat(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	void DealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	void AddFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	void AuthFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	//搜索是判断是否是数字
	bool isPureDigit(std::string str);
	void GetUserByUid(std::string uid_str, Json::Value& rtroot);
	void GetUserByName(std::string name, Json::Value& rtroot);
	bool GetFriendApplyInfo(int uid, std::vector<std::shared_ptr<ApplyInfo>>& _apply_list);
	bool GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list);
	
	void GetUserThreadsHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	bool GetUserThreadsInfo(int uid, int last_thread_id, 
		int page_size, std::vector<std::shared_ptr<ChatThreadInfo>>& threads
		, bool& load_more, int64_t& next_last_id);
	void CreatePrivateChat(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	void LoadChatMsg(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	void DealChatImgMsg(std::shared_ptr<CSession> session,
		const short& msg_id, const std::string& msg_data);
private:
	std::map<short, FunCallBack> _fun_callbacks;
	std::atomic<bool> _b_stop;
	std::thread _work_thread;//工作线程。处理接受的消息
	std::condition_variable _cond;
	std::mutex _mutex;
	std::queue<std::shared_ptr<LogicNode>> _msg_que;
	//用来获取用户基本信息，先去redis中找，找不到再去数据库中找
	bool GetBaseInfo(std::string user_base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
};

