#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include "global.h"
#include "chatitembase.h"
#include "textbubble.h"
#include "picturebubble.h"

namespace Ui {
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();

private slots:
    void on_send_btn_clicked();

private:
    Ui::ChatPage *ui;
    void paintEvent(QPaintEvent *event);
};

#endif // CHATPAGE_H
