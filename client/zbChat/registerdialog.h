#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H
#include "global.h"
#include <QDialog>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

signals:
    void sigSwichLohgin();

private slots:
    void on_get_code_clicked();
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    void on_sure_btn_clicked();

    void on_ret_btn_clicked();

    void on_concel_btn_clicked();

private:
    void InitHttpHandlers();
    Ui::RegisterDialog *ui;
    void showTip(QString str, bool b_ok);
    QMap<TipErr,QString> _tip_errs;
    QMap<ReqId,std::function<void(const QJsonObject&)>> _handlers;
    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    bool checkConfirmValid();
    bool checkVarifyValid();
    void ChangeTipPage();
    QTimer *_countdown_time;
    int _countdown;
};

#endif // REGISTERDIALOG_H
