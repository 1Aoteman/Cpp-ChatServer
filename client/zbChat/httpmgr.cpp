#include "httpmgr.h"

void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{
    //将json数据转成
    QByteArray data = QJsonDocument(json).toJson();
    //通过url构建请求
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,QByteArray::number(data.length()));
    auto self = shared_from_this();
    //接受回
    QNetworkReply *reply =_manager.post(request,data);
    QObject::connect(reply,&QNetworkReply::finished,[reply,self,req_id,mod](){
        if(reply->error()!=QNetworkReply::NoError)//如果有错
        {
            qDebug()<<reply->errorString();
            emit self->sig_http_finish(req_id,"",ErrorCodes::ERR_NETWORK,mod);
            reply->deleteLater();
            return;
        }
        //如果没错
        QString res=reply->readAll();
        //发送信号通知已经完成。
        emit self->sig_http_finish(req_id,res,ErrorCodes::SUCCESS,mod);
        reply->deleteLater();
        return;
    });
}


HttpMgr::HttpMgr() {
    //链接http的信号和槽
    connect(this,&HttpMgr::sig_http_finish,this,&HttpMgr::slot_http_finish);
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if(mod==Modules::REGIDTERMOD)//是注册模块
    {
        emit sig_reg_mod_finish(id,res,err);
    }
    if(mod==Modules::RESETMOD)
    {
        emit sig_reset_mod_finish(id,res,err);
    }
    if(mod==Modules::LOGINMOD){
        emit sig_login_mod_finish(id,res,err);
    }
}
