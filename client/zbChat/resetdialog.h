#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class ReSetDialog;
}

class ReSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReSetDialog(QWidget *parent = nullptr);
    ~ReSetDialog();

private:
    Ui::ReSetDialog *ui;
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
    void initHandlers();
signals:
    void switchLogin();
private slots:
    void slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
    void on_varify_btn_clicked();
    void on_confirm_btn_clicked();
    void on_concel_btn_clicked();
};

#endif // RESETDIALOG_H
