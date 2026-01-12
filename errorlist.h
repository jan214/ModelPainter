#ifndef ERRORLIST_H
#define ERRORLIST_H

#include <QListWidget>
#include <QStyledItemDelegate>

class ErrorList : public QListWidget{
public:
    enum MessageType{
        None,
        Warning,
        Error
    };

    class ErrorListWidgetItemDelegate : public QStyledItemDelegate{
    public:
        explicit ErrorListWidgetItemDelegate(QWidget* const parent);
        ~ErrorListWidgetItemDelegate(){}
    protected:
        virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    };

    explicit ErrorList(QWidget* parent = nullptr);
    ~ErrorList(){}

    void AddError(QString errorText, MessageType type);

protected:
};

#endif // ERRORLIST_H
