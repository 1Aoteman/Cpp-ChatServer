#include "LogicSystem.h"
#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"
#include "UserMgr.h"
#include "data.h"
#include "ChatGrpcClient.h"
#include "message.pb.h"
#include "Utils.h"
LogicSystem::LogicSystem():_b_stop(false)
{
	InitCallBack();
	//使用一个线程专门处理消息
	_work_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::InitCallBack()
{
	_fun_callbacks[MSG_IDS::MSG_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_fun_callbacks[MSG_IDS::ID_SEARCH_USER_REQ]=[this](std::shared_ptr<CSession> session,const short msg_id,
		const std::string msg_data) {
			Json::Value root;
			Json::Reader reader;
			//将string传化为json格式
			bool parse = reader.parse(msg_data, root);
			std::string uid_str = root["uid"].asString();
			std::cout << "search uid is " << uid_str << std::endl;

			Json::Value rtroot;
			Defer defer([this,session,&rtroot] {
				std::string rt_str = rtroot.toStyledString();
				session->Send(rt_str, MSG_IDS::ID_SEARCH_USER_RSP);
				});
			bool b_digit = isPureDigit(uid_str);
			if (b_digit) {
				GetUserByUid(uid_str, rtroot);
			}
			else {
				GetUserByName(uid_str, rtroot);
			}
		};
	//用来处理添加朋友申请的逻辑处理
	_fun_callbacks[MSG_IDS::ID_ADD_FRIEND_REQ] = std::bind(&LogicSystem::AddFriendApply, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//用来验证朋友申请
	_fun_callbacks[MSG_IDS::ID_AUTH_FRIEND_REQ] = std::bind(&LogicSystem::AuthFriendApply, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//处理发送的消息
	_fun_callbacks[MSG_IDS::ID_TEXT_CHAT_MSG_REQ] = std::bind(&LogicSystem::DealChatTextMsg, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//心跳检测处理逻辑
	_fun_callbacks[MSG_IDS::ID_HEART_BEAT_REQ] = std::bind(&LogicSystem::DealHeartBeat, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_fun_callbacks[MSG_IDS::ID_LOAD_CHAT_THREAD_REQ]=std::bind(&LogicSystem::GetUserThreadsHandler, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_fun_callbacks[ID_CREATE_PRIVATE_CHAT_REQ] = std::bind(&LogicSystem::CreatePrivateChat, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_fun_callbacks[ID_LOAD_CHAT_MSG_REQ] = std::bind(&LogicSystem::LoadChatMsg, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_fun_callbacks[ID_IMG_CHAT_MSG_REQ] = std::bind(&LogicSystem::DealChatImgMsg, this,
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
					continue;
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
void LogicSystem::DealHeartBeat(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["fromuid"].asInt();
	std::cout << "receive heart beat msg, uid is " << uid << std::endl;
	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	session->Send(rtvalue.toStyledString(), ID_HEARTBEAT_RSP);
	//更新心跳检测的时间
	session->UpdateHeartTime();
}
void LogicSystem::DealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
	Json::Value root;
	Json::Reader reader;
	bool b_parse = reader.parse(msg_data, root);
	
	int uid = root["fromuid"].asInt();
	int touid = root["touid"].asInt();
	const Json::Value arrays = root["text_array"];

	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["fromuid"] = uid;
	rtvalue["touid"] = touid;
	auto thread_id = root["thread_id"].asInt();
	rtvalue["thread_id"] = thread_id;
	std::vector<std::shared_ptr<ChatMessage>> chat_datas;
	auto timestamp = getCurrentTimestamp();
	for (const auto& txt_obj : arrays) {
		auto content = txt_obj["content"].asString();
		auto unique_id = txt_obj["unique_id"].asString();
		std::cout << "content is " << content << std::endl;
		std::cout << "unique_id is " << unique_id << std::endl;
		auto chat_msg = std::make_shared<ChatMessage>();
		chat_msg->chat_time = timestamp;
		chat_msg->sender_id = uid;
		chat_msg->recv_id = touid;
		chat_msg->unique_id = unique_id;
		chat_msg->thread_id = thread_id;
		chat_msg->content = content;
		chat_msg->status = 2;
		chat_msg->msg_type = int(ChatMsgType::TEXT);
		chat_datas.push_back(chat_msg);
	}


	//插入数据库
	MysqlMgr::GetInstance()->AddChatMsg(chat_datas);


	for (const auto& chat_data : chat_datas) {
		Json::Value  chat_msg;
		chat_msg["message_id"] = chat_data->message_id;
		chat_msg["unique_id"] = chat_data->unique_id;
		chat_msg["content"] = chat_data->content;
		chat_msg["status"] = chat_data->status;
		chat_msg["chat_time"] = chat_data->chat_time;
		rtvalue["chat_datas"].append(chat_msg);
	}

	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_TEXT_CHAT_MSG_RSP);
		});


	//查询redis 查找touid对应的server ip
	auto to_str = std::to_string(touid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}

	auto& cfg = ConfigMgr::Inst();
	auto self_name = cfg["SelfServer"]["Name"];
	//直接通知对方有认证通过消息
	if (to_ip_value == self_name) {
		auto session = UserMgr::GetInstance()->GetSession(touid);
		if (session) {
			//在内存中则直接发送通知对方
			std::string return_str = rtvalue.toStyledString();
			session->Send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
		}

		return;
	}


	TextChatMsgReq text_msg_req;
	text_msg_req.set_fromuid(uid);
	text_msg_req.set_touid(touid);
	text_msg_req.set_thread_id(thread_id);
	for (const auto& chat_data : chat_datas) {
		auto* text_msg = text_msg_req.add_textmsgs();
		text_msg->set_unique_id(chat_data->unique_id);
		text_msg->set_msgcontent(chat_data->content);
		text_msg->set_msg_id(chat_data->message_id);
		text_msg->set_chat_time(chat_data->chat_time);
	}


	//发送通知 todo...
	ChatGrpcClient::GetInstance()->NotifyTextChatMsg(to_ip_value, text_msg_req, rtvalue);
}
//处理登录逻辑
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
	//LoginRsp rsp = StatusGrpcClient::GetInstance()->Login(uid, token);

	//session->Send(return_str, msg_id);
	Json::Value rtvalue;
	//使用defer，在结束时必定运行其中函数
	Defer defer([this, &rtvalue, session] {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, MSG_CHAT_LOGIN_RSP);
		});
	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	std::string token_value = "";
	//从redis中验证token；
	bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
	if (!success) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	if (token_value != token) {
		rtvalue["error"] = ErrorCodes::TokenInvalid;
		return;
	}
	rtvalue["error"] = ErrorCodes::Success;

	

	//如果回复不成功
	if (rtvalue["error"] != ErrorCodes::Success) {
		return;
	}
	//根据用户uid查询详细信息
	std::string user_base_key = USER_BASE_INFO + uid_str;
	auto user_info = std::make_shared<UserInfo>();
	bool _b_base = GetBaseInfo(user_base_key, uid, user_info);

	if (!_b_base) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	rtvalue["uid"] = uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;
	rtvalue["token"] = token;
	//从数据库中获取朋友申请列表
	std::vector<std::shared_ptr<ApplyInfo>> apply_list;
	auto b_apply = GetFriendApplyInfo(uid, apply_list);
	if (b_apply) {
		for (auto& apply : apply_list) {
			Json::Value obj;
			obj["name"] = apply->_name;
			obj["uid"] = apply->_uid;
			obj["icon"] = apply->_icon;
			obj["nick"] = apply->_nick;
			obj["sex"] = apply->_sex;
			obj["desc"] = apply->_desc;
			obj["status"] = apply->_status;
			rtvalue["apply_list"].append(obj);
		}
	}
	//获取好友列表
	std::vector<std::shared_ptr<UserInfo>> friend_list;
	bool b_friend_list = GetFriendList(uid, friend_list);
	for (auto& friend_ele : friend_list) {
		Json::Value obj;
		obj["name"] = friend_ele->name;
		obj["uid"] = friend_ele->uid;
		obj["icon"] = friend_ele->icon;
		obj["nick"] = friend_ele->nick;
		obj["sex"] = friend_ele->sex;
		obj["desc"] = friend_ele->desc;
		obj["back"] = friend_ele->back;
		rtvalue["friend_list"].append(obj);
	}
	auto self_server_name = ConfigMgr::Inst().GetValue("SelfServer", "Name");
	//加上作用域，}后锁会自动释放，来防止锁的资源消耗
	{
		//加入分布式锁
		auto lock_key = LOCK_PREFIX + uid_str;
		auto idebter = RedisMgr::GetInstance()->AcquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
		Defer defer2([this, lock_key, idebter] {
			RedisMgr::GetInstance()->Releaselock(lock_key, idebter);
			});
		//进行服务器踢人操作
		std::string ip_key = USERIPPREFIX + uid_str;
		std::string uid_ip_value = "";
		bool b_ip = RedisMgr::GetInstance()->Get(ip_key, uid_ip_value);
		if (b_ip) {
			auto& conf = ConfigMgr::Inst();
			auto self_ip_value = conf["SelfServer"]["Name"];
			//如果是同一个服务器
			if (uid_ip_value == self_ip_value) {
				//查找旧的连接
				auto old_session = UserMgr::GetInstance()->GetSession(uid);
				//判断是否在线。在线的话
				if (old_session) {
					old_session->NotifyOffline(uid);
					//清除旧的连接
					//_p_server->ClearSession(old_session->GetSessionId());
				}
			}
			//使用grpc跨服踢人
			else {
				KickUserRsp rsp;
				KickUserReq req;
				ChatGrpcClient::GetInstance()->NotifyKickUser(self_server_name, req);
			}
		}
	}
	auto server_name = ConfigMgr::Inst().GetValue("SelfServer", "Name");
	//将登录数量增加
	auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
	int count = 0;
	if (!rd_res.empty()) {
		count = std::stoi(rd_res);
	}
	count++;
	auto count_str = std::to_string(count);
	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);
	//session绑定用户uid
	session->SetUserId(uid);
	//为用户设置登录ip server的名字
	std::string  ipkey = USERIPPREFIX + uid_str;
	RedisMgr::GetInstance()->Set(ipkey, server_name);
	//uid和session绑定管理,方便以后踢人操作
	UserMgr::GetInstance()->SetUserSession(uid, session);
	return;
}
//添加朋友逻辑
void LogicSystem::AddFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
	Json::Value root;
	Json::Reader reader;
	//将string传化为json格式
	bool parse = reader.parse(msg_data, root);
	if (!parse) {
		return;
	}
	//申请人
	int apply_uid = root["uid"].asInt();
	std::string apply_name = root["applyname"].asString();
	//这是被申请人
	int back_uid = root["touid"].asInt();
	std::string back_name = root["backname"].asString();
	std::cout << "user login uid is  " << apply_uid << " applyname  is "
		<< apply_name << " bakname is " << back_name << " touid is " << back_uid << std::endl;
	//回应
	Json::Value rtvalue;
	//自动调用
	Defer defer([session,&rtvalue] {

		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_ADD_FRIEND_RSP);
		});
	MysqlMgr::GetInstance()->AddFriendApply(apply_uid,back_uid);

	//查询redis 查找touid对应的server ip
	auto to_str = std::to_string(back_uid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}
	std::cout << "to_ip_value " << to_ip_value << std::endl;
	auto& cfg = ConfigMgr::Inst();
	auto self_name = cfg["SelfServer"]["Name"];
	//如果实在同一个服务器中，直接通知对方有申请消息
	if (to_ip_value == self_name) {
		std::cout << "在同一个服务器中" << std::endl;
		auto session = UserMgr::GetInstance()->GetSession(back_uid);
		if (session) {
			//在内存中则直接发送通知对方
			Json::Value  notify;
			notify["error"] = ErrorCodes::Success;
			notify["applyuid"] = apply_uid;
			notify["name"] = apply_name;
			notify["desc"] = "";
			std::string return_str = notify.toStyledString();
			session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
		}
		return;
	}
	//如果不在,使用grpc,需要申请人的全部信息
	std::string base_key = USER_BASE_INFO + std::to_string(apply_uid);
	auto apply_info = std::make_shared<UserInfo>();
	bool b_info = GetBaseInfo(base_key,apply_uid,apply_info);

	message::AddFriendReq add_req;
	add_req.set_applyuid(apply_uid);
	add_req.set_touid(back_uid);
	add_req.set_name(apply_name);
	add_req.set_desc("");
	if (b_info) {
		add_req.set_icon(apply_info->icon);
		add_req.set_sex(apply_info->sex);
		add_req.set_nick(apply_info->nick);
	}
	ChatGrpcClient::GetInstance()->NotifyAddFriend(to_ip_value, add_req);
}
void LogicSystem::AuthFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
	Json::Value root;
	Json::Reader reader;
	reader.parse(msg_data, root);
	int from_uid = root["fromuid"].asInt();
	int to_uid = root["touid"].asInt();
	std::string back_name = root["backname"].asString();
	std::cout << "from_uid is " << from_uid << "to_uid is " << to_uid << "back_name is " << back_name << std::endl;

	Json::Value rtvalue;
	std::string base_key = USER_BASE_INFO + std::to_string(to_uid);
	std::shared_ptr<UserInfo> user_info=std::make_shared<UserInfo>();
	rtvalue["error"] = ErrorCodes::Success;
	bool b_info = GetBaseInfo(base_key,to_uid,user_info);
	if (b_info) {
		rtvalue["name"] = user_info->name;
		rtvalue["nick"] = user_info->nick;
		rtvalue["icon"] = user_info->icon;
		rtvalue["sex"] = user_info->sex;
		rtvalue["uid"] = to_uid;
	}
	else {
		rtvalue["error"] = ErrorCodes::UidInvalid;
	}
	Defer defer([this,&session,&rtvalue] {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, MSG_IDS::ID_AUTH_FRIEND_RSP);
		});
	//先更新数据库
	//MysqlMgr::GetInstance()->AuthFriendApply(from_uid,to_uid);
	std::vector<std::shared_ptr<message::AddFriendMsg>> chat_datas;
	//更新朋友表
	//更新数据库添加好友
	MysqlMgr::GetInstance()->AddFriend(from_uid, to_uid, back_name,chat_datas);
	//要给另一个客户端发送，先判断在哪个服务器上
	std::string uid_str = std::to_string(to_uid);
	std::string ip_key = USERIPPREFIX + uid_str;
	std::string ip_value = "";
	bool b_ip = RedisMgr::GetInstance()->Get(ip_key, ip_value);
	//反向思维，如果判断为真，条件较多,因此判断不成功，直接返回即可
	if (!b_ip) {
		return;
	}
	auto& con = ConfigMgr::Inst();
	std::string self_ip_name = con["SelfServer"]["Name"];
	//如果实在同一个服务器中
	if (ip_value == self_ip_name) {
		auto session = UserMgr::GetInstance()->GetSession(to_uid);
		//判断是否在内存中
		if (session) {
			//在内存中则直接发送通知对方
			Json::Value  notify;
			notify["error"] = ErrorCodes::Success;
			notify["fromuid"] = from_uid;
			notify["touid"] = to_uid;
			std::string base_key = USER_BASE_INFO + std::to_string(from_uid);
			auto user_info = std::make_shared<UserInfo>();
			bool b_info = GetBaseInfo(base_key, from_uid, user_info);
			if (b_info) {
				notify["name"] = user_info->name;
				notify["nick"] = user_info->nick;
				notify["icon"] = user_info->icon;
				notify["sex"] = user_info->sex;
			}
			else {
				notify["error"] = ErrorCodes::UidInvalid;
			}
			for (auto& chat_data : chat_datas)
			{
				Json::Value  chat;
				chat["sender"] = chat_data->sender_id();
				chat["msg_id"] = chat_data->msg_id();
				chat["thread_id"] = chat_data->thread_id();
				chat["unique_id"] = chat_data->unique_id();
				chat["msg_content"] = chat_data->msgcontent();
				notify["chat_datas"].append(chat);
				rtvalue["chat_datas"].append(chat);
			}
			std::string return_str = notify.toStyledString();
			session->Send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
		}
		return;
	}
	//如果不在同一个服务器，使用grpc

	AuthFriendReq auth_req;
	auth_req.set_fromuid(from_uid);
	auth_req.set_touid(to_uid);
	for (auto& chat_data : chat_datas)
	{
		auto text_msg = auth_req.add_textmsgs();
		text_msg->CopyFrom(*chat_data);
		Json::Value  chat;
		chat["sender"] = chat_data->sender_id();
		chat["msg_id"] = chat_data->msg_id();
		chat["thread_id"] = chat_data->thread_id();
		chat["unique_id"] = chat_data->unique_id();
		chat["msg_content"] = chat_data->msgcontent();
		rtvalue["chat_datas"].append(chat);
	}
	ChatGrpcClient::GetInstance()->NotifyAuthFriend(ip_value, auth_req);

}
//验证查询条件是id还是name
bool LogicSystem::isPureDigit(std::string str)
{
	//判断查询条件是uid还是name
	for (auto& c : str) {
		if (!std::isdigit(c)) {
			return false;
		}
	}
	return true;
}
void LogicSystem::GetUserByUid(std::string uid_str, Json::Value& rtroot)
{
	rtroot["error"] = ErrorCodes::Success;
	std::string base_key_str = USER_BASE_INFO + uid_str;
	std::string user_str="";
	//先去redis中查询
	bool _b_base = RedisMgr::GetInstance()->Get(base_key_str, user_str);
	if (_b_base) {
		//redis中存放的是string类型的，先转为json
		Json::Reader reader;
		Json::Value root;
		reader.parse(user_str, root);
		auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pwd = root["pwd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		auto icon = root["icon"].asString();
		std::cout << "user  uid is  " << uid << " name  is "
			<< name << " pwd is " << pwd << " email is " << email << " icon is " << icon << std::endl;

		rtroot["uid"] = uid;
		rtroot["pwd"] = pwd;
		rtroot["name"] = name;
		rtroot["email"] = email;
		rtroot["nick"] = nick;
		rtroot["desc"] = desc;
		rtroot["sex"] = sex;
		rtroot["icon"] = icon;
		return;
	}
	//redis中没有去数据库中找
	int uid = std::stoi(uid_str);
	std::shared_ptr<UserInfo> user_info = MysqlMgr::GetInstance()->GetUser(uid);
	if (user_info == nullptr) {
		rtroot["error"] = ErrorCodes::UidInvalid;
		return;
	}
	//将用户数据放入redis中，方便下次
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pwd"] = user_info->pwd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;
	redis_root["icon"] = user_info->icon;

	RedisMgr::GetInstance()->Set(base_key_str, redis_root.toStyledString());

	//返回数据
	rtroot["uid"] = user_info->uid;
	rtroot["pwd"] = user_info->pwd;
	rtroot["name"] = user_info->name;
	rtroot["email"] = user_info->email;
	rtroot["nick"] = user_info->nick;
	rtroot["desc"] = user_info->desc;
	rtroot["sex"] = user_info->sex;
	rtroot["icon"] = user_info->icon;
}
void LogicSystem::GetUserByName(std::string name, Json::Value& rtroot)
{
	rtroot["error"] = ErrorCodes::Success;
	std::string name_key_str = NAME_INFO + name;
	std::string user_str = "";
	//先去redis中查询
	bool _b_base = RedisMgr::GetInstance()->Get(name_key_str, user_str);
	if (_b_base) {
		//redis中存放的是string类型的，先转为json
		Json::Reader reader;
		Json::Value root;
		reader.parse(user_str, root);
		auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pwd = root["pwd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		auto icon = root["icon"].asString();
		std::cout << "user  uid is  " << uid << " name  is "
			<< name << " pwd is " << pwd << " email is " << email << " icon is " << icon << std::endl;

		rtroot["uid"] = uid;
		rtroot["pwd"] = pwd;
		rtroot["name"] = name;
		rtroot["email"] = email;
		rtroot["nick"] = nick;
		rtroot["desc"] = desc;
		rtroot["sex"] = sex;
		rtroot["icon"] = icon;
		return;
	}
	//redis中没有去数据库中找
	std::shared_ptr<UserInfo> user_info = MysqlMgr::GetInstance()->GetUser(name);
	if (user_info == nullptr) {
		rtroot["error"] = ErrorCodes::UidInvalid;
		return;
	}
	//将用户数据放入redis中，方便下次
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pwd"] = user_info->pwd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;
	redis_root["icon"] = user_info->icon;

	RedisMgr::GetInstance()->Set(name_key_str, redis_root.toStyledString());

	//返回数据
	rtroot["uid"] = user_info->uid;
	rtroot["pwd"] = user_info->pwd;
	rtroot["name"] = user_info->name;
	rtroot["email"] = user_info->email;
	rtroot["nick"] = user_info->nick;
	rtroot["desc"] = user_info->desc;
	rtroot["sex"] = user_info->sex;
	rtroot["icon"] = user_info->icon;
}
bool LogicSystem::GetFriendApplyInfo(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& _apply_list)
{
	return MysqlMgr::GetInstance()->GetApplyList(to_uid,_apply_list,0,10);
}
bool LogicSystem::GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list)
{
	return  MysqlMgr::GetInstance()->GetFriendList(self_id, user_list);
}
//获取用户的会话列表
void LogicSystem::GetUserThreadsHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
	Json::Value root;
	Json::Reader reader;
	reader.parse(msg_data, root);
	int uid = root["uid"].asInt();
	int last_thread_id = root["thread_id"].asInt();
	std::cout << "get uid thread " << uid << std::endl;

	//
	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["uid"] = std::to_string(uid);
	Defer defer([&rtvalue,this,&session] {
		std::string  data = rtvalue.toStyledString();
		session->Send(data,ID_LOAD_CHAT_THREAD_RSP);
		});
	//去数据库中查询消息列表
	std::vector<std::shared_ptr<ChatThreadInfo>> chat_threads;
	int page_size = 10;
	bool load_more = false;
	int64_t next_last_id = 0;
	bool res = GetUserThreadsInfo(uid,last_thread_id,page_size,chat_threads,load_more,next_last_id);
	if (!res) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	rtvalue["load_more"] = load_more;
	rtvalue["next_last_id"] = (int)next_last_id;

	for (auto thread : chat_threads) {
		Json::Value thread_value;
		thread_value["thread_id"] = thread->thread_id;
		thread_value["type"] = thread->thread_type;
		thread_value["user1_id"] = thread->user1_id;
		thread_value["user2_id"] = thread->user2_id;
		rtvalue["threads"].append(thread_value);
	}
}
//去数据库中进行查询
bool LogicSystem::GetUserThreadsInfo(int uid, int last_thread_id,int page_size,std::vector<std::shared_ptr<ChatThreadInfo>>& threads
,bool& load_more,int64_t& next_last_id)
{
	return MysqlMgr::GetInstance()->GetChatThreads(uid,last_thread_id,page_size,threads,load_more,next_last_id);
}

