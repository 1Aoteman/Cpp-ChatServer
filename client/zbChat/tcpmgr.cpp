#include "tcpmgr.h"
#include "usermgr.h"

TcpMgr::TcpMgr():_host("") {
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
        UserMgr::GetInstance()->SetUid(jsonObj["uid"].toInt());
        UserMgr::GetInstance()->SetName(jsonObj["name"].toString());
        UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());
        emit sig_swich_chatdlg();
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

void TcpMgr::slot_send_data(ReqId id,QString data )
{
    uint16_t _id =id;
    uint16_t _len=data.length();
    QByteArray block;
    //使用流想block中注入数据
    QDataStream out(&block,QIODevice::ReadOnly);
    //放入消息头
    out>>_id>>_len;
    //放进要传递的消息
    QByteArray message = data.toUtf8();
    block.append(message);
    _socket.write(block);
}
