#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "global.h"
#include <QPainter>
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    Ui::LoginDialog *ui;
    QMap<TipErr,QString> _tip_errs;
    void initHead();
    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);
    bool checkEmailValid();
    bool checkPassValid();
    void showTip(QString str, bool b_ok);
    QMap<ReqId,std::function<void(const QJsonObject&)>> _handlers;
    void inithandlers();
    int _uid;
    QString _token;
signals:
    void switchRegister();
    void switchReset();
    void sig_tcp_con(ServerInfo &si);
private slots:
    void slot_forget_pwd();
    void slot_user_login(ReqId id,QString res, ErrorCodes err);
    void on_login_btn_clicked();
    void slot_tcp_finish(bool success);
};

#endif // LOGINDIALOG_H
