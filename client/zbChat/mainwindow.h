#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "registerdialog.h"
#include "resetdialog.h"
#include "chatdialog.h"
#include <QMessageBox>

QT_BEGIN_NAMESPACE
enum UIStatus{
    LOGIN_UI,
    REGISTER_UI,
    RESET_UI,
    CHAT_UI
};
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void offlineLogin();
private:
    Ui::MainWindow *ui;
    LoginDialog *_login_dlg;
    RegisterDialog *_register_dlg;
    ReSetDialog *_reset_dlg;
    ChatDialog *_chat_dlg;
    UIStatus _ui_status;
public slots:
    void SlotswitchLogin();
    void SlotswitchRegister();
    void SlotswitchReSet();
    void SlotSwitchLogin2();
    void SlotSwitchChat();
    void SlotOffline();
    void SlotExcepConOffline();
};
#endif // MAINWINDOW_H
