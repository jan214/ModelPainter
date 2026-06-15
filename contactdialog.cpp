#include "contactdialog.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

ContactDialog::ContactItem::ContactItem(QImage image, QString text, QWidget* parent, const bool isLink, const bool isCopyable) :
QWidget(parent),
mainLayout(this),
icon(this),
label(text, this){
	setObjectName("ContactItem");
	setStyleSheet("#ContactItem{ padding: 2px; color: rgba(33,33,33,255); padding: 2px; border: none; }"
					"#ContactItem #Label{ padding: 2px; }"
					"#ContactItem::hover{ background-color: rgba(56,56,56,125); border-radius: 4px; }");

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

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

ContactDialog::ContactDialog(QWidget* parent) :
QDialog(parent),
mainLayout(this),
closeButton("Close", this){
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	mainLayout.setSizeConstraint(QLayout::SetFixedSize);
	const QRect screenRect = QGuiApplication::primaryScreen()->availableGeometry();
    const QPoint spawnPosition = QPoint(screenRect.width() / 2, screenRect.height() / 2);
	move(spawnPosition);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    mainLayout.setSizeConstraint(QLayout::SetMinimumSize);
	mainLayout.setSpacing(0);

	QPixmap testPixmap = QIcon::fromTheme("edit-copy").pixmap(QSize(16, 16));

    QImage contactIcon(":/icons/contact.png");
	if (contactIcon.isNull()) {
		printf("contactIcon is null\n");
	}
    QImage aboutMeIcon(":/icons/aboutme.png");
	if (aboutMeIcon.isNull()) {
		printf("aboutMeIcon is null\n");
	}
    QImage homeIcon(":/icons/home.png");
	if (homeIcon.isNull()) {
		printf("homeIcon is null\n");
	}

	QImage qRCode(":/icons/QRCode.png");
	if (qRCode.isNull()) {
		printf("qRCode is null\n");
	}

    const int iconSize = 16;

    ContactItem* nameItem = new ContactItem(aboutMeIcon.scaled(iconSize,iconSize,Qt::KeepAspectRatio,Qt::SmoothTransformation), "Jan Moritz", this);
	mainLayout.addWidget(nameItem);

    ContactItem* websiteItem = new ContactItem(homeIcon.scaled(iconSize,iconSize,Qt::KeepAspectRatio,Qt::SmoothTransformation), "<a href='https://jan214.github.io'>jan214.github.io</a>", this, /*isLink*/ true, /*isCopyable*/ false);
	QLabel* qRCodeWrapperLabel = new QLabel(websiteItem);
	const int qRCodeSize = 64;
	qRCodeWrapperLabel->setFixedSize(QSize(qRCodeSize, qRCodeSize));
	qRCodeWrapperLabel->setPixmap(QPixmap::fromImage(qRCode.scaled(qRCodeSize,qRCodeSize,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
	qRCodeWrapperLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//qRCodeWrapperLabel->setAlignment(Qt::AlignCenter);
	qRCodeWrapperLabel->setVisible(true);
	websiteItem->AddWidget(qRCodeWrapperLabel);
	mainLayout.addWidget(websiteItem);

	const QString emailAddress("janmoritz@hotmail.de");
    ContactItem* emailAddressItem = new ContactItem(contactIcon.scaled(iconSize,iconSize,Qt::KeepAspectRatio,Qt::SmoothTransformation), emailAddress, this, /*isLink*/ false, /*isCopyable*/ true);
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

    QObject::connect(&closeButton, &QPushButton::clicked, this, [this]() { close(); });
	
	mainLayout.addWidget(&closeButton);
}

void ContactDialog::paintEvent(QPaintEvent* paintEvent) {
	QDialog::paintEvent(paintEvent);

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
