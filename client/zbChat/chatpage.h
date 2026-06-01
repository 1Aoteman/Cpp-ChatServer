#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include "global.h"
#include "chatitembase.h"
#include "textbubble.h"
#include "picturebubble.h"
#include "userdata.h"
#include "usermgr.h"
#include <QJsonObject>
#include <QJsonDocument>
namespace Ui {
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();
    void SetUserInfo(std::shared_ptr<UserInfo> userinfo);
    void AppendChatMsg(std::shared_ptr<TextChatData> msg);
private slots:
    void on_send_btn_clicked();
signals:
    void sig_append_send_chat_msg(std::shared_ptr<TextChatData>);
private:
    Ui::ChatPage *ui;
    void paintEvent(QPaintEvent *event);
    std::shared_ptr<UserInfo> _user_info;
};

#endif // CHATPAGE_H
