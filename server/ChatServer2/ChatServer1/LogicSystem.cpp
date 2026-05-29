#include "LogicSystem.h"
#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"
#include "UserMgr.h"
#include "data.h"
#include "ChatGrpcClient.h"
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
	LoginRsp rsp = StatusGrpcClient::GetInstance()->Login(uid, token);

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

	rtvalue["error"] = rsp.error();
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

	std::string base_key = USER_BASE_INFO + std::to_string(apply_uid);
	auto apply_info = std::make_shared<UserInfo>();
	bool b_info = GetBaseInfo(base_key, apply_uid, apply_info);

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
	//如果不在使用grpc,需要申请人的全部信息
	
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
	MysqlMgr::GetInstance()->AuthFriendApply(from_uid,to_uid);
	//更新朋友表
	//更新数据库添加好友
	MysqlMgr::GetInstance()->AddFriend(from_uid, to_uid, back_name);
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
			std::string return_str = notify.toStyledString();
			session->Send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
		}
		return;
	}
	//如果不在同一个服务器，使用grpc

	AuthFriendReq auth_req;
	auth_req.set_fromuid(from_uid);
	auth_req.set_touid(to_uid);
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

