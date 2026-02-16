#ifndef CONTACTDIALOG_H
#define CONTACTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

class ContactDialog : public QDialog {
public:
    explicit ContactDialog(QWidget* parent = nullptr);
	~ContactDialog(){}
protected:
	class ContactItem : public QWidget {
	public:
        explicit ContactItem(QImage image, QString text, QWidget* parent = nullptr, const bool isLink = false, const bool isCopyable = false);
		~ContactItem(){}

		void AddWidget(QWidget* widget);

	private:
		QHBoxLayout mainLayout;

		QLabel icon;
		QLabel label;
	};

	void paintEvent(QPaintEvent* paintEvent) override;

	QVBoxLayout mainLayout;
	QPushButton closeButton;
};

#endif // CONTACTDIALOG_H
