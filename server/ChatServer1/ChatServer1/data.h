#pragma once
#include <string>
struct UserInfo {
	UserInfo() :name(""), pwd(""), uid(0), email(""), nick(""), desc(""), sex(0), icon(""), back("") {}
	std::string name;
	std::string pwd;
	int uid;
	std::string email;
	std::string nick;
	std::string desc;
	int sex;
	std::string icon;
	std::string back;
};
struct ApplyInfo {
	ApplyInfo(int uid, std::string name, std::string nick, std::string desc, int sex, std::string icon, int status) :_uid(uid),
		_name(name),_nick(nick),_desc(desc),_sex(sex),_icon(icon),_status(status)
	{

	}
	int _uid;
	std::string _name;
	std::string _nick;
	std::string _desc;
	int _sex;
	std::string _icon;
	int _status;
};
struct ChatThreadInfo {
	int thread_id;
	std::string thread_type;         //对用私聊和群聊“private”和："group"
	int user1_id;					//群聊为0
	int user2_id;
};
//聊天消息信息
struct ChatMessage {
	int message_id;
	int thread_id;
	int sender_id;
	int recv_id;
	std::string unique_id;
	std::string content;
	std::string chat_time;
	int status;
	int msg_type;
};
struct PageResult {
	std::vector<ChatMessage> messages;
	bool load_more;
	int next_cursor;  // 本页最后一条message_id，用于下次查询
};
enum class ChatMsgType {
	TEXT = 0,
	PIC = 1,
	VIDEO = 2,
	FILE = 3
};