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