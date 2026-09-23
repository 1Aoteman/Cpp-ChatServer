#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QRandomGenerator>
#include "loadingdlg.h"
#include "tcpmgr.h"
#include "usermgr.h"
#include "friendinfopage.h"
#include "applyfriendpage.h"
#include "ui_applyfriendpage.h"  // 注意全小写，这是 Qt 生成文件的默认命名规则
#include "ui_friendinfopage.h"
#include <QStandardPaths>
#include <QDir>
#include "filetcpmgr.h"

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent),ui(new Ui::ChatDialog),_mode(ChatUIMode::ChatMode),
    _state(ChatUIMode::ChatMode),_b_loading(false),_last_widget(nullptr),_cur_chat_uid(0),
    _loading_dlg(nullptr)
{

    ui->setupUi(this);
    ui->btnAdd->SetState("normal","hover","press");
    //模拟加载自己头像
    QString head_icon = UserMgr::GetInstance()->GetIcon();
    //使用正则表达式检查是否使用默认头像
    QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
    QRegularExpressionMatch match = regex.match(head_icon);
    if (match.hasMatch()) {
        // 如果是默认头像（:/res/head_X.jpg 格式）
        QPixmap pixmap(head_icon); // 加载默认头像图片
        QPixmap scaledPixmap = pixmap.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->side_head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
        ui->side_head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
    }
    else {
        // 如果是用户上传的头像，获取存储目录
        QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir avatarsDir(storageDir + "/avatars");

        // 确保目录存在
        if (avatarsDir.exists()) {
            QString avatarPath = avatarsDir.filePath(QFileInfo(head_icon).fileName()); // 获取上传头像的完整路径
            QPixmap pixmap(avatarPath); // 加载上传的头像图片
            if (!pixmap.isNull()) {
                QPixmap scaledPixmap = pixmap.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                ui->side_head_lb->setPixmap(scaledPixmap);
                ui->side_head_lb->setScaledContents(true);
            }
            else {
                qWarning() << "无法加载上传的头像：" << avatarPath;
            }
        }
        else {
            qWarning() << "头像存储目录不存在：" << avatarsDir.path();
        }
    }
    QAction *searchAction = new QAction(ui->search_edit);
    searchAction->setIcon(QIcon(":/res/search.png"));
    ui->search_edit->addAction(searchAction,QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));
    // 创建一个清除动作并设置图标
    QAction *clearAction = new QAction(ui->search_edit);
    clearAction->setIcon(QIcon(":/res/close_transparent.png"));
    // 初始时不显示清除图标
    // 将清除动作添加到LineEdit的末尾位置
    ui->search_edit->addAction(clearAction, QLineEdit::TrailingPosition);
    // 当需要显示清除图标时，更改为实际的清除图标
    connect(ui->search_edit, &QLineEdit::textChanged, [clearAction](const QString &text) {
        if (!text.isEmpty()) {
            clearAction->setIcon(QIcon(":/res/close_search.png"));
        } else {
            clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 文本为空时，切换回透明图标
        }
    });
    // 连接清除动作的触发信号到槽函数，用于清除文本
    connect(clearAction, &QAction::triggered, [this, clearAction]() {
        ui->search_edit->clear();
        clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 清除文本后，切换回透明图标
        ui->search_edit->clearFocus();
        //清除按钮被按下则不显示搜索框
        ShowSearch(false);
    });
    ShowSearch(false);
    ui->search_edit->SetMaxLength(15);

    // QPixmap pixmap(":/res/head_1.jpg");
    // ui->side_head_lb->setPixmap(pixmap); // 将图片设置到QLabel上
    // QPixmap scaledPixmap = pixmap.scaled( ui->side_head_lb->size(), Qt::KeepAspectRatio); // 将图片缩放到label的大小
    // ui->side_head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
    // ui->side_head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
    ui->side_chat_lb->setProperty("state","normal");
    ui->side_chat_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");
    ui->side_contract_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");
    ui->side_setting_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");
    AddLBGroup(ui->side_chat_lb);
    AddLBGroup(ui->side_contract_lb);
    AddLBGroup(ui->side_setting_lb);
    connect(ui->side_chat_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contract_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_contract);
    connect(ui->side_setting_lb,&StateWidget::clicked, this, &ChatDialog::slot_side_setting);
    //链接搜索框输入变化
    connect(ui->search_edit, &QLineEdit::textChanged, this, &ChatDialog::slot_text_changed);
    ShowSearch(false);

    ui->search_list->SetSearchEdit(ui->search_edit);

    //检测鼠标点击位置判断是否要清空搜索框
    this->installEventFilter(this); // 安装事件过滤器
    //设置聊天label选中状态
    ui->side_chat_lb->SetSelected(true);

    //设置选中条目
    SetSelectChatItem();
    //更新聊天界面信息
    SetSelectChatPage();

    //连接加载联系人的信号和槽函数
    connect(ui->con_user_list, &ContactUserList::sig_loading_contact_user,
            this, &ChatDialog::slot_loading_contact_user);

    //连接联系人页面点击好友申请条目的信号
    connect(ui->con_user_list, &ContactUserList::sig_switch_apply_friend_page,
            this, &ChatDialog::slot_switch_apply_friend_page);

    //连接清除搜索框操作
    connect(ui->friend_apply_page, &ApplyFriendPage::sig_show_search, this, &ChatDialog::slot_show_search);

    //为searchlist 设置search edit
    ui->search_list->SetSearchEdit(ui->search_edit);

    //连接添加朋友按钮与界面
    connect(ui->btnAdd,&QPushButton::clicked,this,&ChatDialog::on_btnAdd_clicked);
    //连接聊天列表点击信号
    connect(ui->chat_user_list, &QListWidget::itemClicked, this, &ChatDialog::slot_item_clicked);
    //连接朋友列表点击信号
    connect(ui->con_user_list,&ContactUserList::sig_switch_friend_info_page,this,&ChatDialog::slot_friend_info_clicked);
    //添加朋友信号和槽
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_friend_apply,this,&ChatDialog::slot_appply_friend);

    //设置中心部件为chatpage
    ui->stackedWidget->setCurrentWidget(ui->chat_page);

    //连接searchlist跳转聊天信号
    connect(ui->search_list, &SearchList::sig_jump_chat_item, this, &ChatDialog::slot_jump_chat_item);

    //连接好友信息界面发送的点击事件
    connect(ui->friend_info_page, &FriendInfoPage::sig_jump_chat_item, this,
            &ChatDialog::slot_jump_chat_item_from_infopage);

    //验证朋友和展示
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_auth_rsp,this,&ChatDialog::slot_auth_rsp);
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_add_auth_friend,this,&ChatDialog::slot_add_auth_friend);
    //发送信息，聊天框里展示信息
    connect(ui->chat_page,&ChatPage::sig_append_send_chat_msg,this,&ChatDialog::slot_append_send_chat_msg);
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_text_chat_msg,this,&ChatDialog::slot_text_chat_msg);
    //定时发送消息，保证连接
    _timer = new QTimer(this);
    connect(_timer,&QTimer::timeout,this,[this]{
        auto userInfo = UserMgr::GetInstance()->GetUserInfo();
        QJsonObject jsonobj;
        jsonobj["from_uid"] = userInfo->_uid;
        QJsonDocument jsondoc(jsonobj);
        QByteArray data = jsondoc.toJson(QJsonDocument::Compact);
        //发送信号
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_HEART_BEAT_REQ,data);
    });
    _timer->start(10000);
    //连接tcp返回的加载聊天回复
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_load_chat_thread,
            this, &ChatDialog::slot_load_chat_thread);

    //连接tcp返回的创建私聊的回复
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_create_private_chat,
            this, &ChatDialog::slot_create_private_chat);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_load_chat_msg,
            this, &ChatDialog::slot_load_chat_msg);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_chat_msg_rsp, this, &ChatDialog::slot_add_chat_msg);
    //重置label icon
    connect(FileTcpMgr::GetInstance().get(), &FileTcpMgr::sig_reset_label_icon, this, &ChatDialog::slot_reset_icon);
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_chat_img_rsp,this,&ChatDialog::slot_add_img_msg);
    //接收tcp返回的上传进度信息
    connect(FileTcpMgr::GetInstance().get(), &FileTcpMgr::sig_update_upload_progress,
            this, &ChatDialog::slot_update_upload_progress);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_img_chat_msg,
            this, &ChatDialog::slot_img_chat_msg);
    //接收tcp返回的下载进度信息
    connect(FileTcpMgr::GetInstance().get(), &FileTcpMgr::sig_update_download_progress,
            this, &ChatDialog::slot_update_download_progress);
    //接收tcp返回的下载完成信息
    connect(FileTcpMgr::GetInstance().get(), &FileTcpMgr::sig_download_finish,
            this, &ChatDialog::slot_download_finish);

}

