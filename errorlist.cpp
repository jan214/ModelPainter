#include "errorlist.h"

#include <QApplication>
#include <QPainter>

ErrorList::ErrorList(QWidget* parent) :
QListWidget(parent)
{
    viewport()->setAutoFillBackground(false);
    ErrorListWidgetItemDelegate* const errorListWidgetItemDelegate = new ErrorListWidgetItemDelegate(this);
    setItemDelegate(errorListWidgetItemDelegate);

    setMaximumHeight(60);
}

void ErrorList::AddError(QString errorText, MessageType type){
    QListWidgetItem* item = new QListWidgetItem(this);
    item->setText(errorText);
    QIcon messageIcon;
    QStyle* style = QApplication::style();
    if(type == MessageType::Warning){
        messageIcon = style->standardIcon(QStyle::SP_MessageBoxWarning);
    }else if(type == MessageType::Error){
        messageIcon = style->standardIcon(QStyle::SP_MessageBoxCritical);
    }else{
        messageIcon = style->standardIcon(QStyle::SP_MessageBoxInformation);
    }
    item->setData(Qt::UserRole + 1, QVariant((uint)type));
    item->setIcon(messageIcon);
    addItem(item);
}

ErrorList::ErrorListWidgetItemDelegate::ErrorListWidgetItemDelegate(QWidget* const parent) :
QStyledItemDelegate(parent){
}

void ErrorList::ErrorListWidgetItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const{
    painter->save();

    MessageType itemMessageType = (MessageType)index.data(Qt::UserRole + 1).toUInt();
    // try to cache the gradient
    QLinearGradient linearGradient(option.rect.topLeft(), option.rect.bottomLeft());
    if(itemMessageType == MessageType::Error){
        painter->setPen(Qt::red);
        linearGradient.setColorAt(0.0, QColor(255,0,0,100));
        linearGradient.setColorAt(1.0, QColor(255,0,0,200));
    }else if(itemMessageType == MessageType::Warning){
        painter->setPen(Qt::yellow);
        linearGradient.setColorAt(0.0, QColor(255,255,0,100));
        linearGradient.setColorAt(1.0, QColor(255,255,0,200));
    }else{
        painter->setPen(Qt::white);
        linearGradient.setColorAt(0.0, QColor(255,255,255,100));
        linearGradient.setColorAt(1.0, QColor(255,255,255,200));
    }
    painter->setBrush(linearGradient);
    painter->drawRoundedRect(option.rect.marginsRemoved(QMargins(1,1,1,1)), 2.0, 2.0);
    painter->restore();

    QStyledItemDelegate::paint(painter, option, index);
}
