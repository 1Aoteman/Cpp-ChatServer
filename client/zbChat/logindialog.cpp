#include "logindialog.h"
#include "ui_logindialog.h"
#include "clickedlabel.h"
#include "httpmgr.h"
#include "tcpmgr.h"
#include <QPainterPath>
LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->reg_btn,&QPushButton::clicked,this, &LoginDialog::switchRegister);
    ui->forget_label->SetState("unvisible","unvisible_hover","","visible","visible_hover","");
    ui->forget_label->setCursor(Qt::PointingHandCursor);
    connect(ui->forget_label,&ClickedLabel::clicked,this,&LoginDialog::slot_forget_pwd);
    connect(HttpMgr::GetInstance().get(),&HttpMgr::sig_login_mod_finish,this,&LoginDialog::slot_user_login);
    //单例模板返回的是智能指针
    connect(this,&LoginDialog::sig_tcp_con,TcpMgr::GetInstance().get(),&TcpMgr::slot_tcp_con);
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_con_success,this,&LoginDialog::slot_tcp_finish);
    initHead();
    inithandlers();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}
void LoginDialog::initHead()
{
    // 加载图片
    QPixmap originalPixmap(":/res/head_1.jpg");

    // 设置图片自动缩放
    qDebug()<< originalPixmap.size() << ui->head_label->size();
    originalPixmap = originalPixmap.scaled(ui->head_label->size(),
                                           Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建一个和原始图片相同大小的QPixmap，用于绘制圆角图片
    QPixmap roundedPixmap(originalPixmap.size());
    roundedPixmap.fill(Qt::transparent); // 用透明色填充

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing); // 设置抗锯齿，使圆角更平滑
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 使用QPainterPath设置圆角
    QPainterPath path;
    path.addRoundedRect(0, 0, originalPixmap.width(), originalPixmap.height(), 10, 10); // 最后两个参数分别是x和y方向的圆角半径
    painter.setClipPath(path);

    // 将原始图片绘制到roundedPixmap上
    painter.drawPixmap(0, 0, originalPixmap);
    // 加上这一句，结束绘制操作！
    painter.end();

    // 设置绘制好的圆角图片到QLabel上
    ui->head_label->setPixmap(roundedPixmap);

}
void LoginDialog::slot_forget_pwd()
{
    qDebug()<<"slot forget pwd";
    emit switchReset();
}
void LoginDialog::slot_tcp_finish(bool success)
{
    if(success){
        showTip(tr("正在连接聊天服务器"),true);
        QJsonObject jsonobj;
        jsonobj["uid"] = _uid;
        jsonobj["token"] = _token;

        QJsonDocument doc(jsonobj);
        QString jsonString = doc.toJson(QJsonDocument::Indented);
        TcpMgr::GetInstance()->sig_send_data(ReqId::ID_CHAT_LOGIN,jsonString);
    }else{
        qDebug()<<"网络错误";
    }
}

void LoginDialog::inithandlers()
{
    //初始化回包逻辑
    _handlers.insert(ReqId::ID_LOGIN_USER,[this](QJsonObject jsonobj){
        int error=jsonobj["error"].toInt();
        if(error!=ErrorCodes::SUCCESS){
            showTip(tr("参数错误，登陆失败"),false);
            return;
        }

        QString email =jsonobj["email"].toString();
        //接受服务器端回应，收到进行聊天服务的地址等会信息
        ServerInfo si;
        si.Uid = jsonobj["uid"].toInt();
        si.Host = jsonobj["host"].toString();
        si.Port = jsonobj["port"].toString();
        si.Token = jsonobj["token"].toString();

        _uid = si.Uid;
        _token = si.Token;
        qDebug()<< "email is " << email << " uid is " << si.Uid <<" host is "
                 << si.Host << " Port is " << si.Port << " Token is " << si.Token;
        //收到服务器端回应，发送信号进行tcp长连接；
        emit sig_tcp_con(si);
    });
}
void LoginDialog::on_login_btn_clicked()
{

    if(checkEmailValid()==false){
        return;
    }
    if(checkPassValid()==false){
        return;
    }

    QJsonObject jsonobj;
    jsonobj["email"]=ui->email_edit->text();
    jsonobj["pass"]=ui->pass_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_login"),jsonobj,ReqId::ID_LOGIN_USER,Modules::LOGINMOD);
}
//登录槽函数
void LoginDialog::slot_user_login(ReqId id,QString res, ErrorCodes err){
    if(err!=ErrorCodes::SUCCESS){
        qDebug()<<"网络错误";
        return;
    }
    QJsonDocument jsondoc = QJsonDocument::fromJson(res.toUtf8());
    //判断json是否转化成功
    if(jsondoc.isNull()){
        showTip(tr("json解析失败"),false);
        return;
    }
    //json解析错误
    if(!jsondoc.isObject()){
        showTip(tr("json解析错误"),false);
        return;
    }
    _handlers[id](jsondoc.object());
    return;
}
//增加错误提示
void LoginDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}
void LoginDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()){
        ui->err_tip->clear();
        return;
    }
    showTip(_tip_errs.first(), false);
}
bool LoginDialog::checkEmailValid()
{
    //验证邮箱的地址正则表达式
    auto email = ui->email_edit->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(!match){
        //提示邮箱不正确
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}
bool LoginDialog::checkPassValid()
{
    auto pass = ui->pass_edit->text();
    if(pass.length() < 6 || pass.length()>15){
        //提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }
    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        //提示字符非法
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}
void LoginDialog::showTip(QString str, bool b_ok)
{
    if(b_ok)
    {
        ui->err_tip->setProperty("state","normal");
    }
    else
    {
        ui->err_tip->setProperty("state","err");
    }
    ui->err_tip->setText(str);
    QString currentText = ui->err_tip->text();
    qDebug()<<currentText;
    repolish(ui->err_tip);
}
