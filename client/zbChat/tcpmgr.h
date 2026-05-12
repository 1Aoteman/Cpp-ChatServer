#pragma once
#ifndef TCPMGR_H
#define TCPMGR_H

#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include "singleton.h"
#include "global.h"
#include "userdata.h"

class TcpMgr:public QObject,public Singleton<TcpMgr>,public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
public:
    TcpMgr();
private:
    void inithandler();
    void dealmsg(ReqId id, int len, QByteArray data);
    QTcpSocket _socket;
    QString _host;
    quint16 _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    qint16 _msg_id;
    qint16 _msg_len;
    QMap<ReqId,std::function<void(ReqId id, int len, QByteArray data)>> _handler;
signals:
    void sig_con_success(bool success);
    void sig_send_data(ReqId id,QString data);
    void sig_login_failed(int err);
    void sig_swich_chatdlg();
    void sig_user_search(std::shared_ptr<SearchInfo>);
public slots:
    void slot_tcp_con(ServerInfo &si);
    void slot_send_data(ReqId id,QString data);
};

#endif // TCPMGR_H
