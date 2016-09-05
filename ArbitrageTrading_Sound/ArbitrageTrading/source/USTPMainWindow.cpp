#include <QtCore/QByteArray>
#include "USTPConfig.h"
#include "USTPMainWindow.h"
#include "USTPCtpLoader.h"
#include "USTPMutexId.h"
#include "USTPTradeApi.h"
#include "USTPProfile.h"


USTPMainWindow::USTPMainWindow(QWidget * parent)
: QMainWindow(parent)
{	
	mBrokerId = USTPCtpLoader::getBrokerId();
	mUserId = USTPMutexId::getUserId();
	mPsw = USTPMutexId::getLoginPsw();
	createPic();
	createActions();
	createMenus();
	createStatusBar();

	mTradeWidget = new USTPTradeWidget(this);
	setCentralWidget(mTradeWidget);
	initConnect();
	setWindowIcon(QIcon("../image/title.png"));
	QString titleName = MAIN_WINDOW_TILTE + tr("           用户账号: ") + mUserId + tr("           会员席位: ") + mBrokerId;
	setWindowTitle(titleName);
	QDesktopWidget *pDesk = QApplication::desktop();
	resize(200, 150); 
	move((pDesk->width() - width()) / 2, (pDesk->height() - height()) / 2);
}

USTPMainWindow::~USTPMainWindow()
{	
	if(mTradeWidget){
		delete mTradeWidget;
		mTradeWidget = NULL;
	}
}

void USTPMainWindow::initConnect()
{	
	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPTradeFrontConnected()), this, SLOT(doUSTPTradeFrontConnected()));
	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPTradeFrontDisconnected(int)), this, SLOT(doUSTPTradeFrontDisconnected(int)));
	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPTradeRspUserLogin(const QString&, const QString&, const QString&, const int&, const int&, const int&, const int&, const QString&, bool)),
		this, SLOT(doUSTPTradeRspUserLogin(const QString&, const QString&, const QString&, const int&, const int&, const int&, const int&, const QString&, bool)));
}

void USTPMainWindow::createPic()
{	
	QImage image(LINK_LABEL_WIDTH, LINK_LABEL_HEIGHT, QImage::Format_RGB32);
	image.load(QString("../image/red.png"));
	QImage scaleImage;
	if(image.width() > LINK_LABEL_WIDTH  || image.height() > LINK_LABEL_HEIGHT){
		scaleImage = image.scaled(QSize(LINK_LABEL_WIDTH, LINK_LABEL_HEIGHT));
	}
	mDisconnectPic = QPixmap::fromImage(scaleImage);
	
	image.load(QString("../image/green.png"));
	if(image.width() > LINK_LABEL_WIDTH  || image.height() > LINK_LABEL_HEIGHT){
		scaleImage = image.scaled(QSize(LINK_LABEL_WIDTH, LINK_LABEL_HEIGHT));
	}
	mLinkPic = QPixmap::fromImage(scaleImage);
}

void USTPMainWindow::createActions()
{
	mHelpAction = new QAction(QString(tr("关于(&A)")), this);
	mHelpAction->setShortcut(QKeySequence(QObject::tr("Ctrl+A")));
	
	mExitAction = new QAction(QIcon("../image/exit.png"), QString(tr("退出(&E)")), this);
	mExitAction->setShortcuts(QKeySequence::Quit);

	connect(mExitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
	connect(mHelpAction, SIGNAL(triggered()), this, SLOT(about()));
}

void USTPMainWindow::createMenus()
{
	mFileMenu = menuBar()->addMenu(tr("文件(&F)"));
	mFileMenu->addAction(mExitAction);

	mHelpMenu = menuBar()->addMenu(tr("帮助(&H)"));
	mHelpMenu->addAction(mHelpAction);
}

void USTPMainWindow::createStatusBar()
{
	QStatusBar* bar = statusBar();
	mMessageLabel = new QLabel;
	mTradeLinkLabel = new QLabel;
	mTradeLinkLabel->setFixedSize(LINK_LABEL_WIDTH, LINK_LABEL_WIDTH);
	mTradeLinkLabel->setPixmap(mLinkPic);

	bar->addWidget(mMessageLabel, width() - LINK_LABEL_WIDTH);
	bar->addWidget(mTradeLinkLabel);
}


void USTPMainWindow::about()
{
	QMessageBox::about(this, tr("关于 成交回报监听系统"),
		tr("<b>成交回报监听系统</b> 为一款基于CTP柜台的成交回报监听系统。"));
}

void USTPMainWindow::doUSTPTradeFrontConnected()
{
	USTPTradeApi::reqUserLogin(USTPMutexId::getMutexId(), mBrokerId, mUserId, mPsw);
}

void USTPMainWindow::doUSTPTradeFrontDisconnected(int reason)
{
	mTradeLinkLabel->setPixmap(mDisconnectPic);
}

void USTPMainWindow::doUSTPTradeRspUserLogin(const QString& tradingDate, const QString& brokerId, const QString& userId, const int& maxLocalId, const int& frontId,
							 const int& sessionId, const int& errorId, const QString& errorMsg, bool bIsLast)
{	
	if (errorId == 0)
		mTradeLinkLabel->setPixmap(mLinkPic); //交易登录，显示连接成功信号
	
}

#include "moc_USTPMainWindow.cpp"