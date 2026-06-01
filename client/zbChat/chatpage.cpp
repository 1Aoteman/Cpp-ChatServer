#include "chatpage.h"
#include "ui_chatpage.h"
#include "listitembase.h"
#include <QVector>
#include "tcpmgr.h"

ChatPage::ChatPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatPage)
{
    ui->setupUi(this);
    //设置按钮样式
    ui->receive_btn->SetState("normal","hover","press");
    ui->send_btn->SetState("normal","hover","press");
    //设置图标样式
    ui->emo_lb->SetState("normal","hover","press","normal","hover","press");
    ui->file_lb->SetState("normal","hover","press","normal","hover","press");

}
void ChatPage::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

ChatPage::~ChatPage()
{
    delete ui;
}
void ChatPage::AppendChatMsg(std::shared_ptr<TextChatData> msg)
{
    auto self_info = UserMgr::GetInstance()->GetUserInfo();
    ChatRole role;
    //todo... 添加聊天显示
    if (msg->_from_uid == self_info->_uid) {
        role = ChatRole::Self;
        ChatItemBase* pChatItem = new ChatItemBase(role);

        pChatItem->setUserName(self_info->_name);
        pChatItem->setUserIcon(QPixmap(self_info->_icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chat_data_list->appendChatItem(pChatItem);
    }
    else {
        role = ChatRole::Other;
        ChatItemBase* pChatItem = new ChatItemBase(role);
        auto friend_info = UserMgr::GetInstance()->GetFriendById(msg->_from_uid);
        if (friend_info == nullptr) {
            return;
        }
        pChatItem->setUserName(friend_info->_name);
        pChatItem->setUserIcon(QPixmap(friend_info->_icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chat_data_list->appendChatItem(pChatItem);
    }


}
void ChatPage::SetUserInfo(std::shared_ptr<UserInfo> userinfo)
{
    _user_info = userinfo;
    ui->title_lb->setText(userinfo->_name);
}

void ChatPage::on_send_btn_clicked()
{
    //发送消息；
    //获取要发送对象的用户信息；
    //如果对象数据为空
    if(_user_info==nullptr){
        qDebug()<<"invaild user";
        return;
    }
    std::shared_ptr<UserInfo> self_info = UserMgr::GetInstance()->GetUserInfo();
    auto pTextedit = ui->chat_text_edit;
    //设置职责，根据职责来设置气泡的位置;
    ChatRole role = ChatRole::Self;
    QString username = self_info->_name;
    QString usericon =self_info->_icon;
    //取出聊天筐里的数据
    const QVector<MsgInfo>& msgList=pTextedit->getMsgList();
    QJsonObject textObj;
    QJsonArray textArray;
    //信息大小
    int txt_size =0;

    for(int i=0;i<msgList.size();i++){
        //判断消息的长度是否符合要求
        if(msgList[i].content.length()>1024){
            qDebug()<<"消息长度非法";
            continue;
        }
        //判断消息类型
        QString type = msgList[i].msgFlag;
        ChatItemBase* pChatItem= new ChatItemBase(role);
        pChatItem->setUserName(username);
        pChatItem->setUserIcon(QPixmap(usericon));
        QWidget *pBubble = nullptr;
        if(type == "text"){
            //生成消息的id
            QUuid uuid = QUuid::createUuid();
            QString msg_id =uuid.toString();
            pBubble = new TextBubble(role,msgList[i].content);
            //如果消息大于1024了，直接发送之前的
            if(txt_size+msgList[i].content.length()>1024){
                textObj["from_uid"] = self_info->_uid;
                textObj["tou_id"] = _user_info->_uid;
                textObj["text_array"] = textArray;
                QJsonDocument doc(textObj);
                QByteArray jsondata = doc.toJson(QJsonDocument::Compact);
                //重新设置已经发送为消息大小
                txt_size=0;
                textObj = QJsonObject();
                textArray = QJsonArray();
                //发送消息
                emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ,jsondata);
            }
            //将bubble和uid绑定，以后可以等网络返回消息后设置是否送达
            //_bubble_map[uuidString] = pBubble;
            txt_size= msgList[i].content.length();
            QJsonObject obj;
            QByteArray utf8msg = msgList[i].content.toUtf8();
            obj["content"] = QString::fromUtf8(utf8msg);
            obj["msgid"] = msg_id;
            textArray.append(obj);
            auto chatmsg = std::make_shared<TextChatData>(msg_id,obj["content"].toString(),self_info->_uid,_user_info->_uid);
            emit sig_append_send_chat_msg(chatmsg);
        }else if(type=="image"){

        }
        else if(type=="file"){

        }
        if(pBubble!=nullptr){
            pChatItem ->setWidget(pBubble);
            //把界面放入聊天中
            ui->chat_data_list->appendChatItem(pChatItem);
        }
    }
    qDebug()<<"textArray is "<<textArray;
    textObj["text_array"] = textArray;
    textObj["from_uid"] = self_info->_uid;
    textObj["to_uid"] =_user_info->_uid;
    QJsonDocument jsondoc(textObj);
    QByteArray chat_msg = jsondoc.toJson(QJsonDocument::Compact);
    //重新设置已经发送为消息大小
    txt_size=0;
    textObj = QJsonObject();
    textArray = QJsonArray();
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ,chat_msg);

}

