#ifndef HTTPMGR_H
#define HTTPMGR_H
#include "SIngleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>
#include "global.h"
#include <memory>
#include <QJsonObject>
#include <QJsonDocument>

class HttpMgr:public QObject,public Singleton<HttpMgr>,public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT;
    friend class Singleton<HttpMgr>;
public:

    void PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);
private:
    friend class Singleton<HttpMgr>;
    //发送post http请求
    QNetworkAccessManager _manager;
    HttpMgr();

public slots:
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
signals:
    void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
    void sig_reg_mod_finish(ReqId id,QString res, ErrorCodes err);//注册模块信号
    void sig_reset_mod_finish(ReqId id,QString res, ErrorCodes err);//重置密码信号
    void sig_login_mod_finish(ReqId id,QString res, ErrorCodes err);//登陆信号
};

#endif // HTTPMGR_H