ChatDialog::~ChatDialog()
{
    _timer->stop();
    delete ui;
}

void ChatDialog::ShowSearch(bool bsearch)
{
    if(bsearch){
        ui->chat_user_list->hide();
        ui->con_user_list->hide();
        ui->search_list->show();
        _mode = ChatUIMode::SearchMode;
    }else if(_state == ChatUIMode::ChatMode){
        ui->chat_user_list->show();
        ui->con_user_list->hide();
        ui->search_list->hide();
        _mode = ChatUIMode::ChatMode;
    }else if(_state == ChatUIMode::ContactMode){
        ui->chat_user_list->hide();
        ui->search_list->hide();
        ui->con_user_list->show();
        _mode = ChatUIMode::ContactMode;
    }else if(_state == ChatUIMode::SettingsMode){
        ui->chat_user_list->hide();
        ui->search_list->hide();
        ui->con_user_list->show();
        _mode = ChatUIMode::ContactMode;
    }
}

//向服务器请求加载聊天列表
void ChatDialog::loadChatList()
{
    showLoadingDlg(true);

    //发送请求逻辑
    QJsonObject jsonObj;
    auto uid = UserMgr::GetInstance()->GetUId();
    jsonObj["uid"] = uid;
    int last_chat_thread_id = UserMgr::GetInstance()->GetLastChatThreadId();
    jsonObj["thread_id"] = last_chat_thread_id;


    QJsonDocument doc(jsonObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_THREAD_REQ, jsonData);
}

