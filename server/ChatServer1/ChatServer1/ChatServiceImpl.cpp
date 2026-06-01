#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"

ChatServiceImpl::ChatServiceImpl()
{

}

Status ChatServiceImpl::NotifyAddFriend(ServerContext* context, const AddFriendReq* request, AddFriendRsp* reply)
{
	//取出对应的session
	auto session = UserMgr::GetInstance()->GetSession(request->touid());
	Defer defer([&reply, &request] {
		reply->set_error(ErrorCodes::Success);
		reply->set_applyuid(request->applyuid());
		reply->set_touid(request->touid());
		});
	//判断是否还在内存中
	if (session == nullptr) {
		return Status::OK;
	}

	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["applyuid"] = request->applyuid();
	rtvalue["name"] = request->name();
	rtvalue["desc"] = request->desc();
	rtvalue["icon"] = request->icon();
	rtvalue["sex"] = request->sex();
	rtvalue["nick"] = request->nick();
	std::string return_str = rtvalue.toStyledString();
	session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
	std::cout << "申请添加朋友回应已经发送" << std::endl;
	return Status::OK;
}

Status ChatServiceImpl::NotifyAuthFriend(ServerContext* context, const AuthFriendReq* request, AuthFriendRsp* reply)
{
	auto to_uid = request->touid();
	auto from_uid = request->fromuid();
	auto session = UserMgr::GetInstance()->GetSession(to_uid);
	Defer defer([&request,&reply] {
		reply->set_error(ErrorCodes::Success);
		reply->set_fromuid(request->fromuid());
		reply->set_touid(request->touid());
		});
	//验证是否在内存中
	if(session == nullptr){
		return Status::OK;
	}
	//在内存中的话
	//在内存中则直接发送通知对方
	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["fromuid"] = request->fromuid();
	rtvalue["touid"] = request->touid();

	std::string base_key = USER_BASE_INFO + std::to_string(from_uid);
	auto user_info = std::make_shared<UserInfo>();
	bool b_info = GetBaseInfo(base_key, from_uid, user_info);
	if (b_info) {
		rtvalue["name"] = user_info->name;
		rtvalue["nick"] = user_info->nick;
		rtvalue["icon"] = user_info->icon;
		rtvalue["sex"] = user_info->sex;
	}
	else {
		rtvalue["error"] = ErrorCodes::UidInvalid;
	}
	std::string return_str = rtvalue.toStyledString();
	session->Send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
	return Status::OK;
	return Status();
}
Status ChatServiceImpl::NotifyTextChatMsg(ServerContext* context, const TextChatMsgReq* request, TextChatMsgRsp* reply)
{
	//检查用户是否在内存中
	int to_uid = request->touid();
	auto session = UserMgr::GetInstance()->GetSession(to_uid);
	reply->set_error(ErrorCodes::Success);
	if (session == nullptr) {
		return Status::OK;
	}
	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["fromuid"] = request->fromuid();
	rtvalue["touid"] = to_uid;
	//将聊天数据组织为数组
	Json::Value text_array;
	for (auto& msg : request->textmsgs()) {
		Json::Value element;
		element["content"] = msg.msgcontent();
		element["msgid"] = msg.msgid();
		text_array.append(element);
	}
	rtvalue["text_array"] = text_array;
	std::string return_str = rtvalue.toStyledString();
	session->Send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
	return Status::OK;

}
bool ChatServiceImpl::GetBaseInfo(std::string user_base_key, int uid, std::shared_ptr<UserInfo>& userinfo) {

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
		std::shared_ptr<UserInfo> user_info = nullptr;
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
