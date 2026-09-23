#ifndef USERDATA_H
#define USERDATA_H
#include <QString>
#include <memory>
#include <QJsonArray>
#include <vector>
#include <QJsonObject>
#include "global.h"

class SearchInfo {
public:
    SearchInfo(int uid, QString name, QString nick, QString desc, int sex,QString icon);
    int _uid;
    QString _name;
    QString _nick;
    QString _desc;
    int _sex;
    QString _icon;
};
Q_DECLARE_METATYPE(SearchInfo)
Q_DECLARE_METATYPE(std::shared_ptr<SearchInfo>)

class AddFriendApply {
public:
    AddFriendApply(int from_uid, QString name, QString desc,
                   QString icon, QString nick, int sex);
    int _from_uid;
    QString _name;
    QString _desc;
    QString _icon;
    QString _nick;
    int     _sex;
};
Q_DECLARE_METATYPE(std::shared_ptr<AddFriendApply>)

struct ApplyInfo {
    ApplyInfo(int uid, QString name, QString desc,
              QString icon, QString nick, int sex, int status)
        :_uid(uid),_name(name),_desc(desc),
        _icon(icon),_nick(nick),_sex(sex),_status(status){}

    ApplyInfo(std::shared_ptr<AddFriendApply> addinfo)
        :_uid(addinfo->_from_uid),_name(addinfo->_name),
        _desc(addinfo->_desc),_icon(addinfo->_icon),
        _nick(addinfo->_nick),_sex(addinfo->_sex),
        _status(0)
    {}
    void SetIcon(QString head){
        _icon = head;
    }
    int _uid;
    QString _name;
    QString _desc;
    QString _icon;
    QString _nick;
    int _sex;
    int _status;
};
struct TextChatData;
struct AuthInfo {
    AuthInfo(int uid, QString name,
             QString nick, QString icon, int sex):
        _uid(uid), _name(name), _nick(nick), _icon(icon),
        _sex(sex){}
    void SetChatDatas(std::vector<std::shared_ptr<TextChatData>> _chat_datas);
    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
    int _thread_id;
    std::vector<std::shared_ptr<TextChatData>> _chat_datas;
};
Q_DECLARE_METATYPE(std::shared_ptr<AuthInfo>)
struct AuthRsp {
    AuthRsp(int peer_uid, QString peer_name,
            QString peer_nick, QString peer_icon, int peer_sex)
        :_uid(peer_uid),_name(peer_name),_nick(peer_nick),
        _icon(peer_icon),_sex(peer_sex)
    {}

    void SetChatDatas(std::vector<std::shared_ptr<TextChatData>> _chat_datas);
    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
    int _thread_id;
    std::vector<std::shared_ptr<TextChatData>> _chat_datas;
};
Q_DECLARE_METATYPE(std::shared_ptr<AuthRsp>)
struct FriendInfo {
    FriendInfo(int uid, QString name, QString nick, QString icon,
               int sex, QString desc, QString back, QString last_msg=""):_uid(uid),
        _name(name),_nick(nick),_icon(icon),_sex(sex),_desc(desc),
        _back(back),_last_msg(last_msg){}

    FriendInfo(std::shared_ptr<AuthInfo> auth_info):_uid(auth_info->_uid),
        _nick(auth_info->_nick),_icon(auth_info->_icon),_name(auth_info->_name),
        _sex(auth_info->_sex){}

    FriendInfo(std::shared_ptr<AuthRsp> auth_rsp):_uid(auth_rsp->_uid),
        _nick(auth_rsp->_nick),_icon(auth_rsp->_icon),_name(auth_rsp->_name),
        _sex(auth_rsp->_sex){}

    void AppendChatMsgs(const std::vector<std::shared_ptr<TextChatData>> text_vec);

    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
    QString _desc;
    QString _back;
    QString _last_msg;
    std::vector<std::shared_ptr<TextChatData>> _chat_msgs;
};

