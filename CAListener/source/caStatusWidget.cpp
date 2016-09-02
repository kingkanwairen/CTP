#include "caStatusWidget.h"
#include "caPublicDefine.h"
#include <QtGui/QtGui>
#define  STATUS_BUFFER 1024

caStatusWidget::caStatusWidget(QWidget * parent, Qt::WindowFlags flags)
: QWidget(parent, flags)
{	
	mCaiApi = NULL;
	mImageLabel = new QLabel;
	QImage image(STATUS_LABEL_WIDTH, STATUS_LABEL_HEIGHT, QImage::Format_RGB32);
	image.fill(Qt::gray);
	mPixmap = QPixmap::fromImage(image);
	mImageLabel->setPixmap(mPixmap);
	mUpdatehBtn = createButton(tr("Ë¢ÐÂ"), SLOT(updateStatus()));

	mViewLayout = new QGridLayout;
	mViewLayout->addWidget(mUpdatehBtn, 0, 2, 1, 1);
	mViewLayout->addWidget(mImageLabel, 1, 0, 5, 5);
	setLayout(mViewLayout);
	setMaximumWidth(STATUS_WINDOW_MAX_WIDTH);
}

caStatusWidget::~caStatusWidget()
{
	if(mImageLabel != NULL){
		delete mImageLabel;
		mImageLabel = NULL;
	}

	if(mUpdatehBtn != NULL){
		delete mUpdatehBtn;
		mUpdatehBtn = NULL;
	}

	if(mViewLayout != NULL){
		delete mViewLayout;
		mViewLayout = NULL;
	}

}

void caStatusWidget::updateStatus()
{	
	if(mCaiApi != NULL){
		char* buffer = new char[STATUS_BUFFER];
		memset(buffer, 0, STATUS_BUFFER);
		mCaiApi->GetEngineStatus(buffer, STATUS_BUFFER);
		QPixmap pixmap(mPixmap);
		QPainter painter(&pixmap);
		QFont font;
		font.setPixelSize(13);
		font.setBold(true);
		painter.setFont(font);
		painter.setPen(QColor(255, 0, 0));
		painter.drawText(QRect(0, 0, STATUS_LABEL_WIDTH, STATUS_LABEL_HEIGHT), Qt::AlignCenter, QString(buffer));
		mImageLabel->setPixmap(pixmap);
		delete buffer;
		buffer = NULL;
	}
	
}

void caStatusWidget::doSetCaiApi(CaiApi*  pCaiApi)
{
	mCaiApi = pCaiApi;
	updateStatus();
}

QPushButton *caStatusWidget::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}

void caStatusWidget::doDisconnectEngine()
{
	updateStatus();
}

#include "moc_caStatusWidget.cpp"