void ChatDialog::loadChatMsg() {

    //发送聊天记录请求
    _cur_load_chat = UserMgr::GetInstance()->GetCurLoadData();
    if (_cur_load_chat == nullptr) {
        return;
    }

    showLoadingDlg(true);

    //发送请求给服务器
    //发送请求逻辑
    QJsonObject jsonObj;
    jsonObj["thread_id"] = _cur_load_chat->GetThreadId();
    jsonObj["message_id"] = _cur_load_chat->GetLastMsgId();

    QJsonDocument doc(jsonObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_MSG_REQ, jsonData);
}
void ChatDialog::slot_loading_chat_user()
{
    if(_b_loading){
        return;
    }
    _b_loading = true;
    LoadingDlg *loadingDialog = new LoadingDlg(this);
    loadingDialog->setModal(true);
    loadingDialog->show();
    qDebug() << "add new data to list.....";
    loadMoreChatUser();
    // 加载完成后关闭对话框
    loadingDialog->deleteLater();
    _b_loading = false;
}
void ChatDialog::slot_side_chat()
{
    qDebug()<< "receive side chat clicked";
    ClearLabelState(ui->side_chat_lb);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    _state = ChatUIMode::ChatMode;
    ShowSearch(false);
}
void ChatDialog::slot_text_changed(const QString &str)
{
    //qDebug()<< "receive slot text changed str is " << str;
    if (!str.isEmpty()) {
        ShowSearch(true);
    }
}
void ChatDialog::slot_side_contract(){
    qDebug()<< "receive side contact clicked";
    ClearLabelState(ui->side_contract_lb);
    //设置
    if(_last_widget == nullptr){
        ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
        _last_widget = ui->friend_apply_page;
    }else{
        ui->stackedWidget->setCurrentWidget(_last_widget);
    }

    _state = ChatUIMode::ContactMode;
    ShowSearch(false);
}

void ChatDialog::slot_side_setting()
{
    qDebug()<< "receive side chat clicked";
    ClearLabelState(ui->side_setting_lb);
    ui->stackedWidget->setCurrentWidget(ui->user_info_page);
    _state = ChatUIMode::ChatMode;
    ShowSearch(false);
}