struct UserInfo {
    UserInfo(int uid, QString name, QString nick, QString icon, int sex, QString last_msg = "", QString desc=""):
        _uid(uid),_name(name),_nick(nick),_icon(icon),_sex(sex),_last_msg(last_msg),_desc(desc){}

    UserInfo(std::shared_ptr<AuthInfo> auth):
        _uid(auth->_uid),_name(auth->_name),_nick(auth->_nick),
        _icon(auth->_icon),_sex(auth->_sex),_last_msg(""),_desc(""){}

    UserInfo(int uid, QString name, QString icon):
        _uid(uid), _name(name), _icon(icon),_nick(_name),
        _sex(0),_last_msg(""),_desc(""){

    }

    UserInfo(std::shared_ptr<AuthRsp> auth):
        _uid(auth->_uid),_name(auth->_name),_nick(auth->_nick),
        _icon(auth->_icon),_sex(auth->_sex),_last_msg(""){}

    UserInfo(std::shared_ptr<SearchInfo> search_info):
        _uid(search_info->_uid),_name(search_info->_name),_nick(search_info->_nick),
        _icon(search_info->_icon),_sex(search_info->_sex),_last_msg(""){

    }

    UserInfo(std::shared_ptr<FriendInfo> friend_info):
        _uid(friend_info->_uid),_name(friend_info->_name),_nick(friend_info->_nick),
        _icon(friend_info->_icon),_sex(friend_info->_sex),_last_msg(""){
        _chat_msgs = friend_info->_chat_msgs;
    }

    int _uid;
    QString _name;
    QString _nick;
    QString _icon;
    int _sex;
    QString _desc;
    QString _last_msg;
    std::vector<std::shared_ptr<TextChatData>> _chat_msgs;
};
Q_DECLARE_METATYPE(std::shared_ptr<UserInfo>)
class ChatDataBase {
public:
    ChatDataBase(int msg_id, int thread_id, ChatFormType form_type, ChatMsgType msg_type,
                 QString content,int _send_uid, int status, QString chat_time );
    ChatDataBase(QString unique_id, int thread_id, ChatFormType form_type, ChatMsgType msg_type,
                 QString content, int send_uid, int status, QString chat_time);
    ChatDataBase(int msg_id, QString unique_id, int thread_id, ChatFormType form_type, ChatMsgType msg_type,
                 QString content, int send_uid, int status, QString chat_time);
    int GetMsgId() { return _msg_id; }
    int GetThreadId() { return _thread_id; }
    ChatFormType GetFormType() { return _form_type; }
    ChatMsgType GetMsgType() { return _msg_type; }
    QString GetContent() { return _content; }
    int GetSendUid() { return _send_uid; }
    QString GetMsgContent(){return _content;}
    void SetUniqueId(int unique_id);
    QString GetUniqueId(){return _unique_id;}
    int GetStatus(){return _status;}
    void SetStatus(int status) { _status = status; }
    void SetMsgId(int msg_id) { _msg_id = msg_id; }
    virtual ~ChatDataBase() {}  // 添加虚析构函数
protected:
    //客户端本地唯一标识
    QString _unique_id;
    //消息id
    int _msg_id;
    //会话id
    int _thread_id;
    //群聊还是私聊
    ChatFormType _form_type;
    //文本信息为0，图片为1，文件为2
    ChatMsgType _msg_type;
    QString _content;
    //状态
    int _status;
    //发送者id
    int _send_uid;
    //聊天时间
    QString _chat_time;
};
Q_DECLARE_METATYPE(std::vector<std::shared_ptr<ChatDataBase>>)
struct TextChatData:public ChatDataBase{
    TextChatData(int msg_id, int thread_id, ChatFormType form_type, ChatMsgType msg_type,  QString content,
                 int send_uid, int status, QString chat_time="") :
        ChatDataBase(msg_id, thread_id, form_type, msg_type, content, send_uid, status, chat_time)
    {

    }

