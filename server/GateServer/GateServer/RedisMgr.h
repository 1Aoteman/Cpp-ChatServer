#pragma once
#include "const.h"
#include "Singleton.h"
#include <sw/redis++/redis++.h>
class RedisConPool {
public:
    RedisConPool(size_t poolsize,const char* host,int port,const char* pwd);
    ~RedisConPool();
    void close();
    redisContext* Getconnect();
    void returnConn(redisContext *context);
private:
    size_t _pool_size;
    std::queue<redisContext*> _connections;
    const char* _host;
    int _port;
    std::atomic<bool> _b_stop;
    std::mutex _mutex;
    std::condition_variable _cond;
};

class RedisMgr:public Singleton<RedisMgr>,public std::enable_shared_from_this<RedisMgr>
{
    friend class Singleton<RedisMgr>;

public:
	~RedisMgr();
    bool Get(const std::string& key, std::string& value);
    bool Set(const std::string& key, const std::string& value);
    bool Auth(const std::string& password);
    bool LPush(const std::string& key, const std::string& value);
    bool LPop(const std::string& key, std::string& value);
    bool RPush(const std::string& key, const std::string& value);
    bool RPop(const std::string& key, std::string& value);
    bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    std::string HGet(const std::string& key, const std::string& hkey);
    bool Del(const std::string& key);
    bool ExistsKey(const std::string& key);
    void Close();
private:
    RedisMgr();
    redisContext* _connect;
    redisReply* _reply;
    std::unique_ptr<RedisConPool> _con_pool;
};