void ChatDialog::slot_auth_rsp(std::shared_ptr<AuthRsp> authrsp)
{
    qDebug() << "receive slot_auth_rsp uid is " << authrsp->_uid
             << " name is " << authrsp->_name << " nick is " << authrsp->_nick;
    //判断如果已经是好友则跳过
    auto bfriend = UserMgr::GetInstance()->CheckFriendById(authrsp->_uid);
    if(bfriend){
        return;
    }
    UserMgr::GetInstance()->AddFriend(authrsp);
    int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int str_i = randomValue % strs.size();
    int head_i = randomValue % heads.size();
    int name_i = randomValue % names.size();
    auto* chat_user_wid = new ChatUserWid();
    auto chat_thread_data = std::make_shared<ChatThreadData>(authrsp->_uid, authrsp->_thread_id, 0);
    UserMgr::GetInstance()->AddChatThreadData(chat_thread_data, authrsp->_uid);
    for (auto& chat_msg : authrsp->_chat_datas) {
        chat_thread_data->AppendMsg(chat_msg->GetMsgId(), chat_msg);
    }
    chat_user_wid->SetChatData(chat_thread_data);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_thread_items.insert(authrsp->_thread_id, item);
}

void ChatDialog::slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info)
{
    qDebug() << "receive slot_add_auth__friend uid is " << auth_info->_uid
             << " name is " << auth_info->_name << " nick is " << auth_info->_nick;
    //判断一下是否已经是好友
    bool b_friend = UserMgr::GetInstance()->CheckFriendById(auth_info->_uid);
    if(b_friend){
        return;
    }
    //将朋友添加这内存，以便显示
    UserMgr::GetInstance()->AddFriend(auth_info);

    auto* chat_user_wid = new ChatUserWid();
    auto chat_thread_data = std::make_shared<ChatThreadData>(auth_info->_uid, auth_info->_thread_id, 0);
    UserMgr::GetInstance()->AddChatThreadData(chat_thread_data, auth_info->_uid);
    for (auto& chat_msg : auth_info->_chat_datas) {
        chat_thread_data->AppendMsg(chat_msg->GetMsgId(), chat_msg);
    }

    chat_user_wid->SetChatData(chat_thread_data);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_thread_items.insert(auth_info->_thread_id, item);
}

void ChatDialog::slot_append_send_chat_msg(std::shared_ptr<TextChatData> chatmsg)
{
    if (_cur_chat_uid == 0) {
        return;
    }

    auto find_iter = _chat_thread_items.find(_cur_chat_uid);
    if (find_iter == _chat_thread_items.end()) {
        return;
    }

    //转为widget
    QWidget* widget = ui->chat_user_list->itemWidget(find_iter.value());
    if (!widget) {
        return;
    }

    //判断转化为自定义的widget
    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase* customItem = qobject_cast<ListItemBase*>(widget);
    if (!customItem) {
        qDebug() << "qobject_cast<ListItemBase*>(widget) is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if (itemType == CHAT_USER_ITEM) {
        auto con_item = qobject_cast<ChatUserWid*>(customItem);
        if (!con_item) {
            return;
        }

        //设置信息
        auto user_info = con_item->GetUserInfo();
        user_info->_chat_msgs.push_back(chatmsg);
        std::vector<std::shared_ptr<TextChatData>> msg_vec;
        msg_vec.push_back(chatmsg);
        UserMgr::GetInstance()->AppendFriendChatMsg(_cur_chat_uid,msg_vec);
        return;
    }
}

void ChatDialog::slot_text_chat_msg(std::vector<std::shared_ptr<TextChatData>> msg_list)
{
    for (auto& msg : msg_list) {

        //更新数据
        auto thread_id = msg->GetThreadId();
        auto thread_data = UserMgr::GetInstance()->GetChatThreadByThreadId(thread_id);

        thread_data->AddMsg(msg);

        if (_cur_chat_thread_id != thread_id) {
            continue;
        }

        ui->chat_page->AppendChatMsg(msg);
    }
}

void ChatDialog::slot_loading_contact_user()
{

}
void ChatDialog::SetSelectChatItem(int thread_id)
{
    if (ui->chat_user_list->count() <= 0) {
        return;
    }

    if (thread_id == 0) {
        ui->chat_user_list->setCurrentRow(0);
        QListWidgetItem* firstItem = ui->chat_user_list->item(0);
        if (!firstItem) {
            return;
        }

        //转为widget
        QWidget* widget = ui->chat_user_list->itemWidget(firstItem);
        if (!widget) {
            return;
        }

        auto con_item = qobject_cast<ChatUserWid*>(widget);
        if (!con_item) {
            return;
        }

        _cur_chat_thread_id = con_item->GetChatData()->GetThreadId();

        return;
    }

    auto find_iter = _chat_thread_items.find(thread_id);
    if (find_iter == _chat_thread_items.end()) {
        qDebug() << "thread_id [" << thread_id << "] not found, set curent row 0";
        ui->chat_user_list->setCurrentRow(0);
        return;
    }

    ui->chat_user_list->setCurrentItem(find_iter.value());

    _cur_chat_thread_id = thread_id;
}

