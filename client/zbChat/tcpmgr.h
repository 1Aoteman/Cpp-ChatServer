#pragma once
#ifndef TCPMGR_H
#define TCPMGR_H

#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include "singleton.h"
#include "global.h"
#include "userdata.h"
#include <qthread.h>
#include <QQueue>

class TcpThread:public std::enable_shared_from_this<TcpThread> {
public:
    TcpThread();
    ~TcpThread();
private:
    QThread* _tcp_thread;
};

class TcpMgr:public QObject,public Singleton<TcpMgr>,public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
public:
    TcpMgr();
    void CloseConnection();
private:
    void inithandler();
    void dealmsg(ReqId id, int len, QByteArray data);
    void registerMetaType();
    void CreatePlaceholderImgMsgL(QString img_path_str, QString msg_content,
                                          int msg_id, int thread_id, int send_uid, int recv_id, int status, QString chat_time,
                                  std::vector<std::shared_ptr<ChatDataBase>> &chat_datas);
    QTcpSocket _socket;
    QString _host;
    quint16 _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    qint16 _msg_id;
    qint16 _msg_len;
    QMap<ReqId,std::function<void(ReqId id, int len, QByteArray data)>> _handler;
    //发送队列
    QQueue<QByteArray> _send_queue;
    //正在发送的包
    QByteArray  _current_block;
    //当前已发送的字节数
    qint64        _bytes_sent;
    //是否正在发送
    bool _pending;
signals:
    void sig_con_success(bool success);
    void sig_send_data(ReqId id,QByteArray data);
    void sig_login_failed(int err);
    void sig_swich_chatdlg();
    void sig_user_search(std::shared_ptr<SearchInfo>);
    void sig_auth_rsp(std::shared_ptr<AuthRsp>);
    void sig_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
    void sig_friend_apply(std::shared_ptr<AddFriendApply>);
    void sig_text_chat_msg(std::vector<std::shared_ptr<TextChatData>> msg_list);
    void sig_connect_closed();
    //通知下线
    void sig_notify_off_line();
    //加载聊天消息
    void sig_load_chat_thread(bool load_more,int last_thread_id,std::vector<std::shared_ptr<ChatThreadInfo>> chat_threads);
    void sig_create_private_chat(int uid, int other_id, int thread_id);
    void sig_load_chat_msg(int thread_id, int message_id, bool load_more,
                           std::vector<std::shared_ptr<ChatDataBase>> msg_list);
    void sig_chat_msg_rsp(int thread_id, std::vector<std::shared_ptr<TextChatData>> msg_list);
    void sig_chat_img_rsp(int,std::shared_ptr<ImgChatData>);
    void sig_img_chat_msg(std::shared_ptr<ImgChatData>);
    void sig_video_call_event(QJsonObject event);
public slots:
    void slot_tcp_con(std::shared_ptr<ServerInfo> si);
    void slot_send_data(ReqId id,QByteArray data);
};

#endif // TCPMGR_H
