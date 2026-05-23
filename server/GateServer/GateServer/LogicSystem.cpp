#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VarifyGrpcClient.h"
#include "RedisMgr.h"
#include "MySqlMgr.h"
#include "StatusGrpcClient.h"
bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> connection)
{
	if (_get_handler.find(path) == _get_handler.end())
	{
		return false;
	}
	_get_handler[path](connection);
	return true;
}
bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> connection)
{
	if (_post_handler.find(path) == _post_handler.end())
	{
		return false;
	}
	_post_handler[path](connection);
	return true;
}

void LogicSystem::RegGet(std::string url, _handlers handler)
{
	_get_handler.insert(make_pair(url, handler));//将get请求以及回调函数加入map；
}
void LogicSystem::RegPost(std::string url, _handlers handler)
{
	_post_handler.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection)
		{
			beast::ostream(connection->_response.body()) << "receive get_test req";
			int i = 0;
			for (auto& elem : connection->_get_params) {
				i++;
				beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
				beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
			}
		});
	RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = beast::buffers_to_string(connection->_request.body().data());
		std::cout << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success)
		{
			std::cout << "Failed parse to Json" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		auto email = src_root["email"].asString();
		GetVarifyRsp reply = VarifyGrpcClient::GetInstance()->GetVarifyCode(email);
		std::cout << "email is " << email <<std::endl;
		root["error"] = 0;
		root["email"] = src_root["email"];
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
		});
	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = beast::buffers_to_string(connection->_request.body().data());
		std::cout << "/user_register request body is " << body_str << std::endl;
		//设置response
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool _parse = reader.parse(body_str, src_root);
		if (!_parse) {
			std::cout << "Json parse failed" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string reponse_str = root.toStyledString();
			beast::ostream(connection->_response.body()) << reponse_str;
			return true;
		}
		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["pass"].asString();
		auto confirm = src_root["confirm"].asString();

		//判断验证码是否一样
		std::string varify_code;
		bool _b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX+src_root["email"].asString(), varify_code);
		if (!_b_get_varify)
		{
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		if (varify_code != src_root["varifycode"].asString()) {
			std::cout << "  varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		//检查用户是否存在
		bool _b_user = RedisMgr::GetInstance()->ExistsKey(src_root["user"].asString()); 
		if(_b_user)
		{
			std::cout << "  user exist" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		//查找数据库判断用户是否存在
		int uid = MySqlMgr::GetInstance()->UserReg(name, email, pwd);
		if (uid == 0 || uid == -1) {
			std::cout << " user or email exist" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		root["error"] = 0;
		root["email"] = email;
		root["user"] = name;
		root["password"] = pwd;
		root["confirm"] = confirm;
		root["varifycode"] = src_root["varifycode"].asString();
		std::string json_str = root.toStyledString();
		beast::ostream(connection->_response.body()) << json_str;
		return true;
		});
	RegPost("/reset_pwd", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = beast::buffers_to_string(connection->_request.body().data());
		//设置回复
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["pass"].asString();
		//先查找redis中email对应的验证码是否合理
		std::string  varify_code;
		bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX + src_root["email"].asString(), varify_code);
		if (!b_get_varify) {
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		if (varify_code != src_root["varifycode"].asString()) {
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		//查询数据库判断用户名和邮箱是否匹配
		bool email_valid = MySqlMgr::GetInstance()->CheckEmail(name, email);
		if (!email_valid) {
			std::cout << " user email not match" << std::endl;
			root["error"] = ErrorCodes::EmailNotMatch;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		//更新密码为最新密码
		bool b_up = MySqlMgr::GetInstance()->UndatedPwd(name, pwd);
		if (!b_up) {
			std::cout << " update pwd failed" << std::endl;
			root["error"] = ErrorCodes::PasswdUpFailed;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		std::cout << "succeed to update password" << pwd << std::endl;
		root["error"] = 0;
		root["email"] = email;
		root["user"] = name;
		root["passwd"] = pwd;
		root["varifycode"] = src_root["varifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
		});
	RegPost("/user_login", [](std::shared_ptr<HttpConnection> conn) {
			std::string body_str = beast::buffers_to_string(conn->_request.body().data());
			//设置回复
			conn->_response.set(http::field::content_type, "text/json");
			Json::Value root;
			Json::Reader reader;
			Json::Value src_root;
			//读取json
			bool parse_success = reader.parse(body_str, src_root);
			//如果解析失败
			if (!parse_success) {
				std::cout << "json parse failed" << std::endl;
				root["error"] = ErrorCodes::Error_Json;
				std::string json_str = root.toStyledString();
				beast::ostream(conn->_response.body()) << json_str;
				return false;
			}
			//登录传入emial和pass两个参数
			std::string email = src_root["email"].asString();
			std::string password = src_root["pass"].asString();
			UserInfo userInfo;
			bool _b_check_pwd = MySqlMgr::GetInstance()->CheckPwd(email, password,userInfo);
			if (!_b_check_pwd) {
				std::cout << "密码与用户不匹配" << std::endl;
				root["error"] = ErrorCodes::PasswdErr;
				std::string json_str = root.toStyledString();
				beast::ostream(conn->_response.body()) << json_str;
				return true;
			}
			//查询StatusServer找到合适的连接
			auto reply = StatusGrpcClient::GetInstance()->GetChatServer(userInfo.uid);
			if (reply.error()) {
				std::cout << " grpc get chat server failed, error is " << reply.error() << std::endl;
				root["error"] = ErrorCodes::RPCGetFailed;
				std::string jsonstr = root.toStyledString();
				beast::ostream(conn->_response.body()) << jsonstr;
				return true;
			}
			std::cout << "succeed to load userinfo uid is " << userInfo.uid << std::endl;
			root["error"] = 0;
			root["email"] = email;
			root["uid"] = userInfo.uid;
			root["token"] = reply.token();
			root["host"] = reply.host();
			root["port"] = reply.port();
			std::string jsonstr = root.toStyledString();
			std::cout << jsonstr << std::endl;
			beast::ostream(conn->_response.body()) << jsonstr;
			return true;
			});
}