void ChatDialog::SetSelectChatPage(int thread_id)
{
    if (ui->chat_user_list->count() <= 0) {
        return;
    }

    if (thread_id == 0) {
        auto item = ui->chat_user_list->item(0);
        //转为widget
        QWidget* widget = ui->chat_user_list->itemWidget(item);
        if (!widget) {
            return;
        }

        auto con_item = qobject_cast<ChatUserWid*>(widget);
        if (!con_item) {
            return;
        }

        //设置信息
        auto chat_data = con_item->GetChatData();
        ui->chat_page->SetChatData(chat_data);
        return;
    }

    auto find_iter = _chat_thread_items.find(thread_id);
    if (find_iter == _chat_thread_items.end()) {
        return;
    }

    //转为widget
    QWidget* widget = ui->chat_user_list->itemWidget(find_iter.value());
    if (!widget) {
        return;
    }

    //判断转化为自定义的widget
    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase* customItem = qobject_cast<ListItemBase*>(widget);
    if (!customItem) {
        qDebug() << "qobject_cast<ListItemBase*>(widget) is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if (itemType == CHAT_USER_ITEM) {
        auto con_item = qobject_cast<ChatUserWid*>(customItem);
        if (!con_item) {
            return;
        }

        //设置信息
        auto chat_data = con_item->GetChatData();
        ui->chat_page->SetChatData(chat_data);

        return;
    }

}
void ChatDialog::slot_switch_apply_friend_page()
{
    _last_widget = ui->friend_apply_page;
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
}

void ChatDialog::slot_show_search(bool show)
{
    ShowSearch(show);
}

void ChatDialog::slot_jump_chat_item(std::shared_ptr<SearchInfo> si)
{

}

void ChatDialog::slot_jump_chat_item_from_infopage(std::shared_ptr<UserInfo> user_info)
{
    qDebug() << "slot jump chat item " << Qt::endl;
    auto chat_thread_data = UserMgr::GetInstance()->GetChatThreadByUid(user_info->_uid);
    if (chat_thread_data) {
        auto find_iter = _chat_thread_items.find(chat_thread_data->GetThreadId());
        if (find_iter != _chat_thread_items.end()) {
            qDebug() << "jump to chat item , uid is " << user_info->_uid;
            ui->chat_user_list->scrollToItem(find_iter.value());
            ui->side_chat_lb->SetSelected(true);
            SetSelectChatItem(chat_thread_data->GetThreadId());
            //更新聊天界面信息
            SetSelectChatPage(chat_thread_data->GetThreadId());
            slot_side_chat();
            return;
        } //说明之前有缓存过聊天列表，只是被删除了，那么重新加进来即可
        else {
            auto* chat_user_wid = new ChatUserWid();
            chat_user_wid->SetInfo(user_info);
            QListWidgetItem* item = new QListWidgetItem;
            qDebug() << "chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
            ui->chat_user_list->insertItem(0, item);
            ui->chat_user_list->setItemWidget(item, chat_user_wid);
            _chat_thread_items.insert(chat_thread_data->GetThreadId(), item);
            ui->side_chat_lb->SetSelected(true);
            SetSelectChatItem(user_info->_uid);
            //更新聊天界面信息
            SetSelectChatPage(user_info->_uid);
            slot_side_chat();
            return;
        }
    }

    //如果没找到，则发送创建请求
    auto uid = UserMgr::GetInstance()->GetUId();
    QJsonObject jsonObj;
    jsonObj["uid"] = uid;
    jsonObj["other_id"] = user_info->_uid;

    QJsonDocument doc(jsonObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_CREATE_PRIVATE_CHAT_REQ, jsonData);
}

