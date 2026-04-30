#ifndef TIMEBTN_H
#define TIMEBTN_H
#include <QTimer>
#include <QPushButton>

class TimeBtn : public QPushButton
{
public:
    TimeBtn(QWidget *parent = nullptr);//对象树机制，指明父指针，
    ~TimeBtn();
    void mouseReleaseEvent(QMouseEvent *e) override;
private:
    QTimer *_timer;
    int _counter;
};

#endif // TIMEBTN_H
