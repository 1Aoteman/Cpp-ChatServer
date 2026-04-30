#include "timebtn.h"
#include <QMouseEvent>
#include <QDebug>

TimeBtn::TimeBtn(QWidget *parent):QPushButton(parent),_counter(10)
{
    _timer=new QTimer(this);
    connect(_timer,&QTimer::timeout,[this]{
        _counter--;
        if(_counter<=0){
            _timer->stop();
            _counter=10;
            this->setEnabled(true);//按钮可以按
            this->setText("获取");
            return;
        }
        this->setText(QString::number(_counter));
    });
}

TimeBtn::~TimeBtn()
{
    _timer->stop();
}

void TimeBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button()==Qt::LeftButton){
        qDebug()<<"验证吗发送";
        this->setEnabled(false);
        this->setText(QString::number(_counter));
        _timer->start(1000);
        emit clicked();
    }
    // 调用基类的mouseReleaseEvent以确保正常的事件处理（如点击效果）
    QPushButton::mouseReleaseEvent(e);
}