void ChatDialog::slot_load_chat_thread(bool load_more, int last_thread_id, std::vector<std::shared_ptr<ChatThreadInfo> > chat_threads)
{
    for (auto& cti : chat_threads) {
        //先处理单聊，群聊跳过，以后添加
        if (cti->_type == "group") {
            continue;
        }

        auto uid = UserMgr::GetInstance()->GetUId();
        auto other_uid = 0;
        if (uid == cti->_user1_id) {
            other_uid = cti->_user2_id;
        }else {
            other_uid = cti->_user1_id;
        }

        auto chat_thread_data = std::make_shared<ChatThreadData>(other_uid,cti->_thread_id,0);
        UserMgr::GetInstance()->AddChatThreadData(chat_thread_data,other_uid);

        auto* chat_user_wid = new ChatUserWid();

        chat_user_wid->SetChatData(chat_thread_data);
        QListWidgetItem* item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
        _chat_thread_items.insert(cti->_thread_id, item);
    }

    UserMgr::GetInstance()->SetLastChatThreadId(last_thread_id);

    if (load_more) {
        //发送请求逻辑
        QJsonObject jsonObj;
        auto uid = UserMgr::GetInstance()->GetUId();
        jsonObj["uid"] = uid;
        jsonObj["thread_id"] = last_thread_id;


        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        //发送tcp请求给chat server
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_THREAD_REQ, jsonData);
        return;
    }

    //更新聊天界面信息

    showLoadingDlg(false);
    //继续加载聊天数据
    loadChatMsg();
}

void ChatDialog::slot_create_private_chat(int uid, int other_id, int thread_id)
{
    auto* chat_user_wid = new ChatUserWid();
    auto chat_thread_data = std::make_shared<ChatThreadData>(other_id,thread_id,0);
    if(chat_thread_data==nullptr){
        return;
    }
    UserMgr::GetInstance()->AddChatThreadData(chat_thread_data,other_id);

    chat_user_wid->SetChatData(chat_thread_data);
    QListWidgetItem* item = new QListWidgetItem;
    item->setSizeHint(chat_user_wid->sizeHint());
    qDebug() << "chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_thread_items.insert(thread_id, item);

    ui->side_chat_lb->SetSelected(true);
    SetSelectChatItem(thread_id);
    //更新聊天界面信息
    SetSelectChatPage(thread_id);
    slot_side_chat();
    return;
}

void ChatDialog::slot_load_chat_msg(int thread_id, int msg_id, bool load_more, std::vector<std::shared_ptr<ChatDataBase>> msglists)
{
    _cur_load_chat->SetLastMsgId(msg_id);
    //加载聊天信息
    for (auto& chat_msg : msglists) {
        _cur_load_chat->AppendMsg(chat_msg->GetMsgId(), chat_msg);
    }

    //还有未加载完的消息，就继续加载
    if (load_more) {
        //发送请求给服务器
        //发送请求逻辑
        QJsonObject jsonObj;
        jsonObj["thread_id"] = _cur_load_chat->GetThreadId();
        jsonObj["message_id"] = _cur_load_chat->GetLastMsgId();

        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        //发送tcp请求给chat server
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_MSG_REQ, jsonData);
        return;
    }

    //获取下一个chat_thread
    _cur_load_chat = UserMgr::GetInstance()->GetNextLoadData();
    //都加载完了
    if(!_cur_load_chat){
        //更新聊天界面信息
        SetSelectChatItem();
        SetSelectChatPage();
        showLoadingDlg(false);
        return;
    }

    //继续加载下一个聊天
    //发送请求给服务器
    //发送请求逻辑
    QJsonObject jsonObj;
    jsonObj["thread_id"] = _cur_load_chat->GetThreadId();
    jsonObj["message_id"] = _cur_load_chat->GetLastMsgId();

    QJsonDocument doc(jsonObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_MSG_REQ, jsonData);
}

