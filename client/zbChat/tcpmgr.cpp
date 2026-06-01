#include "tcpmgr.h"
#include "usermgr.h"
#include "userdata.h"

TcpMgr::TcpMgr():_host(""),_port(0),_b_recv_pending(false),_msg_id(0),_msg_len(0)
{
    connect(&_socket,&QTcpSocket::connected,[&]{
        qDebug()<<"tcp connect success";
        emit sig_con_success(true);
    });
    connect(&_socket,&QTcpSocket::readyRead,[this]{
        //全部加入缓存
        _buffer.append(_socket.readAll());
        forever{
            QDataStream stream(&_buffer,QIODevice::ReadOnly);
            //如果没有剩余未读的，从消息头开始处理
            if(!_b_recv_pending){
                if(_buffer.size()<sizeof(quint16)*2){
                    return;
                }
                //从流中抽出消息头
                stream>>_msg_id>>_msg_len;
                //buffer也截取
                _buffer.remove(0,sizeof(quint16)*2);

            }
            if(_buffer.size()<_msg_len){
                //有未读完的数据，把——b_recv_pending置为true
                _b_recv_pending=true;
                return;
            }
            _b_recv_pending =false;
            //读取消息体
            QByteArray msgbody = _buffer.mid(0,_msg_len);
            qDebug()<<"消息长度是"<<msgbody;
            //处理消息
            dealmsg(ReqId(_msg_id),_msg_len,msgbody);
            _buffer = _buffer.mid(_msg_len);
        }
        //5.15 之后版本
        QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
            Q_UNUSED(socketError)
            qDebug() << "Error:" << _socket.errorString();
            });
    });
    connect(&_socket,&QTcpSocket::disconnected,[]{
        qDebug()<<"连接断开";
    });
    connect(this,&TcpMgr::sig_send_data,this,&TcpMgr::slot_send_data);
    inithandler();
}

