/********************************************************************************
** Form generated from reading UI file 'loadingdlg.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOADINGDLG_H
#define UI_LOADINGDLG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoadingDlg
{
public:
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *status_lb;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QLabel *loading_lb;
    QSpacerItem *verticalSpacer_2;

    void setupUi(QDialog *LoadingDlg)
    {
        if (LoadingDlg->objectName().isEmpty())
            LoadingDlg->setObjectName("LoadingDlg");
        LoadingDlg->resize(400, 300);
        verticalLayout = new QVBoxLayout(LoadingDlg);
        verticalLayout->setObjectName("verticalLayout");
        verticalSpacer = new QSpacerItem(20, 16, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        widget_2 = new QWidget(LoadingDlg);
        widget_2->setObjectName("widget_2");
        horizontalLayout_2 = new QHBoxLayout(widget_2);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        status_lb = new QLabel(widget_2);
        status_lb->setObjectName("status_lb");
        status_lb->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_2->addWidget(status_lb);


        verticalLayout->addWidget(widget_2);

        widget = new QWidget(LoadingDlg);
        widget->setObjectName("widget");
        widget->setMinimumSize(QSize(50, 50));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setObjectName("horizontalLayout");
        loading_lb = new QLabel(widget);
        loading_lb->setObjectName("loading_lb");
        loading_lb->setMinimumSize(QSize(200, 200));
        loading_lb->setMaximumSize(QSize(200, 200));
        loading_lb->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout->addWidget(loading_lb);


        verticalLayout->addWidget(widget);

        verticalSpacer_2 = new QSpacerItem(20, 16, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);


        retranslateUi(LoadingDlg);

        QMetaObject::connectSlotsByName(LoadingDlg);
    } // setupUi

    void retranslateUi(QDialog *LoadingDlg)
    {
        LoadingDlg->setWindowTitle(QCoreApplication::translate("LoadingDlg", "Dialog", nullptr));
        status_lb->setText(QString());
        loading_lb->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class LoadingDlg: public Ui_LoadingDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOADINGDLG_H
