/********************************************************************************
** Form generated from reading UI file 'chatpage.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHATPAGE_H
#define UI_CHATPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <chatview.h>
#include <clickedlabel.h>
#include <messagetextedit.h>
#include "clickedbtn.h"

QT_BEGIN_NAMESPACE

class Ui_ChatPage
{
public:
    QHBoxLayout *horizontalLayout_3;
    QWidget *chat_data_wid;
    QVBoxLayout *verticalLayout;
    QWidget *widget_2;
    QVBoxLayout *verticalLayout_4;
    QWidget *title_wid;
    QVBoxLayout *verticalLayout_5;
    QLabel *title_lb;
    ChatView *chat_data_list;
    QWidget *tool_wid;
    QHBoxLayout *horizontalLayout;
    ClickedLabel *emo_lb;
    QSpacerItem *horizontalSpacer;
    ClickedLabel *file_lb;
    QSpacerItem *horizontalSpacer_2;
    MessageTextEdit *chat_text_edit;
    QWidget *send_wid;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_3;
    ClickedBtn *receive_btn;
    QSpacerItem *horizontalSpacer_4;
    ClickedBtn *send_btn;
    QSpacerItem *horizontalSpacer_5;

    void setupUi(QWidget *ChatPage)
    {
        if (ChatPage->objectName().isEmpty())
            ChatPage->setObjectName("ChatPage");
        ChatPage->resize(528, 471);
        horizontalLayout_3 = new QHBoxLayout(ChatPage);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        chat_data_wid = new QWidget(ChatPage);
        chat_data_wid->setObjectName("chat_data_wid");
        chat_data_wid->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout = new QVBoxLayout(chat_data_wid);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        widget_2 = new QWidget(chat_data_wid);
        widget_2->setObjectName("widget_2");
        widget_2->setMaximumSize(QSize(16777215, 50));
        verticalLayout_4 = new QVBoxLayout(widget_2);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        title_wid = new QWidget(widget_2);
        title_wid->setObjectName("title_wid");
        title_wid->setMaximumSize(QSize(16777215, 50));
        verticalLayout_5 = new QVBoxLayout(title_wid);
        verticalLayout_5->setObjectName("verticalLayout_5");
        title_lb = new QLabel(title_wid);
        title_lb->setObjectName("title_lb");

        verticalLayout_5->addWidget(title_lb);


        verticalLayout_4->addWidget(title_wid);


        verticalLayout->addWidget(widget_2);

        chat_data_list = new ChatView(chat_data_wid);
        chat_data_list->setObjectName("chat_data_list");

        verticalLayout->addWidget(chat_data_list);

        tool_wid = new QWidget(chat_data_wid);
        tool_wid->setObjectName("tool_wid");
        tool_wid->setMaximumSize(QSize(16777215, 40));
        horizontalLayout = new QHBoxLayout(tool_wid);
        horizontalLayout->setObjectName("horizontalLayout");
        emo_lb = new ClickedLabel(tool_wid);
        emo_lb->setObjectName("emo_lb");
        emo_lb->setMinimumSize(QSize(30, 30));
        emo_lb->setMaximumSize(QSize(30, 30));

        horizontalLayout->addWidget(emo_lb);

        horizontalSpacer = new QSpacerItem(5, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        file_lb = new ClickedLabel(tool_wid);
        file_lb->setObjectName("file_lb");
        file_lb->setMinimumSize(QSize(30, 30));
        file_lb->setMaximumSize(QSize(30, 30));

        horizontalLayout->addWidget(file_lb);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        verticalLayout->addWidget(tool_wid);

        chat_text_edit = new MessageTextEdit(chat_data_wid);
        chat_text_edit->setObjectName("chat_text_edit");
        chat_text_edit->setMinimumSize(QSize(0, 150));
        chat_text_edit->setMaximumSize(QSize(16777215, 150));

        verticalLayout->addWidget(chat_text_edit);

        send_wid = new QWidget(chat_data_wid);
        send_wid->setObjectName("send_wid");
        send_wid->setMaximumSize(QSize(16777215, 35));
        horizontalLayout_2 = new QHBoxLayout(send_wid);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);

        receive_btn = new ClickedBtn(send_wid);
        receive_btn->setObjectName("receive_btn");

        horizontalLayout_2->addWidget(receive_btn);

        horizontalSpacer_4 = new QSpacerItem(5, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);

        send_btn = new ClickedBtn(send_wid);
        send_btn->setObjectName("send_btn");

        horizontalLayout_2->addWidget(send_btn);

        horizontalSpacer_5 = new QSpacerItem(30, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_5);


        verticalLayout->addWidget(send_wid);


        horizontalLayout_3->addWidget(chat_data_wid);


        retranslateUi(ChatPage);

        QMetaObject::connectSlotsByName(ChatPage);
    } // setupUi

    void retranslateUi(QWidget *ChatPage)
    {
        ChatPage->setWindowTitle(QCoreApplication::translate("ChatPage", "Form", nullptr));
        title_lb->setText(QString());
        emo_lb->setText(QString());
        file_lb->setText(QString());
        receive_btn->setText(QCoreApplication::translate("ChatPage", "\346\216\245\345\217\227", nullptr));
        send_btn->setText(QCoreApplication::translate("ChatPage", "\345\217\221\351\200\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChatPage: public Ui_ChatPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHATPAGE_H