void TcpMgr::inithandler()
{
    _handler.insert(ID_CHAT_LOGIN_RSP,[this](ReqId id, int len, QByteArray data){
        qDebug()<<"request id id"<<id<<"data is"<<data;
        //转化为json
        QJsonDocument jsondoc =QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if(jsondoc.isNull()){
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }
        QJsonObject jsonObj = jsondoc.object();
        if(!jsonObj.contains("error")){
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login Failed, err is Json Parse Err" << err ;
            emit sig_login_failed(err);
            return;
        }
        int err = jsonObj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Login Failed, err is " << err ;
            emit sig_login_failed(err);
            return;
        }
        auto uid =jsonObj["uid"].toInt();
        auto name = jsonObj["name"].toString();
        auto nick = jsonObj["nick"].toString();
        auto icon = jsonObj["icon"].toString();
        auto desc =jsonObj["icon"].toString();
        auto sex =jsonObj["sex"].toInt();
        auto user_info = std::make_shared<UserInfo>(uid,name,nick, icon, sex);
        UserMgr::GetInstance()->SetUserInfo(user_info);
        UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());
        //加载朋友列表
        if(jsonObj.contains("apply_list")){
            UserMgr::GetInstance()->AppendApplyList(jsonObj["apply_list"].toArray());
        }
        if(jsonObj.contains("friend_list")){
            UserMgr::GetInstance()->AppendFriendList(jsonObj["friend_list"].toArray());
        }
        emit sig_swich_chatdlg();
    });
    //收到服务器端查询回复
    _handler.insert(ID_SEARCH_USER_RSP,[this](ReqId id, int len, QByteArray data){
        QJsonDocument jsondoc = QJsonDocument::fromJson(data);
        if(jsondoc.isNull()){
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }
        QJsonObject jsonobj =jsondoc.object();
        if(!jsonobj.contains("error")){
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "search Failed, err is Json Parse Err" << err ;
            //这里应该发送查询失败信号
            return;
        }
        int err = jsonobj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Search Failed, err is " << err ;
            return;
        }
        auto search_info =std::make_shared<SearchInfo>(jsonobj["uid"].toInt(),
        jsonobj["name"].toString(), jsonobj["nick"].toString(),
                                                        jsonobj["desc"].toString(), jsonobj["sex"].toInt(), jsonobj["icon"].toString());
        emit sig_user_search(search_info);
    });
    _handler.insert(ID_NOTIFY_ADD_FRIEND_REQ,[this](ReqId id, int len, QByteArray data){
        QJsonDocument jsondoc = QJsonDocument::fromJson(data);
        if(jsondoc.isNull()){
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }
        QJsonObject jsonobj =jsondoc.object();
        if(!jsonobj.contains("error")){
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Add friend Failed, err is Json Parse Err" << err ;
            //这里应该发送查询失败信号
            return;
        }
        int err = jsonobj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Add friend Failed, err is " << err ;
            return;
        }
        int from_uid = jsonobj["applyuid"].toInt();
        QString name = jsonobj["name"].toString();
        QString desc = jsonobj["desc"].toString();
        QString icon = jsonobj["icon"].toString();
        QString nick = jsonobj["nick"].toString();
        int sex = jsonobj["sex"].toInt();
        auto apply_info = std::make_shared<AddFriendApply>(from_uid,name,desc,icon,nick,sex);

        emit sig_friend_apply(apply_info);
    });
    _handler.insert(ID_AUTH_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Auth Friend Failed, err is Json Parse Err" << err;
            return;
        }
        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Auth Friend Failed, err is " << err;
            return;
        }
        auto name = jsonObj["name"].toString();
        auto nick = jsonObj["nick"].toString();
        auto icon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();
        auto uid = jsonObj["uid"].toInt();
        auto rsp = std::make_shared<AuthRsp>(uid, name, nick, icon, sex);
        emit sig_auth_rsp(rsp);
        qDebug() << "Auth Friend Success " ;
    });
    _handler.insert(ID_NOTIFY_AUTH_FRIEND_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Auth Friend Failed, err is " << err;
            return;
        }
        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Auth Friend Failed, err is " << err;
            return;
        }
        int from_uid = jsonObj["fromuid"].toInt();
        QString name = jsonObj["name"].toString();
        QString nick = jsonObj["nick"].toString();
        QString icon = jsonObj["icon"].toString();
        int sex = jsonObj["sex"].toInt();
        auto auth_info = std::make_shared<AuthInfo>(from_uid,name,
                                                    nick, icon, sex);
        emit sig_add_auth_friend(auth_info);
    });
    _handler.insert(ID_TEXT_CHAT_MSG_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Chat Msg Rsp Failed, err is Json Parse Err" << err;
            return;
        }
        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Chat Msg Rsp Failed, err is " << err;
            return;
        }
        qDebug() << "Receive Text Chat Rsp Success " ;
        //ui设置送达等标记 todo...
    });
    _handler.insert(ID_NOTIFY_TEXT_CHAT_MSG_REQ, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Notify Chat Msg Failed, err is Json Parse Err" << err;
            return;
        }
        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Notify Chat Msg Failed, err is " << err;
            return;
        }
        qDebug() << "Receive Text Chat Notify Success " ;
        auto msg_ptr = std::make_shared<TextChatMsg>(jsonObj["fromuid"].toInt(),
                                                     jsonObj["touid"].toInt(),jsonObj["text_array"].toArray());
        emit sig_text_chat_msg(msg_ptr);
    });
}

void TcpMgr::dealmsg(ReqId id, int len, QByteArray data)
{
    auto find_iter =  _handler.find(id);
    if(find_iter == _handler.end()){
        qDebug()<< "not found id ["<< id << "] to handle";
        return ;
    }
    find_iter.value()(id,len,data);
}

void TcpMgr::slot_tcp_con(ServerInfo &si)
{
    qDebug()<<"与服务器进行连接";
    _host=si.Host;
    _port=static_cast<uint16_t>(si.Port.toUInt());
    _socket.connectToHost(_host,_port);
}

void TcpMgr::slot_send_data(ReqId id,QByteArray data )
{
    uint16_t _id =id;
    quint16 _len = static_cast<quint16>(data.length());
    QByteArray block;
    //使用流想block中注入数据
    QDataStream out(&block,QIODevice::WriteOnly);
    //放入消息头
    out<<_id<<_len;
    //放进要传递的消息

    block.append(data);
    _socket.write(block);
}
