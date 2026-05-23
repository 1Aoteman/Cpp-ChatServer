#pragma once
#include "const.h"
#include "Singleton.h"
#include <map>
#include <string>
class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> _handlers;
class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	bool HandleGet(std::string path, std::shared_ptr<HttpConnection> connection);
	bool HandlePost(std::string path, std::shared_ptr<HttpConnection> connection);
	void RegGet(std::string url, _handlers handler);
	void RegPost(std::string url, _handlers handler);
private:
	LogicSystem();
	std::map<std::string, _handlers> _post_handler;
	std::map<std::string, _handlers> _get_handler;
};