    TextChatData(QString unique_id, int thread_id, ChatFormType form_type, ChatMsgType msg_type, QString content,
                 int send_uid, int status, QString chat_time="") :
        ChatDataBase(unique_id, thread_id, form_type, msg_type, content, send_uid, status, chat_time)
    {

    }

    TextChatData(int msg_id, QString unique_id, int thread_id, ChatFormType form_type, ChatMsgType msg_type, QString content,
                 int send_uid, int status, QString chat_time = "") :
        ChatDataBase(unique_id, thread_id, form_type, msg_type, content, send_uid, status, chat_time)
    {

    }
    QString _msg_id;
    QString _msg_content;
    int _from_uid;
    int _to_uid;
};
Q_DECLARE_METATYPE(std::vector<std::shared_ptr<TextChatData>>)
//聊天线程信息
struct ChatThreadInfo{
    int _thread_id;
    QString _type; // "private" or "group"
    int _user1_id;    // 私聊时对应 private_chat.user1_id；群聊时设为 0
    int _user2_id;    // 私聊时对应 private_chat.user2_id；群聊时设为 0
};
Q_DECLARE_METATYPE(std::vector<std::shared_ptr<ChatThreadInfo>>)
///客户端本地存储的聊天线程数据结构
class ChatThreadData {
public:
    ChatThreadData(int other_id, int thread_id, int last_msg_id):
        _other_id(other_id), _thread_id(thread_id), _last_msg_id(last_msg_id){}
    void AddMsg(std::shared_ptr<ChatDataBase> msg);
    void SetLastMsgId(int msg_id);
    void SetOtherId(int other_id);
    int  GetOtherId();
    QString GetGroupName();
    void UpdateProgress(std::shared_ptr<MsgInfo> msg);
    QMap<int, std::shared_ptr<ChatDataBase>> GetMsgMap();
    int  GetThreadId();
    QMap<int, std::shared_ptr<ChatDataBase>>&  GetMsgMapRef();
    QMap<QString, std::shared_ptr<ChatDataBase>>& GetMsgUnRspRef();
    int GetLastMsgId();
    void AppendMsg(int msg_id, std::shared_ptr<ChatDataBase> base_msg);
    void AppendUnRspMsg(QString unique_id, std::shared_ptr<ChatDataBase> base_msg);
    QString GetLastMsg();
    void MoveMsg(std::shared_ptr<ChatDataBase> msg);
    std::shared_ptr<ChatDataBase> GetChatDataBase(int msg_id);
private:
    //如果是私聊，则为对方的id；如果是群聊，则为0
    int _other_id;
    int _last_msg_id;
    int _thread_id;
    QString _last_msg;
    //群聊信息,成员列表
    std::vector<int> _group_members;
    //群聊名称
    QString _group_name;
    //缓存消息map，抽象为基类，因为会有图片等其他类型消息
    QMap<int, std::shared_ptr<ChatDataBase>>  _msg_map;
    //缓存未回复的消息
    //已发送的消息，还未收到回应的。
    QMap<QString, std::shared_ptr<ChatDataBase>> _msg_unrsp_map;
};
class ImgChatData : public ChatDataBase {
public:
    ImgChatData(std::shared_ptr<MsgInfo> msg_info, QString unique_id,
                int thread_id, ChatFormType form_type, ChatMsgType msg_type,
                int send_uid, int status, QString chat_time = ""):
        ChatDataBase(unique_id,thread_id, form_type, msg_type, msg_info->_text_or_url,
                     send_uid, status, chat_time), _msg_info(msg_info){
        _msg_id = _msg_info->_msg_id;

    }

    std::shared_ptr<MsgInfo> _msg_info;
};

Q_DECLARE_METATYPE(std::shared_ptr<ImgChatData>)

class UserData
{
public:
    UserData();
};

#endif // USERDATA_H
