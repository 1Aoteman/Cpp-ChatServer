/********************************************************************************
** Form generated from reading UI file 'resetdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RESETDIALOG_H
#define UI_RESETDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ReSetDialog
{
public:
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QLabel *err_tip;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QLabel *user_label;
    QLineEdit *user_edit;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *email_label;
    QLineEdit *email_edit;
    QWidget *widget_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *pass_label;
    QLineEdit *pass_edit;
    QWidget *widget_4;
    QHBoxLayout *horizontalLayout_4;
    QLabel *varify_label;
    QLineEdit *varify_edit;
    QPushButton *varify_btn;
    QSpacerItem *verticalSpacer_2;
    QWidget *widget_5;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *confirm_btn;
    QPushButton *concel_btn;
    QSpacerItem *verticalSpacer_3;

    void setupUi(QDialog *ReSetDialog)
    {
        if (ReSetDialog->objectName().isEmpty())
            ReSetDialog->setObjectName("ReSetDialog");
        ReSetDialog->resize(400, 573);
        verticalLayout = new QVBoxLayout(ReSetDialog);
        verticalLayout->setObjectName("verticalLayout");
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        err_tip = new QLabel(ReSetDialog);
        err_tip->setObjectName("err_tip");

        verticalLayout->addWidget(err_tip);

        widget = new QWidget(ReSetDialog);
        widget->setObjectName("widget");
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setObjectName("horizontalLayout");
        user_label = new QLabel(widget);
        user_label->setObjectName("user_label");

        horizontalLayout->addWidget(user_label);

        user_edit = new QLineEdit(widget);
        user_edit->setObjectName("user_edit");

        horizontalLayout->addWidget(user_edit);


        verticalLayout->addWidget(widget);

        widget_2 = new QWidget(ReSetDialog);
        widget_2->setObjectName("widget_2");
        horizontalLayout_2 = new QHBoxLayout(widget_2);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        email_label = new QLabel(widget_2);
        email_label->setObjectName("email_label");

        horizontalLayout_2->addWidget(email_label);

        email_edit = new QLineEdit(widget_2);
        email_edit->setObjectName("email_edit");

        horizontalLayout_2->addWidget(email_edit);


        verticalLayout->addWidget(widget_2);

        widget_3 = new QWidget(ReSetDialog);
        widget_3->setObjectName("widget_3");
        horizontalLayout_3 = new QHBoxLayout(widget_3);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        pass_label = new QLabel(widget_3);
        pass_label->setObjectName("pass_label");

        horizontalLayout_3->addWidget(pass_label);

        pass_edit = new QLineEdit(widget_3);
        pass_edit->setObjectName("pass_edit");

        horizontalLayout_3->addWidget(pass_edit);


        verticalLayout->addWidget(widget_3);

        widget_4 = new QWidget(ReSetDialog);
        widget_4->setObjectName("widget_4");
        horizontalLayout_4 = new QHBoxLayout(widget_4);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        varify_label = new QLabel(widget_4);
        varify_label->setObjectName("varify_label");

        horizontalLayout_4->addWidget(varify_label);

        varify_edit = new QLineEdit(widget_4);
        varify_edit->setObjectName("varify_edit");

        horizontalLayout_4->addWidget(varify_edit);

        varify_btn = new QPushButton(widget_4);
        varify_btn->setObjectName("varify_btn");

        horizontalLayout_4->addWidget(varify_btn);


        verticalLayout->addWidget(widget_4);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        widget_5 = new QWidget(ReSetDialog);
        widget_5->setObjectName("widget_5");
        horizontalLayout_5 = new QHBoxLayout(widget_5);
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        confirm_btn = new QPushButton(widget_5);
        confirm_btn->setObjectName("confirm_btn");

        horizontalLayout_5->addWidget(confirm_btn);

        concel_btn = new QPushButton(widget_5);
        concel_btn->setObjectName("concel_btn");

        horizontalLayout_5->addWidget(concel_btn);


        verticalLayout->addWidget(widget_5);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);


        retranslateUi(ReSetDialog);

        QMetaObject::connectSlotsByName(ReSetDialog);
    } // setupUi

    void retranslateUi(QDialog *ReSetDialog)
    {
        ReSetDialog->setWindowTitle(QCoreApplication::translate("ReSetDialog", "Dialog", nullptr));
        err_tip->setText(QCoreApplication::translate("ReSetDialog", "l", nullptr));
        user_label->setText(QCoreApplication::translate("ReSetDialog", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        email_label->setText(QCoreApplication::translate("ReSetDialog", "\351\202\256\347\256\261\357\274\232  ", nullptr));
        pass_label->setText(QCoreApplication::translate("ReSetDialog", "\346\226\260\345\257\206\347\240\201\357\274\232", nullptr));
        varify_label->setText(QCoreApplication::translate("ReSetDialog", "\351\252\214\350\257\201\347\240\201\357\274\232 ", nullptr));
        varify_btn->setText(QCoreApplication::translate("ReSetDialog", "\350\216\267\345\217\226", nullptr));
        confirm_btn->setText(QCoreApplication::translate("ReSetDialog", "\347\241\256\350\256\244", nullptr));
        concel_btn->setText(QCoreApplication::translate("ReSetDialog", "\345\217\226\346\266\210", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ReSetDialog: public Ui_ReSetDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RESETDIALOG_H
