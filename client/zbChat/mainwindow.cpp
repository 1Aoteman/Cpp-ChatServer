#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tcpmgr.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    //_login_dlg->show();

    //进行链接
    connect(_login_dlg,&LoginDialog::switchRegister,this,&MainWindow::SlotswitchRegister);
    //注册界面
    connect(_login_dlg,&LoginDialog::switchReset,this,&MainWindow::SlotswitchReSet);
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_swich_chatdlg, this, &MainWindow::SlotSwitchChat);
    //服务器踢人下线
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_notify_off_line,this,&MainWindow::SlotOffline);
    //连接服务器断开心跳超时或异常连接信息
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_connect_closed, this, &MainWindow::SlotExcepConOffline);
}

MainWindow::~MainWindow()
{
    delete ui;
    //注销两个界面
//     if(_login_dlg){
//         delete _login_dlg;
//         _login_dlg = nullptr;
//     }

//     if(_register_dlg){
//         delete _register_dlg;
//         _register_dlg = nullptr;
//     }
}

void MainWindow::SlotswitchLogin()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    _register_dlg->hide();
    _login_dlg->show();
    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotswitchRegister);
    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotswitchReSet);
}
void MainWindow::SlotswitchRegister()
{
    _register_dlg = new RegisterDialog(this);//指明父类不会加到对象数中
    _register_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    connect(_register_dlg,&RegisterDialog::sigSwichLohgin,this,&MainWindow::SlotswitchLogin);
    setCentralWidget(_register_dlg);
    _login_dlg->hide();
    _register_dlg->show();
}

void MainWindow::SlotswitchReSet()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _reset_dlg = new ReSetDialog(this);
    _reset_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_reset_dlg);
    _login_dlg->hide();
    _reset_dlg->show();
    //注册返回登录信号和槽函数
    connect(_reset_dlg, &ReSetDialog::switchLogin, this, &MainWindow::SlotSwitchLogin2);
}
void MainWindow::SlotSwitchLogin2()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    _reset_dlg->hide();
    _login_dlg->show();
    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotswitchReSet);
    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotswitchRegister);
}
void MainWindow::SlotSwitchChat()
{
    _chat_dlg = new ChatDialog();
    _chat_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_chat_dlg);
    _chat_dlg->show();
    _login_dlg->hide();
    this->setMinimumSize(QSize(1050,900));
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    _ui_status = CHAT_UI;
    _chat_dlg->loadChatList();
}
void MainWindow::SlotOffline()
{
    // 使用静态方法直接弹出一个信息框
    QMessageBox::information(this, "下线提示", "有其他客户端登录，该终端下线！");
    TcpMgr::GetInstance()->CloseConnection();
    offlineLogin();
}

void MainWindow::SlotExcepConOffline()
{
    // 使用静态方法直接弹出一个信息框
    QMessageBox::information(this, "下线提示", "心跳超时或临界异常，该终端下线！");
    TcpMgr::GetInstance()->CloseConnection();
    offlineLogin();
}
void MainWindow::offlineLogin(){
    if(_ui_status == LOGIN_UI){
        return;
    }
    //通过setcentra qt可能会删除旧界面，
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    _chat_dlg->hide();
    this->setMaximumSize(300,500);
    this->setMinimumSize(300,500);
    this->resize(300,500);
    _login_dlg->show();
    //界面换了，连接和槽也要换
    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotswitchRegister);
    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotswitchReSet);
    _ui_status = LOGIN_UI;
}

