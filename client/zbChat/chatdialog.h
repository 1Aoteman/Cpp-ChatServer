#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include <QTimer>
#include "global.h"
#include "chatuserwid.h"
#include "statewidget.h"
#include "applyfriend.h"
#include "QListWidgetItem"
#include "loadingdlg.h"
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
    void UpdateImgChatStatus(std::shared_ptr<ImgChatData> msg);
    void loadChatList();
    void loadChatMsg();
    void SetSelectChatItem(int thread_id = 0);
    void SetSelectChatPage(int thread_id = 0);
    void showLoadingDlg(bool show);
public slots:
    void slot_loading_chat_user();
    void slot_side_chat();
    void slot_text_changed(const QString &str);
    void slot_side_contract();
    void slot_side_setting();
    void slot_auth_rsp(std::shared_ptr<AuthRsp> authrsp);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
    void slot_append_send_chat_msg(std::shared_ptr<TextChatData> chatmsg);
    void slot_text_chat_msg(std::vector<std::shared_ptr<TextChatData>> msg_list);
    void slot_loading_contact_user();
    void slot_switch_apply_friend_page();
    void slot_show_search(bool show);
    void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);
    void slot_jump_chat_item_from_infopage(std::shared_ptr<UserInfo> user_info);
    void slot_load_chat_thread(bool load_more,int last_thread_id,std::vector<std::shared_ptr<ChatThreadInfo>> chat_threads);
    void slot_create_private_chat(int uid, int other_id, int thread_id);
    void slot_load_chat_msg(int thread_id, int msg_id, bool load_more, std::vector<std::shared_ptr<ChatDataBase>> msglists);
    void slot_add_chat_msg(int thread_id, std::vector<std::shared_ptr<TextChatData>> msglists);
    void slot_reset_icon(QString path);
    void slot_add_img_msg(int thread_id, std::shared_ptr<ImgChatData> img_msg);
    void slot_update_upload_progress(std::shared_ptr<MsgInfo> msg_info);
    void slot_download_finish(std::shared_ptr<MsgInfo> msg_info, QString file_path);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void handleGlobalMousePress(QMouseEvent *event);
private slots:
    void on_btnAdd_clicked();
    void slot_appply_friend(std::shared_ptr<AddFriendApply> apply);
    void slot_item_clicked(QListWidgetItem* item);
    void slot_friend_info_clicked(std::shared_ptr<UserInfo> userinfo);
    void slot_img_chat_msg(std::shared_ptr<ImgChatData> imgchat);
    void slot_update_download_progress(std::shared_ptr<MsgInfo> msg_info);
private:
    void ShowSearch(bool bsearch =false);
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    Ui::ChatDialog *ui;
    int _cur_chat_uid;
    void loadMoreChatUser();
    //测试数据
    void addChatUserList();
    QList<StateWidget*> _lb_list;
    QWidget* _last_widget;
    //使用map或者hash，当有人发消息是就可以轻松找到对应的，并展示出来
    //QMap<int, QListWidgetItem*> _chat_items_added;
    QTimer *_timer;
    LoadingDlg* _loading_dlg;
    int _cur_chat_thread_id;
    //chat_thred_id和对应的item的映射关系。

    QMap<int, QListWidgetItem*>  _chat_thread_items;
    std::shared_ptr<ChatThreadData> _cur_load_chat;
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
