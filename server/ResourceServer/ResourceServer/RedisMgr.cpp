#include "RedisMgr.h"
#include "RedisMgr.h"
#include <iostream>
#include "Distlock.h"
#include "ConfigMgr.h"
RedisMgr::RedisMgr()
{
    auto& grpc = ConfigMgr::Inst();
    auto host = grpc["Redis"]["Host"];
    auto port = grpc["Redis"]["Port"];
    auto pwd = grpc["Redis"]["Passwd"];
    //加入分布式锁也会占用redis连接，将连接池数量
    _con_pool.reset(new RedisPool(10, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}
RedisMgr::~RedisMgr()
{
    Close();
}
bool RedisMgr::Get(const std::string& key, std::string& value)
{
    auto connect = _con_pool->GetConnection();//从连接池中取出一个
    if (connect == nullptr) {
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());
    if (reply == NULL) {
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    if (reply->type != REDIS_REPLY_STRING) {
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    value = reply->str;
    freeReplyObject(reply);
    std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
    _con_pool->ReturnConnnection(connect);//还连接
    return true;
}
bool RedisMgr::Set(const std::string& key, const std::string& value) {
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }
    //执行redis命令行
    auto reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());
    //如果返回NULL则说明执行失败
    if (NULL == reply)
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    //如果执行失败则释放连接
    if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0)))
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(reply);
    std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
    _con_pool->ReturnConnnection(connect);//还连接
    return true;
}

bool RedisMgr::Auth(const std::string& password)
{
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "AUTH %s", password.c_str());
    if (reply->type == REDIS_REPLY_ERROR) {
        std::cout << "认证失败" << std::endl;
        //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    else {
        //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
        freeReplyObject(reply);
        std::cout << "认证成功" << std::endl;
        _con_pool->ReturnConnnection(connect);//还连接
        return true;
    }
}

bool RedisMgr::LPush(const std::string& key, const std::string& value)
{
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "LPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == reply)
    {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->ReturnConnnection(connect);//还连接
    return true;
}

bool RedisMgr::LPop(const std::string& key, std::string& value) {
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "LPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ LPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->ReturnConnnection(connect);//还连接
    return true;
}
bool RedisMgr::RPush(const std::string& key, const std::string& value) {
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "RPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == reply)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->ReturnConnnection(connect);//还连接
    return true;
}
bool RedisMgr::RPop(const std::string& key, std::string& value) {
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "RPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->ReturnConnnection(connect);//还连接
    return true;
}

bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value) {
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->ReturnConnnection(connect);//还连接
    return true;
}
bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
    const char* argv[4];
    size_t argvlen[4];
    argv[0] = "HSET";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = strlen(key);
    argv[2] = hkey;
    argvlen[2] = strlen(hkey);
    argv[3] = hvalue;
    argvlen[3] = hvaluelen;
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }
    auto reply = (redisReply*)redisCommandArgv(connect, 4, argv, argvlen);
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] success ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->ReturnConnnection(connect);//还连接
    return true;
}
std::string RedisMgr::HGet(const std::string& key, const std::string& hkey)
{
    const char* argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return "";
    }
    auto reply = (redisReply*)redisCommandArgv(connect, 3, argv, argvlen);
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        freeReplyObject(reply);
        std::cout << "Execut command [ HGet " << key << " " << hkey << "  ] failure ! " << std::endl;
        _con_pool->ReturnConnnection(connect);//还连接
        return "";
    }
    std::string value = reply->str;
    freeReplyObject(reply);
    std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
    _con_pool->ReturnConnnection(connect);//还连接
    return value;
}
bool RedisMgr::HDel(const std::string& key, const std::string& field)
{
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }

    Defer defer([&connect, this]() {
        _con_pool->ReturnConnnection(connect);
        });

    redisReply* reply = (redisReply*)redisCommand(connect, "HDEL %s %s", key.c_str(), field.c_str());
    if (reply == nullptr) {
        std::cerr << "HDEL command failed" << std::endl;
        return false;
    }

    bool success = false;
    if (reply->type == REDIS_REPLY_INTEGER) {
        success = reply->integer > 0;
    }

    freeReplyObject(reply);
    return success;
}
bool RedisMgr::Del(const std::string& key)
{
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "DEL %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->ReturnConnnection(connect);//还连接
    return true;
}
bool RedisMgr::ExistsKey(const std::string& key)
{
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "exists %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) {
        std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);//还连接
        return false;
    }
    std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->ReturnConnnection(connect);//还连接
    return true;
}
void RedisMgr::Close()
{
    _con_pool->Close();
}

std::string RedisMgr::AcquireLock(std::string lockname,int locktimeout,int acquiretime)
{
    auto conn = _con_pool->GetConnection();
    if (conn == nullptr) {
        return "";
    }
    Defer defer([&conn, this]() {
        _con_pool->ReturnConnnection(std::move(conn));
        });
    return Distlock::Inst().acquirelock(conn,lockname, locktimeout, acquiretime);
    
}

