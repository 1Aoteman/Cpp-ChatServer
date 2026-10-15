/********************************************************************************
** Form generated from reading UI file 'chatdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHATDIALOG_H
#define UI_CHATDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <applyfriendpage.h>
#include <chatpage.h>
#include <contactuserlist.h>
#include <searchlist.h>
#include "chatuserlist.h"
#include "clickedbtn.h"
#include "customizeedit.h"
#include "friendinfopage.h"
#include "statewidget.h"
#include "userinfopage.h"

QT_BEGIN_NAMESPACE

class Ui_ChatDialog
{
public:
    QHBoxLayout *horizontalLayout_main;
    QWidget *side_bar;
    QVBoxLayout *verticalLayout_1;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QLabel *side_head_lb;
    StateWidget *side_chat_lb;
    StateWidget *side_contract_lb;
    StateWidget *side_setting_lb;
    QSpacerItem *verticalSpacer;
    QWidget *chat_user_wid;
    QVBoxLayout *verticalLayout_2;
    QWidget *search_wid;
    QHBoxLayout *horizontalLayout_search;
    CustomizeEdit *search_edit;
    ClickedBtn *btnAdd;
    SearchList *search_list;
    ContactUserList *con_user_list;
    ChatUserList *chat_user_list;
    QStackedWidget *stackedWidget;
    ChatPage *chat_page;
    UserInfoPage *user_info_page;
    ApplyFriendPage *friend_apply_page;
    FriendInfoPage *friend_info_page;

    void setupUi(QDialog *ChatDialog)
    {
        if (ChatDialog->objectName().isEmpty())
            ChatDialog->setObjectName("ChatDialog");
        ChatDialog->resize(809, 485);
        horizontalLayout_main = new QHBoxLayout(ChatDialog);
        horizontalLayout_main->setSpacing(6);
        horizontalLayout_main->setObjectName("horizontalLayout_main");
        horizontalLayout_main->setContentsMargins(0, 0, 0, 0);
        side_bar = new QWidget(ChatDialog);
        side_bar->setObjectName("side_bar");
        verticalLayout_1 = new QVBoxLayout(side_bar);
        verticalLayout_1->setSpacing(30);
        verticalLayout_1->setObjectName("verticalLayout_1");
        widget = new QWidget(side_bar);
        widget->setObjectName("widget");
        widget->setMinimumSize(QSize(56, 56));
        widget->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName("verticalLayout");
        side_head_lb = new QLabel(widget);
        side_head_lb->setObjectName("side_head_lb");
        side_head_lb->setMinimumSize(QSize(35, 35));
        side_head_lb->setMaximumSize(QSize(35, 35));

        verticalLayout->addWidget(side_head_lb);

        side_chat_lb = new StateWidget(widget);
        side_chat_lb->setObjectName("side_chat_lb");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(side_chat_lb->sizePolicy().hasHeightForWidth());
        side_chat_lb->setSizePolicy(sizePolicy);
        side_chat_lb->setMinimumSize(QSize(30, 30));
        side_chat_lb->setMaximumSize(QSize(30, 30));

        verticalLayout->addWidget(side_chat_lb);

        side_contract_lb = new StateWidget(widget);
        side_contract_lb->setObjectName("side_contract_lb");
        side_contract_lb->setMinimumSize(QSize(30, 30));
        side_contract_lb->setMaximumSize(QSize(30, 30));

        verticalLayout->addWidget(side_contract_lb);

        side_setting_lb = new StateWidget(widget);
        side_setting_lb->setObjectName("side_setting_lb");
        side_setting_lb->setMinimumSize(QSize(30, 30));
        side_setting_lb->setMaximumSize(QSize(30, 30));

        verticalLayout->addWidget(side_setting_lb);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        verticalLayout_1->addWidget(widget);


        horizontalLayout_main->addWidget(side_bar);

        chat_user_wid = new QWidget(ChatDialog);
        chat_user_wid->setObjectName("chat_user_wid");
        chat_user_wid->setMinimumSize(QSize(300, 0));
        chat_user_wid->setMaximumSize(QSize(300, 16777215));
        verticalLayout_2 = new QVBoxLayout(chat_user_wid);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        search_wid = new QWidget(chat_user_wid);
        search_wid->setObjectName("search_wid");
        search_wid->setMinimumSize(QSize(0, 60));
        search_wid->setMaximumSize(QSize(16777215, 60));
        horizontalLayout_search = new QHBoxLayout(search_wid);
        horizontalLayout_search->setObjectName("horizontalLayout_search");
        search_edit = new CustomizeEdit(search_wid);
        search_edit->setObjectName("search_edit");
        search_edit->setMinimumSize(QSize(0, 30));

        horizontalLayout_search->addWidget(search_edit);

        btnAdd = new ClickedBtn(search_wid);
        btnAdd->setObjectName("btnAdd");
        btnAdd->setMinimumSize(QSize(30, 30));
        btnAdd->setMaximumSize(QSize(30, 30));

        horizontalLayout_search->addWidget(btnAdd);


        verticalLayout_2->addWidget(search_wid);

        search_list = new SearchList(chat_user_wid);
        search_list->setObjectName("search_list");

        verticalLayout_2->addWidget(search_list);

        con_user_list = new ContactUserList(chat_user_wid);
        con_user_list->setObjectName("con_user_list");

        verticalLayout_2->addWidget(con_user_list);

        chat_user_list = new ChatUserList(chat_user_wid);
        chat_user_list->setObjectName("chat_user_list");

        verticalLayout_2->addWidget(chat_user_list);


        horizontalLayout_main->addWidget(chat_user_wid);

        stackedWidget = new QStackedWidget(ChatDialog);
        stackedWidget->setObjectName("stackedWidget");
        chat_page = new ChatPage();
        chat_page->setObjectName("chat_page");
        stackedWidget->addWidget(chat_page);
        user_info_page = new UserInfoPage();
        user_info_page->setObjectName("user_info_page");
        stackedWidget->addWidget(user_info_page);
        friend_apply_page = new ApplyFriendPage();
        friend_apply_page->setObjectName("friend_apply_page");
        stackedWidget->addWidget(friend_apply_page);
        friend_info_page = new FriendInfoPage();
        friend_info_page->setObjectName("friend_info_page");
        stackedWidget->addWidget(friend_info_page);

        horizontalLayout_main->addWidget(stackedWidget);


        retranslateUi(ChatDialog);

        QMetaObject::connectSlotsByName(ChatDialog);
    } // setupUi

    void retranslateUi(QDialog *ChatDialog)
    {
        ChatDialog->setWindowTitle(QCoreApplication::translate("ChatDialog", "Dialog", nullptr));
        side_head_lb->setText(QString());
        search_edit->setPlaceholderText(QCoreApplication::translate("ChatDialog", "\346\220\234\347\264\242", nullptr));
        btnAdd->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class ChatDialog: public Ui_ChatDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHATDIALOG_H
