#include "chatuserwid.h"
#include "ui_chatuserwid.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include "filetcpmgr.h"

ChatUserWid::ChatUserWid(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CHAT_USER_ITEM);
    ui->verticalLayout_2->setAlignment(Qt::AlignTop);
}
ChatUserWid::~ChatUserWid()
{
    delete ui;
}
void ChatUserWid::SetInfo(QString name, QString head, QString msg)
{
    _name = name;
    _head = head;
    _msg = msg;
    // 加载图片
    QPixmap pixmap(_head);
    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    ui->user_name_lb->setText(_name);
    ui->user_chat_lb->setText(_msg);
}
void ChatUserWid::SetInfo(std::shared_ptr<UserInfo> userinfo){

    _user_info = userinfo;
    QPixmap pixmap(_user_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_user_info->_name);
    ui->user_chat_lb->setText(_user_info->_last_msg);
}
void ChatUserWid::SetInfo(std::shared_ptr<FriendInfo> friendinfo){

    _user_info = std::make_shared<UserInfo>(friendinfo);
    QPixmap pixmap(_user_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_user_info->_name);
    ui->user_chat_lb->setText(_user_info->_last_msg);
}
void ChatUserWid::updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs) {

    QString last_msg = "";
    for (auto& msg : msgs) {
        last_msg = msg->_msg_content;
        _user_info->_chat_msgs.push_back(msg);
    }

    _user_info->_last_msg = last_msg;
    ui->user_chat_lb->setText(_user_info->_last_msg);
}

void ChatUserWid::SetChatData(std::shared_ptr<ChatThreadData> chat_data) {
    _chat_data = chat_data;
    auto other_id = _chat_data->GetOtherId();
    auto other_info = UserMgr::GetInstance()->GetFriendById(other_id);
    if (!other_info) {
        qWarning() << "Friend not found for chat user:" << other_id;
        return;
    }

    auto showIcon = [this](const QPixmap &pixmap) {
        ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(),
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
        ui->icon_lb->setScaledContents(true);
    };
    const QPixmap fallback(":/res/head_1.jpg");
    QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
    if (regex.match(other_info->_icon).hasMatch()) {
        const QPixmap pixmap(other_info->_icon);
        showIcon(pixmap.isNull() ? fallback : pixmap);
    } else {
        const QString fileName = QFileInfo(other_info->_icon).fileName();
        QDir avatarsDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                        + "/avatars");
        const QString avatarPath = avatarsDir.filePath(fileName);
        const QPixmap pixmap(avatarPath);
        if (!pixmap.isNull()) {
            showIcon(pixmap);
        } else {
            showIcon(fallback);
            if (!fileName.isEmpty() && avatarsDir.mkpath(".")) {
                auto manager = UserMgr::GetInstance();
                manager->AddLabelToReset(avatarPath, ui->icon_lb);
                if (!manager->IsDownLoading(fileName)) {
                    auto download = std::make_shared<DownloadInfo>();
                    download->_name = fileName;
                    download->_client_path = avatarPath;
                    download->_seq = 1;
                    download->_current_size = 0;
                    download->_total_size = 0;
                    manager->AddDownloadFile(fileName, download);
                    FileTcpMgr::GetInstance()->SendDownloadInfo(download);
                }
            } else {
                qWarning() << "Cannot prepare avatar directory or filename:" << avatarPath;
            }
        }
    }

    ui->user_name_lb->setText(other_info->_name);

    ui->user_chat_lb->setText(chat_data->GetLastMsg());
}
std::shared_ptr<ChatThreadData> ChatUserWid::GetChatData()
{
    return _chat_data;
}
std::shared_ptr<UserInfo> ChatUserWid::GetUserInfo()
{
    return _user_info;
}

