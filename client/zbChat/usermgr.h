#ifndef USERMGR_H
#define USERMGR_H
#include <QObject>
#include <memory>
#include <SIngleton.h>
#include "userdata.h"
#include <QMap>
#include <QJsonArray>
#include <QFileInfo>
#include <qlabel.h>
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
    QString GetIcon();
    void SetUid(int uid);
    void SetToken(QString token);
    QString GetToken();
    void SetUserInfo(std::shared_ptr<UserInfo> userinfo);
    QString GetNick();
    QString GetDesc();
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
    void AddChatThreadData(std::shared_ptr<ChatThreadData> chat_thread_data,int other_uid);
    void SetLastChatThreadId(int last_thread_id);
    int GetLastChatThreadId();
    int GetThreadIdByUid(int uid);
    std::shared_ptr<ChatThreadData> GetChatThreadByUid(int uid);
    std::shared_ptr<ChatThreadData> GetChatThreadByThreadId(int thread_id);
    std::shared_ptr<ChatThreadData> GetCurLoadData();
    std::shared_ptr<ChatThreadData> GetNextLoadData();
    std::shared_ptr<QFileInfo>GetFileInfoByfilename(QString name);
    void AddNameFile(QString name,std::shared_ptr<QFileInfo> file_info);
    bool IsDownLoading(QString name);
    void AddLabelToReset(QString path, QLabel* label);
    void AddDownloadFile(QString name,
                                  std::shared_ptr<DownloadInfo> file_info);
    void ResetLabelIcon(QString path);
    void RmvDownloadFile(QString name);
    std::shared_ptr<DownloadInfo> GetDownloadInfo(QString name);
    void AddTransFile(QString name, std::shared_ptr<MsgInfo> msg_info);
    std::shared_ptr<MsgInfo> GetTransFileByName(QString name);
    void RmvTransFileByName(QString name);
    void PauseTransFileByName(QString name);
    void ResumeTransFileByName(QString name);
    bool TransFileIsUploading(QString name);
    std::shared_ptr<MsgInfo> GetFreeUploadFile();
private:
    UserMgr();
    QString _name;
    QString _token;
    std::shared_ptr<UserInfo> _user_info;
    int _uid;
    std::mutex _mtx;
    int _chat_loaded;
    int _contact_loaded;
    std::vector<std::shared_ptr<ApplyInfo>> _apply_list;
    std::vector<std::shared_ptr<FriendInfo>> _friend_list;
    //这个是当有信息传来时，方便查询
    QMap<int, std::shared_ptr<FriendInfo>> _friend_map;
    //上一次会话的id
    int _last_chat_thread_id;
    //建立会话id到数据的映射关系
    QMap<int, std::shared_ptr<ChatThreadData>> _chat_map;
    //聊天会话id列表
    std::vector<int> _chat_thread_ids;
    //记录已经加载聊天列表的会话索引
    int _cur_load_chat_index;
    //缓存其他用户uid和聊天的thread_id的映射关系。
    QMap<int, int> _uid_to_thread_id;
    //md5和文件信息的映射
    QMap<QString,std::shared_ptr<QFileInfo>> _name_to_upload_info;
    std::mutex _down_load_mtx;
    //名字关联下载信息
    QMap<QString, std::shared_ptr<DownloadInfo> > _name_to_download_info;
    QHash<QString, QList<QLabel*>> _path_to_reset_labels;
    //聊天传输文件映射
    QHash<QString, std::shared_ptr<MsgInfo> > _name_to_msg_info;
    //传输文件用的锁
    std::mutex _trans_mtx;
};
#endif // USERMGR_H
