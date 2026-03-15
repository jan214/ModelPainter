#ifndef CONTACTDIALOG_H
#define CONTACTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

class ContactDialog : public QDialog {
public:
	static ContactDialog& GetInstance() {
		printf("GetInstance\n");
		static ContactDialog instance;
		return instance;
	}
protected:
	class ContactItem : public QWidget {
	public:
		explicit ContactItem(QImage image, QString text, QWidget* parent = nullptr, const bool isLink = false, const bool isCopyable = false);
		~ContactItem(){}

		void AddWidget(QWidget* widget);

	protected:
		QHBoxLayout mainLayout;

		QLabel icon;
		QLabel label;
	};

	explicit ContactDialog(QWidget* const parent = nullptr);
	~ContactDialog() {}

	void paintEvent(QPaintEvent* paintEvent) override;

	QVBoxLayout mainLayout;
	QPushButton closeButton;
};

#endif // CONTACTDIALOG_H