void ChatDialog::slot_add_chat_msg(int thread_id, std::vector<std::shared_ptr<TextChatData>> msglists) {
    auto chat_data = UserMgr::GetInstance()->GetChatThreadByThreadId(thread_id);
    if (chat_data == nullptr) {
        return;
    }

    //将消息放入数据中管理
    for (auto& msg : msglists) {
        chat_data->MoveMsg(msg);

        if (_cur_chat_thread_id != thread_id) {
            continue;
        }
        //更新聊天界面信息
        ui->chat_page->UpdateChatStatus(msg);
    }

}
void ChatDialog::ClearLabelState(StateWidget *lb)
{
    for(auto & ele: _lb_list){
        if(ele == lb){
            continue;
        }

        ele->ClearState();
    }
}
void ChatDialog::showLoadingDlg(bool show)
{
    if (show) {
        if (_loading_dlg) {
            _loading_dlg->deleteLater();
        }
        _loading_dlg = new LoadingDlg(this, "正在加载聊天列表...");
        _loading_dlg->setModal(true);
        _loading_dlg->show();
        return;
    }

    if (_loading_dlg) {
        _loading_dlg->deleteLater();
        _loading_dlg = nullptr;
    }

}
void ChatDialog::AddLBGroup(StateWidget *lb)
{
    _lb_list.push_back(lb);
}
bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        handleGlobalMousePress(mouseEvent);
    }
    return QDialog::eventFilter(watched, event);
}
void ChatDialog::handleGlobalMousePress(QMouseEvent *event)
{
    // 实现点击位置的判断和处理逻辑
    // 先判断是否处于搜索模式，如果不处于搜索模式则直接返回
    if( _mode != ChatUIMode::SearchMode){
        return;
    }
    // 将鼠标点击位置转换为搜索列表坐标系中的位置
    QPoint posInSearchList = ui->search_list->mapFromGlobal(event->globalPos());
    // 判断点击位置是否在聊天列表的范围内
    if (!ui->search_list->rect().contains(posInSearchList)) {
        // 如果不在聊天列表内，清空输入框
        ui->search_edit->clear();
        ShowSearch(false);
    }
}
void ChatDialog::UpdateChatMsg(std::vector<std::shared_ptr<TextChatData> > msgdata)
{
    for(auto & msg : msgdata){
        if(msg->_from_uid != _cur_chat_uid){
            break;
        }

        ui->chat_page->AppendChatMsg(msg);
    }
}
//添加测试数据
// void ChatDialog::addChatUserList()
// {
//     //先去用户中取出所有朋哟
//     auto friend_list = UserMgr::GetInstance()->GetFriedList();
//     //如果不为空
//     if(!friend_list.empty())
//     {
//         for(auto& friendinfo : friend_list){
//             auto* chat_user_wid = new ChatUserWid();
//             auto user_info= std::make_shared<UserInfo>(friendinfo);
//             chat_user_wid->SetInfo(user_info);
//             QListWidgetItem* item =new QListWidgetItem;
//             //设置item大小
//             item->setSizeHint(chat_user_wid->sizeHint());

//             ui->chat_user_list->addItem(item);
//             ui->chat_user_list->setItemWidget(item,chat_user_wid);
//             _chat_thread_items.insert(friendinfo->_uid,item);
//         }
//     }
//     // 创建QListWidgetItem，并设置自定义的widget
//     for(int i = 0; i < 13; i++){
//         int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
//         int str_i = randomValue%strs.size();
//         int head_i = randomValue%heads.size();
//         int name_i = randomValue%names.size();
//         auto *chat_user_wid = new ChatUserWid();
//         auto user_info_ = std::make_shared<UserInfo>(0,names[name_i],
//                                                      names[name_i],heads[head_i],0,strs[str_i],"");
//         chat_user_wid->SetInfo(user_info_);
//         QListWidgetItem *item = new QListWidgetItem;
//         //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
//         item->setSizeHint(chat_user_wid->sizeHint());
//         ui->chat_user_list->addItem(item);
//         ui->chat_user_list->setItemWidget(item, chat_user_wid);
//     }
// }
//todo: 加载更多联系人，后期从数据库里添加
void ChatDialog::loadMoreChatUser()
{

}
void ChatDialog::on_btnAdd_clicked()
{
    auto* applyFriend = new ApplyFriend(this);
    applyFriend->show();
}

