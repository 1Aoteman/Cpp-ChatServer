#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include "global.h"
#include "chatitembase.h"
#include "textbubble.h"
#include "picturebubble.h"
#include "userdata.h"
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
private slots:
    void on_send_btn_clicked();

private:
    Ui::ChatPage *ui;
    void paintEvent(QPaintEvent *event);
    std::shared_ptr<UserInfo> _user_info;
};

#endif // CHATPAGE_H
