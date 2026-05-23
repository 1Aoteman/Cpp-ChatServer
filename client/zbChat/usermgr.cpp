#include "usermgr.h"
UserMgr::~UserMgr()
{
}
QString UserMgr::GetName(){
    return _name;
}

int UserMgr::GetUId()
{
    return _uid;
}
void UserMgr::SetName(QString name)
{
    _name = name;
}
void UserMgr::SetUid(int uid)
{
    _uid = uid;
}
void UserMgr::SetToken(QString token)
{
    _token = token;
}

std::vector<std::shared_ptr<ApplyInfo> > UserMgr::GetApplyList()
{
    return _apply_list;
}
UserMgr::UserMgr()
{
}
