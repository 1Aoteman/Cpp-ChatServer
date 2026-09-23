#pragma once
#include <sw/redis++/redis++.h>

class Distlock
{
public:
	static Distlock& Inst();
	~Distlock() = default;
	//redis分布式锁加锁操作，
	std::string acquirelock(redisContext* content, std::string lockname, int locktime, int acquiretime);
	bool releaselock(redisContext* content, std::string lockname, const std::string lock_uuid);
private:
	Distlock() = default;
};

