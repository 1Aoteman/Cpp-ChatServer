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
    void AppendChatMsg(std::shared_ptr<ChatDataBase> msg,bool rsp = true);
    void AppendOtherMsg(std::shared_ptr<ChatDataBase> msg);
    void SetChatData(std::shared_ptr<ChatThreadData> chat_data);
    void UpdateChatStatus(std::shared_ptr<ChatDataBase> msg);
    void UpdateImgChatStatus(std::shared_ptr<ImgChatData> msg);
    void SetSelfIcon(ChatItemBase* pChatItem, QString icon);
    void LoadHeadIcon(QString avatarPath, QLabel* icon_label, QString file_name, QString req_type);
    void UpdateFileProgress(std::shared_ptr<MsgInfo> msg_info);
    void DownloadFileFinished(std::shared_ptr<MsgInfo> msg_info, QString file_path);
private slots:
    void on_send_btn_clicked();
    void on_receive_btn_clicked();
    void on_clicked_paused(QString unique_name, TransferType transfer_type);
    void on_clicked_resume(QString unique_name, TransferType transfer_type);
signals:
    void sig_append_send_chat_msg(std::shared_ptr<TextChatData>);
private:
    Ui::ChatPage *ui;
    void paintEvent(QPaintEvent *event);
    std::shared_ptr<ChatThreadData> _chat_data;
    QHash<QString, ChatItemBase*> _unrsp_item_map;
    //管理已经回复的消息
    QHash<qint64, ChatItemBase*> _base_item_map;
};

#endif // CHATPAGE_H
