#include "chatuserwid.h"
#include "ui_chatuserwid.h"
#include <QStandardPaths>
#include <QDir>

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
    // 加载图片

    QString head_icon = UserMgr::GetInstance()->GetIcon();

    // 使用正则表达式检查是否是默认头像
    QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
    QRegularExpressionMatch match = regex.match(other_info->_icon);

    if (match.hasMatch()) {
        // 如果是默认头像（:/res/head_X.jpg 格式）
        QPixmap pixmap(other_info->_icon); // 加载默认头像图片
        QPixmap scaledPixmap = pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->icon_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
        ui->icon_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
    }
    else {
        // 如果是用户上传的头像，获取存储目录
        QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir avatarsDir(storageDir + "/avatars");

        // 确保目录存在
        if (avatarsDir.exists()) {
            QString avatarPath = avatarsDir.filePath(QFileInfo(other_info->_icon).fileName()); // 获取上传头像的完整路径
            QPixmap pixmap(avatarPath); // 加载上传的头像图片
            if (!pixmap.isNull()) {
                QPixmap scaledPixmap = pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                ui->icon_lb->setPixmap(scaledPixmap);
                ui->icon_lb->setScaledContents(true);
            }
            else {
                qWarning() << "无法加载上传的头像：" << avatarPath;
            }
        }
        else {
            qWarning() << "头像存储目录不存在：" << avatarsDir.path();
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