void LogicSystem::CreatePrivateChat(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
	std::cout << "运行到了回调" << std::endl;
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto other_id = root["other_id"].asInt();

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["uid"] = uid;
	rtvalue["other_id"] = other_id;

	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_CREATE_PRIVATE_CHAT_RSP);
		});

	int thread_id = 0;
	bool res = MysqlMgr::GetInstance()->CreatePrivateChat(uid, other_id, thread_id);
	if (!res) {
		rtvalue["error"] = ErrorCodes::CREATE_CHAT_FAILED;
		return;
	}

	rtvalue["thread_id"] = thread_id;
}

void LogicSystem::LoadChatMsg(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto thread_id = root["thread_id"].asInt();
	auto message_id = root["message_id"].asInt();


	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["thread_id"] = thread_id;

	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_LOAD_CHAT_MSG_RSP);
		});

	int page_size = 10;
	std::shared_ptr<PageResult> res = MysqlMgr::GetInstance()->LoadChatMsg(thread_id, message_id, page_size);
	if (!res) {
		rtvalue["error"] = ErrorCodes::LOAD_CHAT_FAILED;
		return;
	}

	rtvalue["last_message_id"] = res->next_cursor;
	rtvalue["load_more"] = res->load_more;
	for (auto& chat : res->messages) {
		Json::Value  chat_data;
		chat_data["sender"] = chat.sender_id;
		chat_data["msg_id"] = chat.message_id;
		chat_data["thread_id"] = chat.thread_id;
		chat_data["unique_id"] = 0;
		chat_data["msg_content"] = chat.content;
		chat_data["chat_time"] = chat.chat_time;
		chat_data["status"] = chat.status;
		chat_data["msg_type"] = chat.msg_type;
		chat_data["receiver"] = chat.recv_id;
		rtvalue["chat_datas"].append(chat_data);
	}
}
void LogicSystem::DealChatImgMsg(std::shared_ptr<CSession> session,
	const short& msg_id, const std::string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);

	auto uid = root["fromuid"].asInt();
	auto touid = root["touid"].asInt();

	auto md5 = root["md5"].asString();
	auto unique_name = root["name"].asString();
	auto token = root["token"].asString();
	auto unique_id = root["unique_id"].asString();
	auto chat_time = root["chat_time"].asString();
	auto status = root["status"].asInt();

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;

	rtvalue["fromuid"] = uid;
	rtvalue["touid"] = touid;
	auto thread_id = root["thread_id"].asInt();
	rtvalue["thread_id"] = thread_id;
	rtvalue["md5"] = md5;
	rtvalue["unique_name"] = unique_name;
	rtvalue["unique_id"] = unique_id;
	rtvalue["chat_time"] = chat_time;
	rtvalue["status"] = status;

	auto timestamp = getCurrentTimestamp();
	auto chat_msg = std::make_shared<ChatMessage>();
	chat_msg->chat_time = timestamp;
	chat_msg->sender_id = uid;
	chat_msg->recv_id = touid;
	chat_msg->unique_id = unique_id;
	chat_msg->thread_id = thread_id;
	chat_msg->content = unique_name;
	chat_msg->status = MsgStatus::UN_UPLOAD;
	chat_msg->msg_type= int(ChatMsgType::PIC);


	//插入数据库
	MysqlMgr::GetInstance()->AddChatMsg(chat_msg);
	
	rtvalue["message_id"] = chat_msg->message_id;

	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_IMG_CHAT_MSG_RSP);
		});

	//发送通知 todo... 以后等文件上传完成再通知
}
bool LogicSystem::GetBaseInfo(std::string user_base_key, int uid, std::shared_ptr<UserInfo>& userinfo) {

	std::cout << "执行到了redis验证环节" << std::endl;
	std::string info_str = "";
	bool rd_success = RedisMgr::GetInstance()->Get(user_base_key, info_str);
	//如果可以在redis中查到
	if (rd_success) {
		Json::Value root;
		Json::Reader read;
		read.parse(info_str, root);
		userinfo->uid = root["uid"].asInt();
		userinfo->name = root["name"].asString();
		userinfo->pwd = root["pwd"].asString();
		userinfo->email = root["email"].asString();
		userinfo->nick = root["nick"].asString();
		userinfo->desc = root["desc"].asString();
		userinfo->sex = root["sex"].asInt();
		userinfo->icon = root["icon"].asString();
		std::cout << "user login uid is  " << userinfo->uid << " name  is "
			<< userinfo->name << " pwd is " << userinfo->pwd << " email is " << userinfo->email << std::endl;
	}
	else //redis中没有去mysql中找，找到了放进redis中；
	{
		std::shared_ptr<UserInfo> user_info =nullptr;
		user_info = MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}
		userinfo = user_info;
		//放入redis中方便下次登录
		Json::Value root;
		Json::Value redis_root;
		redis_root["uid"] = uid;
		redis_root["pwd"] = userinfo->pwd;
		redis_root["name"] = userinfo->name;
		redis_root["email"] = userinfo->email;
		redis_root["nick"] = userinfo->nick;
		redis_root["desc"] = userinfo->desc;
		redis_root["sex"] = userinfo->sex;
		redis_root["icon"] = userinfo->icon;
		RedisMgr::GetInstance()->Set(user_base_key, redis_root.toStyledString());
		
	}
	return true;
}

