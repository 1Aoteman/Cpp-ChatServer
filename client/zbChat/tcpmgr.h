#pragma once
#ifndef TCPMGR_H
#define TCPMGR_H

#include <QTcpSocket>
#include "singleton.h"
#include "global.h"

class TcpMgr:public QObject,public Singleton<TcpMgr>,public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
public:
    TcpMgr();
private:
    QTcpSocket _socket;
    QString _host;
    quint16 _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    qint16 _msg_id;
    qint16 _msg_len;
signals:
    void sig_con_success(bool success);
    void sig_send_data(ReqId id,QString data);
public slots:
    void slot_tcp_con(ServerInfo &si);
    void slot_send_data(ReqId id,QString data);
};

#endif // TCPMGR_H
