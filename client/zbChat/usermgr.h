#ifndef USERMGR_H
#define USERMGR_H
#include <QObject>
#include <memory>
#include <SIngleton.h>
#include "userdata.h"
#include <QMap>
#include <QJsonArray>
class UserMgr:public QObject,public Singleton<UserMgr>,
                public std::enable_shared_from_this<UserMgr>
{
    Q_OBJECT
public:
    friend class Singleton<UserMgr>;
    ~ UserMgr();
    void SetName(QString name);
    QString GetName();
    int GetUId();
    void SetUid(int uid);
    void SetToken(QString token);
    void SetUserInfo(std::shared_ptr<UserInfo> userinfo);
    std::shared_ptr<UserInfo> GetUserInfo();
    std::vector<std::shared_ptr<ApplyInfo>> GetApplyList();
    std::vector<std::shared_ptr<FriendInfo>> GetFriedList();
    void AppendApplyList(QJsonArray array);
    void AppendFriendList(QJsonArray array);
    void AppendFriendChatMsg(int uid,std::vector<std::shared_ptr<TextChatData>> msg);
    //判断是否已经发送了申请
    bool AlreadyApply(int uid);
    //发送的申请保存在内存中,这段逻辑可以使用map来保证速度，也可以两个都用
    void AddApplyList(std::shared_ptr<ApplyInfo> app);
    //判断是否已经是朋友
    bool CheckFriendById(int uid);
    void AddFriend(std::shared_ptr<AuthRsp> authrsp);
    void AddFriend(std::shared_ptr<AuthInfo> authinfo);
    std::shared_ptr<FriendInfo> GetFriendById(int uid);
private:
    UserMgr();
    QString _name;
    QString _token;
    std::shared_ptr<UserInfo> _user_info;
    int _uid;
    std::vector<std::shared_ptr<ApplyInfo>> _apply_list;
    std::vector<std::shared_ptr<FriendInfo>> _friend_list;
    //这个是当有信息传来时，方便查询
    QMap<int, std::shared_ptr<FriendInfo>> _friend_map;
};
#endif // USERMGR_H
