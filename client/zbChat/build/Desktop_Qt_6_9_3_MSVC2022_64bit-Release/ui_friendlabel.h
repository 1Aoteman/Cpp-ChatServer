/********************************************************************************
** Form generated from reading UI file 'friendlabel.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FRIENDLABEL_H
#define UI_FRIENDLABEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <clickedlabel.h>

QT_BEGIN_NAMESPACE

class Ui_FriendLabel
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *tip_lb;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_2;
    ClickedLabel *close_lb;

    void setupUi(QWidget *FriendLabel)
    {
        if (FriendLabel->objectName().isEmpty())
            FriendLabel->setObjectName("FriendLabel");
        FriendLabel->resize(400, 43);
        FriendLabel->setMinimumSize(QSize(0, 43));
        FriendLabel->setMaximumSize(QSize(16777215, 43));
        horizontalLayout = new QHBoxLayout(FriendLabel);
        horizontalLayout->setObjectName("horizontalLayout");
        tip_lb = new QLabel(FriendLabel);
        tip_lb->setObjectName("tip_lb");

        horizontalLayout->addWidget(tip_lb);

        widget = new QWidget(FriendLabel);
        widget->setObjectName("widget");
        widget->setMinimumSize(QSize(25, 25));
        widget->setMaximumSize(QSize(25, 25));
        horizontalLayout_2 = new QHBoxLayout(widget);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        close_lb = new ClickedLabel(widget);
        close_lb->setObjectName("close_lb");
        close_lb->setMinimumSize(QSize(15, 15));
        close_lb->setMaximumSize(QSize(15, 15));

        horizontalLayout_2->addWidget(close_lb);


        horizontalLayout->addWidget(widget);


        retranslateUi(FriendLabel);

        QMetaObject::connectSlotsByName(FriendLabel);
    } // setupUi

    void retranslateUi(QWidget *FriendLabel)
    {
        FriendLabel->setWindowTitle(QCoreApplication::translate("FriendLabel", "Form", nullptr));
        tip_lb->setText(QCoreApplication::translate("FriendLabel", "TextLabel", nullptr));
        close_lb->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class FriendLabel: public Ui_FriendLabel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FRIENDLABEL_H
