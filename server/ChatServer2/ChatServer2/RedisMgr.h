#pragma once
#include <queue>
#include "Singleton.h"
#include <sw/redis++/redis++.h>
#include "const.h"
class RedisPool {
public:
    RedisPool(size_t poolsize, const char* host, int port, const char* pwd) {
        for (int i = 0; i < poolsize; i++)
        {
            auto* context = redisConnect(host, port);
            if (context == nullptr || context->err != 0)
            {
                if (context == nullptr)
                {
                    redisFree(context);
                }
                continue;
            }
            auto reply = (redisReply*)redisCommand(context, "Auth %s", pwd);//jinxingrenzheng
            if (reply->type == REDIS_REPLY_ERROR)
            {
                std::cout << "认证失败" << std::endl;
                freeReplyObject(reply);
                continue;
            }
            freeReplyObject(reply);
            std::cout << "认证成功" << std::endl;
            _connections.push(context);
        }
    }
    void Close() {
        _b_stop = true;
        _cond.notify_all();
    }
    redisContext* GetConnection() {
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait(lock, [this] {
            if (_b_stop)
            {
                return true;
            }
            return !_connections.empty();
            });
        if (_b_stop)
        {
            return nullptr;
        }
        auto* context = _connections.front();
        _connections.pop();
        return context;
    }
    void ReturnConnnection(redisContext* context) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_b_stop)
        {
            return;
        }
        _connections.push(context);
        _cond.notify_one();
    }
    ~RedisPool() {
        std::lock_guard<std::mutex> lock(_mutex);
        while (!_connections.empty()) {
            _connections.pop();
        }
    }
private:
    size_t _pool_size;
    std::atomic<bool> _b_stop;
    const char* _host;
    int _port;
    std::mutex _mutex;
    std::queue <redisContext*> _connections;
    std::condition_variable _cond;
};
class RedisMgr :public Singleton<RedisMgr>, public std::enable_shared_from_this<RedisMgr>
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
    bool HDel(const std::string& key, const std::string& field);
    bool Del(const std::string& key);
    bool ExistsKey(const std::string& key);
    void Close();
private:
    RedisMgr();
    redisContext* _connect;
    redisReply* _reply;
    std::unique_ptr<RedisPool> _con_pool;
};


