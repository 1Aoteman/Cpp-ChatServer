/********************************************************************************
** Form generated from reading UI file 'chatuserwid.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHATUSERWID_H
#define UI_CHATUSERWID_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChatUserWid
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *icon_lb;
    QWidget *widget_2;
    QVBoxLayout *verticalLayout_2;
    QLabel *user_name_lb;
    QLabel *user_chat_lb;
    QWidget *time_lb_2;
    QVBoxLayout *verticalLayout_3;
    QLabel *time_lb;

    void setupUi(QWidget *ChatUserWid)
    {
        if (ChatUserWid->objectName().isEmpty())
            ChatUserWid->setObjectName("ChatUserWid");
        ChatUserWid->resize(400, 85);
        ChatUserWid->setMinimumSize(QSize(0, 85));
        ChatUserWid->setMaximumSize(QSize(16777215, 85));
        horizontalLayout = new QHBoxLayout(ChatUserWid);
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(2, 2, 2, 2);
        icon_lb = new QLabel(ChatUserWid);
        icon_lb->setObjectName("icon_lb");
        icon_lb->setMinimumSize(QSize(50, 50));
        icon_lb->setMaximumSize(QSize(50, 50));

        horizontalLayout->addWidget(icon_lb);

        widget_2 = new QWidget(ChatUserWid);
        widget_2->setObjectName("widget_2");
        verticalLayout_2 = new QVBoxLayout(widget_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        user_name_lb = new QLabel(widget_2);
        user_name_lb->setObjectName("user_name_lb");

        verticalLayout_2->addWidget(user_name_lb);

        user_chat_lb = new QLabel(widget_2);
        user_chat_lb->setObjectName("user_chat_lb");

        verticalLayout_2->addWidget(user_chat_lb);


        horizontalLayout->addWidget(widget_2);

        time_lb_2 = new QWidget(ChatUserWid);
        time_lb_2->setObjectName("time_lb_2");
        verticalLayout_3 = new QVBoxLayout(time_lb_2);
        verticalLayout_3->setObjectName("verticalLayout_3");
        time_lb = new QLabel(time_lb_2);
        time_lb->setObjectName("time_lb");
        time_lb->setMinimumSize(QSize(50, 50));
        time_lb->setMaximumSize(QSize(50, 50));

        verticalLayout_3->addWidget(time_lb);


        horizontalLayout->addWidget(time_lb_2);


        retranslateUi(ChatUserWid);

        QMetaObject::connectSlotsByName(ChatUserWid);
    } // setupUi

    void retranslateUi(QWidget *ChatUserWid)
    {
        ChatUserWid->setWindowTitle(QCoreApplication::translate("ChatUserWid", "Form", nullptr));
        icon_lb->setText(QCoreApplication::translate("ChatUserWid", "TextLabel", nullptr));
        user_name_lb->setText(QCoreApplication::translate("ChatUserWid", "zb", nullptr));
        user_chat_lb->setText(QCoreApplication::translate("ChatUserWid", "hello world", nullptr));
        time_lb->setText(QCoreApplication::translate("ChatUserWid", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChatUserWid: public Ui_ChatUserWid {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHATUSERWID_H
