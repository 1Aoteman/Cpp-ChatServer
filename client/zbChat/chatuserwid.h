#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include <QWidget>
#include "listitembase.h"
#include "userdata.h"
#include "usermgr.h"

namespace Ui {
class ChatUserWid;
}
class ChatUserWid : public ListItemBase
{
    Q_OBJECT
public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();
    QSize sizeHint() const override {
        return QSize(250, 70); // 返回自定义的尺寸
    }
    void SetInfo(QString name, QString head, QString msg);
    void SetInfo(std::shared_ptr<UserInfo> userinfo);
    void SetInfo(std::shared_ptr<FriendInfo> friendinfo);
    std::shared_ptr<UserInfo> GetUserInfo();
    void updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs);
    void SetChatData(std::shared_ptr<ChatThreadData> chat_data);
    std::shared_ptr<ChatThreadData> GetChatData();
private:
    Ui::ChatUserWid *ui;
    QString _name;
    QString _head;
    QString _msg;
    std::shared_ptr<UserInfo> _user_info;
    std::shared_ptr<ChatThreadData> _chat_data;
};

#endif // CHATUSERWID_H
