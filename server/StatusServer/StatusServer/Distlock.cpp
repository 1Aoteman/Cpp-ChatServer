#include "Distlock.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

std::string generateUUID() {
	boost::uuids::uuid uuid = boost::uuids::random_generator()();
	return to_string(uuid);
}
Distlock& Distlock::Inst() {
	static Distlock lock;
	return lock;
}

std::string Distlock::acquirelock(redisContext* context, std::string lockname, int locktimeout, int acquiretime)
{
	std::string lock_uuid = generateUUID();
	//设置锁的键
	std::string lock_key = "lock" + lockname;
	//设置枷锁的最长时间，如果超过就不加乐
	auto endtime = std::chrono::steady_clock::now() + std::chrono::seconds(acquiretime);
	//加上redis分布式锁
	while (std::chrono::steady_clock::now() < endtime) {
		//Nx代表只有key不存在的时候才进行设置，当其他客户端已经设置（即加上锁之后）防止覆盖原来的锁
		//Ex代表锁的有效期，超时后会自动释放掉
		redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s NX EX %d", lock_key.c_str(), lock_uuid.c_str(), locktimeout);
		if (reply != nullptr) {
			if (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK") {
				freeReplyObject(reply);
				return lock_uuid;
			}
			freeReplyObject(reply);
		}
		//加上限制，防止一直循环占据资源；
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return "";
}

bool Distlock::releaselock(redisContext* context, std::string lockname, const std::string lock_uuid)
{
	std::string lock_key = "lock" + lockname;
	//使用lua脚本保证原子性
	const char* luaScript ="if redis.call('get',KEYS[1])==ARGV[1] then \
								return redis.call('del',KEYS[1]) \
							else \
								return 0 \
							end";
	redisReply* reply = (redisReply*)redisCommand(context, "EVAL %s 1 %s %s", luaScript, lock_key.c_str(), lock_uuid.c_str());
	bool success = false;
	if (reply != nullptr) {
		if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
			success = true;
		}
		freeReplyObject(reply);
	}
	return success;
}