bool RedisMgr::Releaselock(std::string lockname, std::string identifier)
{
    if (identifier.empty())
    {
        return true;
    }
    auto conn = _con_pool->GetConnection();
    if (conn == nullptr) {
        return false;
    }
    Defer defer([&conn, this]() {
        _con_pool->ReturnConnnection(std::move(conn));
        });
    return Distlock::Inst().releaselock(conn, lockname, identifier);
}

void RedisMgr::InitCount(std::string server_name)
{
    auto lock_key = LOCK_COUNT;
    auto identifier = RedisMgr::GetInstance()->AcquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
    //利用defer解锁
    Defer defer2([this, identifier, lock_key]() {
        RedisMgr::GetInstance()->Releaselock(lock_key, identifier);
        });

    RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, "0");
}
bool RedisMgr::SetFileInfo(const std::string& md5, std::shared_ptr<FileInfo> file_info)
{
    Json::Reader reader;
    Json::Value root;
    root["file_path_str"] = file_info->_file_path_str;
    root["name"] = file_info->_name;
    root["seq"] = file_info->_seq;
    root["total_size"] = file_info->_total_size;
    root["trans_size"] = file_info->_trans_size;
    auto file_info_str = root.toStyledString();
    auto redis_key = "file_upload_" + md5;
    bool success = SetExp(redis_key, file_info_str, 3600);
    return success;
}
std::shared_ptr<FileInfo> RedisMgr::GetFileInfo(const std::string& name) {
    auto redis_key = "file_upload_" + name;
    std::string file_info_str = "";

    // 从 Redis 获取数据
    bool success = Get(redis_key, file_info_str);
    if (!success || file_info_str.empty()) {
        return nullptr;
    }

    // 解析 JSON
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(file_info_str, root)) {
        std::cout << "Failed to parse file info JSON for name: " << name << std::endl;
        return nullptr;
    }

    // 创建 FileInfo 对象并填充数据
    auto file_info = std::make_shared<FileInfo>();
    try {
        file_info->_file_path_str = root["file_path_str"].asString();
        file_info->_name = root["name"].asString();
        file_info->_seq = root["seq"].asInt();
        file_info->_total_size = std::stoll(root["total_size"].asString());
        file_info->_trans_size = std::stoll(root["trans_size"].asString());
    }
    catch (const std::exception& e) {
        std::cout << "Error parsing file info fields for name " << name << ": " << e.what() << std::endl;
        return nullptr;
    }

    return file_info;
}
bool RedisMgr::SetExp(const std::string& key, const std::string& value, int expire_seconds) {
    //执行redis命令行
    auto connect = _con_pool->GetConnection();
    if (connect == nullptr) {
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "SETEX %s %d %s", key.c_str(),
        expire_seconds,
        value.c_str());

    if (NULL == reply) {
        std::cout << "Execute command [ SETEX " << key << " " << expire_seconds
            << " " << value << " ] failure ! " << std::endl;
        _con_pool->ReturnConnnection(connect);
        return false;
    }

    if (!(reply->type == REDIS_REPLY_STATUS &&
        (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0))) {
        std::cout << "Execute command [ SETEX " << key << " " << expire_seconds
            << " " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        _con_pool->ReturnConnnection(connect);
        return false;
    }

    freeReplyObject(reply);
    std::cout << "Execute command [ SETEX " << key << " " << expire_seconds
        << " " << value << " ] success ! " << std::endl;
    _con_pool->ReturnConnnection(connect);
    return true;
}
bool RedisMgr::SetDownLoadInfo(const std::string& name, std::shared_ptr<FileInfo> file_info) {
    Json::Reader reader;
    Json::Value root;
    root["file_path_str"] = file_info->_file_path_str;
    root["name"] = file_info->_name;
    root["seq"] = file_info->_seq;
    root["total_size"] = std::to_string(file_info->_total_size);
    root["trans_size"] = std::to_string(file_info->_trans_size);
    auto file_info_str = root.toStyledString();
    auto redis_key = "file_download_" + name;
    bool success = SetExp(redis_key, file_info_str, 3600);
    return success;
}

bool RedisMgr::DelDownLoadInfo(const std::string& name) {
    auto redis_key = "file_download_" + name;
    return Del(redis_key);
}


std::shared_ptr<FileInfo> RedisMgr::GetDownloadInfo(const std::string& name) {
    auto redis_key = "file_download_" + name;
    std::string file_info_str = "";

    // 从 Redis 获取数据
    bool success = Get(redis_key, file_info_str);
    if (!success || file_info_str.empty()) {
        return nullptr;
    }

    // 解析 JSON
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(file_info_str, root)) {
        std::cout << "Failed to parse file info JSON for name: " << name << std::endl;
        return nullptr;
    }

    // 创建 FileInfo 对象并填充数据
    auto file_info = std::make_shared<FileInfo>();
    try {
        file_info->_file_path_str = root["file_path_str"].asString();
        file_info->_name = root["name"].asString();
        file_info->_seq = root["seq"].asInt();
        file_info->_total_size = std::stoll(root["total_size"].asString());
        file_info->_trans_size = std::stoll(root["trans_size"].asString());
    }
    catch (const std::exception& e) {
        std::cout << "Error parsing file info fields for name " << name << ": " << e.what() << std::endl;
        return nullptr;
    }

    return file_info;
}
