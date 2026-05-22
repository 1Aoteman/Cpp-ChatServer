#include "searchlist.h"
#include<QScrollBar>
#include "adduseritem.h"
#include "tcpmgr.h"
#include "customizeedit.h"
#include "findsuccessdlg.h"
#include "loadingdlg.h"
#include "userdata.h"
#include "usermgr.h"


SearchList::SearchList(QWidget *parent):QListWidget(parent),_find_dlg(nullptr), _search_edit(nullptr), _send_pending(false)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    //连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);
    //添加条目
    addTipItem();
    //连接搜索条目
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_user_search, this, &SearchList::slot_user_search);

}
void SearchList::SetSearchEdit(QWidget *edit)
{
    _search_edit =edit;
}
void SearchList::addTipItem()
{
    auto *invalid_item = new QWidget();
    QListWidgetItem *item_tmp = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item_tmp->setSizeHint(QSize(250,10));
    this->addItem(item_tmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(item_tmp, invalid_item);
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);


    auto *add_user_item = new AddUserItem();
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(add_user_item->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, add_user_item);
}
void SearchList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item); //获取自定义widget对象
    if(!widget){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }
    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }
    auto itemType = customItem->GetItemType();
    if(itemType == ListItemType::INVALID_ITEM){
        qDebug()<< "slot invalid item clicked ";
        return;
    }
    if(itemType == ListItemType::ADD_USER_TIP_ITEM){
        //，发送消息查询有延迟，判断是否还查询发送状态
        if(_send_pending){
            return;
        }
        if(!_search_edit){
            return;
        }
        waitPending(true);
        auto search_edit = dynamic_cast<CustomizeEdit*>(_search_edit);
        //取出搜索框中的查询条件
        auto uid_str =search_edit->text();
        QJsonObject jsonobj;
        jsonobj["uid"] =uid_str;
        QJsonDocument jsondoc(jsonobj);
        QByteArray jsondata =jsondoc.toJson(QJsonDocument::Compact);
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_SEARCH_USER_REQ,jsondata);
        qDebug()<<"发送了搜索请求";
        return;
    }
    //清楚弹出框
    CloseFindDlg();
}
void SearchList::CloseFindDlg()
{
    // 这里写关闭查找对话框的逻辑
    // 如果暂时不知道写什么，哪怕里面空着也行，但必须得有这个大括号
}



void SearchList::waitPending(bool pending)
{
    //如果还在查询，放上加载界面
    if(pending){
        _loadingDialog = new LoadingDlg(this);
        _loadingDialog->setModal(true);
        _loadingDialog->show();
        _send_pending = pending;
    }else{
        _loadingDialog->hide();
        _loadingDialog->deleteLater();
        _send_pending = pending;
    }
}
void SearchList::slot_user_search(std::shared_ptr<SearchInfo> info)
{
    //先bu等待
    waitPending(false);
    if(info==nullptr){
        _find_dlg = std::make_shared<FindSuccessDlg>(this);
    }
    else{
        //此处分两种情况，一种是搜多到已经是自己的朋友了，一种是未添加好友
        //查找是否已经是好友 todo.
        _find_dlg = std::make_shared<FindSuccessDlg>(this);
        std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg)->SetSearchInfo(info);
    }
    _find_dlg->show();
    return;
}
