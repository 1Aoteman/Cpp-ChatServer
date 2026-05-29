#include "usermgr.h"
UserMgr::~UserMgr()
{
}
QString UserMgr::GetName(){
    return _user_info->_name;
}

int UserMgr::GetUId()
{
    return _user_info->_uid;
}
void UserMgr::SetName(QString name)
{
    _name = _name;
}
void UserMgr::SetUid(int uid)
{
    _user_info->_uid = uid;
}
void UserMgr::SetToken(QString token)
{
    _token = token;
}
void UserMgr::SetUserInfo(std::shared_ptr<UserInfo> userinfo){
    _user_info =userinfo;

}
std::vector<std::shared_ptr<ApplyInfo> > UserMgr::GetApplyList()
{
    return _apply_list;
}

void UserMgr::AppendApplyList(QJsonArray array)
{
    // 遍历 QJsonArray 并输出每个元素
    for (const QJsonValue &value : array) {
        auto name = value["name"].toString();
        auto desc = value["desc"].toString();
        auto icon = value["icon"].toString();
        auto nick = value["nick"].toString();
        auto sex = value["sex"].toInt();
        auto uid = value["uid"].toInt();
        auto status = value["status"].toInt();
        auto info = std::make_shared<ApplyInfo>(uid, name,
                                                desc, icon, nick, sex, status);
        _apply_list.push_back(info);
    }
}

void UserMgr::AppendFriendList(QJsonArray array)
{
    // 遍历 QJsonArray 并输出每个元素
    for (const QJsonValue& value : array) {
        auto name = value["name"].toString();
        auto desc = value["desc"].toString();
        auto icon = value["icon"].toString();
        auto nick = value["nick"].toString();
        auto sex = value["sex"].toInt();
        auto uid = value["uid"].toInt();
        auto back = value["back"].toString();

        auto info = std::make_shared<FriendInfo>(uid, name,
                                                 nick, icon, sex, desc, back);
        _friend_list.push_back(info);
        _friend_map.insert(uid, info);
    }
}

void UserMgr::AddApplyList(std::shared_ptr<ApplyInfo> app)
{
    _apply_list.push_back(app);
}

bool UserMgr::CheckFriendById(int uid)
{
    auto find_iter = _friend_map.find(uid);
    if(find_iter == _friend_map.end()){
        return false;
    }
    return true;

}

void UserMgr::AddFriend(std::shared_ptr<AuthRsp> authrsp)
{
    auto _friend_info = std::make_shared<FriendInfo>(authrsp);
    _friend_map[authrsp->_uid] = _friend_info;
}

void UserMgr::AddFriend(std::shared_ptr<AuthInfo> authinfo)
{
    auto _friend_info = std::make_shared<FriendInfo>(authinfo);
    _friend_map[authinfo->_uid] = _friend_info;
}
UserMgr::UserMgr()
{
}

//already,已经申请返回true
bool UserMgr::AlreadyApply(int uid)
{
    //查看是否已经添加过
    for(auto& app : _apply_list){
        if(uid==app->_uid){
            return true;
        }
    }
    return false;
}

