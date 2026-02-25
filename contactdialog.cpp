#include "contactdialog.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QPainter>
#include <QPainterPath>

ContactDialog::ContactItem::ContactItem(QImage image, QString text, const bool isLink, const bool isCopyable, QWidget* parent) :
QWidget(parent),
mainLayout(this),
icon(this),
label(text, this){
	setObjectName("ContactItem");
	setStyleSheet("#ContactItem{ padding: 2px; color: rgba(33,33,33,255); padding: 2px; border: none; }"
					"#ContactItem #Label{ padding: 2px; }"
					"#ContactItem::hover{ background-color: rgba(56,56,56,125); border-radius: 4px; }");
	mainLayout.setSpacing(0);
	icon.setPixmap(QPixmap::fromImage(image));
	icon.setFixedSize(QSize(16, 16));
	icon.setAlignment(Qt::AlignCenter);
	icon.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	mainLayout.addWidget(&icon);
	label.setObjectName("Label");
	label.setFixedHeight(27);
	label.setOpenExternalLinks(isLink);
	label.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	if (isCopyable) {
		label.setTextInteractionFlags(Qt::TextSelectableByMouse);
	}
	label.setAlignment(Qt::AlignVCenter & Qt::AlignLeft);
	mainLayout.addWidget(&label);
}

void ContactDialog::ContactItem::AddWidget(QWidget* widget) {
	mainLayout.addWidget(widget);
}

ContactDialog::ContactDialog(QWidget* const parent) :
QDialog(parent),
mainLayout(this),
closeButton("Close", this){
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);

	if (parent != nullptr) {
		setFixedWidth(parent->width() / 3);
	}

	mainLayout.setSpacing(0);

	QPixmap testPixmap = QIcon::fromTheme("edit-copy").pixmap(QSize(16, 16));

	ContactItem* nameItem = new ContactItem(testPixmap.toImage(), "Jan Moritz", this);
	mainLayout.addWidget(nameItem);

	ContactItem* websiteItem = new ContactItem(testPixmap.toImage(), "<a href='https://jan214.github.io'>jan214.github.io</a>", /*isLink*/ true, /*isCopyable*/ false, this);
	mainLayout.addWidget(websiteItem);

	const QString emailAddress("janmoritz@hotmail.de");
	ContactItem* emailAddressItem = new ContactItem(testPixmap.toImage(), emailAddress, /*isLink*/ false, /*isCopyable*/ true, this);
	QPushButton* copyButton = new QPushButton(QIcon::fromTheme("edit-copy"), "", emailAddressItem);
	copyButton->setObjectName("CopyButton");
	copyButton->setFixedSize(QSize(websiteItem->height(), websiteItem->height()));
	copyButton->setStyleSheet("#CopyButton{ background-color: transparent; border-radius: 4px; } #CopyButton::hover{ background-color: rgba(221,221,221,125); border: 1px solid rgba(245,245,245,255); }");
	copyButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QObject::connect(copyButton, &QPushButton::clicked, [emailAddress]() { QGuiApplication::clipboard()->setText(emailAddress); });
	emailAddressItem->AddWidget(copyButton);
	mainLayout.addWidget(emailAddressItem);

	mainLayout.addSpacing(27);
	closeButton.setObjectName("CloseButton");
	closeButton.setStyleSheet("#CloseButton{ color: rgba(51,51,51,255); background-color: rgba(221,221,221,125); border-radius: 4px; border: 1px solid rgba(245,245,245,255); } #CloseButton::hover{ color: rgba(245,245,245,255); background-color: rgba(56,56,56,125); border: 1px solid rgba(51,51,51,255); }");
	closeButton.setFixedHeight(27);

	QObject::connect(&closeButton, &QPushButton::clicked, [this]() { close(); });
	
	mainLayout.addWidget(&closeButton);
}

void ContactDialog::paintEvent(QPaintEvent* paintEvent) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	QPen framePen(QColor(221, 221, 221, 255));
	framePen.setWidth(2);

	painter.setPen(framePen);

	QBrush backgroundBrush(QColor(245, 245, 245, 200));
	painter.setBrush(backgroundBrush);

	const QRect drawingRect = this->rect().adjusted(1, 1, -1, -1);

	painter.drawRoundedRect(drawingRect, 12, 12);
}