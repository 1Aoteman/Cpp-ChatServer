#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "global.h"
#include "chatuserwid.h"
#include "statewidget.h"
#include "applyfriend.h"

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    void AddLBGroup(StateWidget *lb);
    void ClearLabelState(StateWidget *lb);
public slots:
    void slot_loading_chat_user();
    void slot_side_chat();
    void slot_text_changed(const QString &str);
    void slot_side_contract();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void handleGlobalMousePress(QMouseEvent *event);
private slots:
    void on_btnAdd_clicked();

private:
    void ShowSearch(bool bsearch =false);
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    Ui::ChatDialog *ui;
    //测试数据
    void addChatUserList();
    QList<StateWidget*> _lb_list;
    QWidget* _last_widget;

    std::vector<QString>  strs ={"hello world !",
                                 "nice to meet u",
                                 "New year，new life",
                                 "You have to love yourself",
                                 "My love is written in the wind ever since the whole world is you"};
    std::vector<QString> heads = {
        ":/res/head_1.jpg",
        ":/res/head_2.jpg",
        ":/res/head_3.jpg",
        ":/res/head_4.jpg",
        ":/res/head_5.jpg"
    };
    std::vector<QString> names = {
        "llfc",
        "zack",
        "golang",
        "cpp",
        "java",
        "nodejs",
        "python",
        "rust"
    };
};

#endif // CHATDIALOG_H
