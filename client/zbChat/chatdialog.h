#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "global.h"
#include "chatuserwid.h"
#include "statewidget.h"
#include "applyfriend.h"
#include "QListWidgetItem"

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    void AddLBGroup(StateWidget *lb);
    void ClearLabelState(StateWidget *lb);
    void UpdateChatMsg(std::vector<std::shared_ptr<TextChatData> > msgdata);
public slots:
    void slot_loading_chat_user();
    void slot_side_chat();
    void slot_text_changed(const QString &str);
    void slot_side_contract();
    void slot_auth_rsp(std::shared_ptr<AuthRsp> authrsp);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
    void slot_append_send_chat_msg(std::shared_ptr<TextChatData> chatmsg);
    void slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void handleGlobalMousePress(QMouseEvent *event);
private slots:
    void on_btnAdd_clicked();
    void slot_appply_friend(std::shared_ptr<AddFriendApply> apply);
    void slot_item_clicked(QListWidgetItem* item);
private:
    void ShowSearch(bool bsearch =false);
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    Ui::ChatDialog *ui;
    int _cur_chat_uid;
    //测试数据
    void addChatUserList();
    QList<StateWidget*> _lb_list;
    QWidget* _last_widget;
    //使用map或者hash，当有人发消息是就可以轻松找到对应的，并展示出来
    QMap<int, QListWidgetItem*> _chat_items_added;
    std::vector<QString>  strs ={"hello world !",
                                 "nice to meet u",
                                 "New year，new life",
                                 "You have to love yourself",
                                 "My love is written in the wind ever since the whole world is you"};
    std::vector<QString> heads = {
        ":/res/head_1.jpg",
        ":/res/head_2.jpg",
        ":/res/head_3.jpg",
        ":/res/head_4.jpg",
        ":/res/head_5.jpg"
    };
    std::vector<QString> names = {
        "llfc",
        "zack",
        "golang",
        "cpp",
        "java",
        "nodejs",
        "python",
        "rust"
    };
};

#endif // CHATDIALOG_H