void ChatDialog::slot_appply_friend(std::shared_ptr<AddFriendApply> apply)
{
    qDebug() << "receive apply friend slot, applyuid is " << apply->_from_uid << " name is "
             << apply->_name << " desc is " << apply->_desc;
    bool b_already = UserMgr::GetInstance()->AlreadyApply(apply->_from_uid);
    if(b_already){
        return;
    }
    UserMgr::GetInstance()->AddApplyList(std::make_shared<ApplyInfo>(apply));
    ui->side_contract_lb->ShowRedPoint(true);
    ui->con_user_list->ShowRedPoint(true);
    ui->friend_apply_page->AddNewApply(apply);
}
//根据不同联系人切换聊天界面
void ChatDialog::slot_item_clicked(QListWidgetItem *item)
{
    QWidget* widget = ui->chat_user_list->itemWidget(item);
    if(widget==nullptr){
        qDebug()<<"slot item slicked widget is nullptr";
        return;
    }
    ListItemBase* customitem = qobject_cast<ListItemBase*>(widget);
    if(customitem==nullptr){
        qDebug()<<"slot item slicked customitem is nullptr";
        return;
    }
    //判断类型
    auto itemType = customitem->GetItemType();
    if(itemType == ListItemType::INVALID_ITEM
        || itemType == ListItemType::GROUP_TIP_ITEM){
        qDebug()<<"slot item clicked invaild";
        return;
    }
    //如果是聊天用户item
    if(itemType == ListItemType::CHAT_USER_ITEM){
        // 创建对话框，提示用户
        qDebug() << "contact user item clicked ";

        auto chat_wid = qobject_cast<ChatUserWid*>(customitem);
        auto chat_data = chat_wid->GetChatData();
        //跳转到聊天界面
        ui->chat_page->SetChatData(chat_data);
        _cur_chat_thread_id = chat_data->GetThreadId();
        return;
    }

}
//展示好友的信息
void ChatDialog::slot_friend_info_clicked(std::shared_ptr<UserInfo> userinfo)
{
    qDebug()<<"show fried info page";
    //把界面设置成朋友信息
    _last_widget = ui->friend_info_page;
    ui->stackedWidget->setCurrentWidget(ui->friend_info_page);
    ui->friend_info_page->SetInfo(userinfo);
}
void ChatDialog::slot_reset_icon(QString path) {
    UserMgr::GetInstance()->ResetLabelIcon(path);
}
void ChatDialog::slot_add_img_msg(int thread_id, std::shared_ptr<ImgChatData> img_msg) {
    auto chat_data = UserMgr::GetInstance()->GetChatThreadByThreadId(thread_id);
    if (chat_data == nullptr) {
        return;
    }

    chat_data->MoveMsg(img_msg);

    if (_cur_chat_thread_id != thread_id) {
        return;
    }

    //更新聊天界面信息
    ui->chat_page->UpdateChatStatus(img_msg);
}
void ChatDialog::slot_update_upload_progress(std::shared_ptr<MsgInfo> msg_info) {
    auto chat_data = UserMgr::GetInstance()->GetChatThreadByThreadId(msg_info->_thread_id);
    if (chat_data == nullptr) {
        return;
    }

    //更新消息，其实不用更新，都是共享msg_info的一块内存，这里为了安全还是再次更新下

    chat_data->UpdateProgress(msg_info);

    if (_cur_chat_thread_id != msg_info->_thread_id) {
        return;
    }


    //更新聊天界面信息
    ui->chat_page->UpdateFileProgress(msg_info);
}
void ChatDialog::slot_img_chat_msg(std::shared_ptr<ImgChatData> imgchat) {
    //更新数据
    auto thread_id = imgchat->GetThreadId();
    auto thread_data = UserMgr::GetInstance()->GetChatThreadByThreadId(thread_id);
    thread_data->AddMsg(imgchat);
    if (_cur_chat_thread_id != thread_id) {
        return;
    }
    ui->chat_page->AppendOtherMsg(imgchat);
}
void ChatDialog::slot_update_download_progress(std::shared_ptr<MsgInfo> msg_info) {
    auto chat_data = UserMgr::GetInstance()->GetChatThreadByThreadId(msg_info->_thread_id);
    if (chat_data == nullptr) {
        return;
    }

    //更新消息，其实不用更新，都是共享msg_info的一块内存，这里为了安全还是再次更新下

    chat_data->UpdateProgress(msg_info);

    if (_cur_chat_thread_id != msg_info->_thread_id) {
        return;
    }


    //更新聊天界面信息
    ui->chat_page->UpdateFileProgress(msg_info);
}
void ChatDialog::slot_download_finish(std::shared_ptr<MsgInfo> msg_info, QString file_path) {
    auto chat_data = UserMgr::GetInstance()->GetChatThreadByThreadId(msg_info->_thread_id);
    if (chat_data == nullptr) {
        return;
    }

    //更新消息，其实不用更新，都是共享msg_info的一块内存，这里为了安全还是再次更新下

    chat_data->UpdateProgress(msg_info);

    if (_cur_chat_thread_id != msg_info->_thread_id) {
        return;
    }


    //更新聊天界面信息
    ui->chat_page->DownloadFileFinished(msg_info, file_path);
}

