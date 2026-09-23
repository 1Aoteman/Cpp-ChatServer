#include "UserMgr.h"
#include "RedisMgr.h"

UserMgr::~UserMgr()
{
    _uid_to_session.clear();
}
std::shared_ptr<CSession> UserMgr::GetSession(int uid)
{
    std::unique_lock<std::mutex> lock(_session_mtx);
    auto find_iter = _uid_to_session.find(uid);
    if (find_iter == _uid_to_session.end()) {
        return nullptr;
    }
    else {
        return find_iter->second;
    }
}

void UserMgr::SetUserSession(int uid, std::shared_ptr<CSession> session)
{
    std::unique_lock<std::mutex> lock(_session_mtx);
    _uid_to_session[uid] = session;
}

void UserMgr::RevUserSession(int uid,std::string sessionid)
{
    std::lock_guard<std::mutex> lock(_session_mtx);
    //
    auto idter = _uid_to_session.find(uid);
    if (idter == _uid_to_session.end()) {
        return;
    }
    std::string session_uid = idter->second->GetSessionId();
    //如果两个session id不相等，说明并不是同一个设备上登陆
    if (session_uid != sessionid) {
        return;
    }
    _uid_to_session.erase(uid);
   
}
UserMgr::UserMgr() {

}
