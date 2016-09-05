#include <QtCore/QByteArray>
#include "USTPConfig.h"
#include "USTPMainWindow.h"
#include "USTPMdDialog.h"
#include "USTPCtpLoader.h"
#include "USTPMutexId.h"
#include "USTPMdApi.h"
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
	createToolBar();
	createStatusBar();
	mMarketWidget = new USTPMarketWidget(this);
	setCentralWidget(mMarketWidget);
	initConnect();
	setWindowIcon(QIcon("../image/title.png"));
	setWindowTitle(MAIN_WINDOW_TILTE);
	showMaximized(); 
}

USTPMainWindow::~USTPMainWindow()
{	
	if(mMarketWidget){
		delete mMarketWidget;
		mMarketWidget = NULL;
	}
}

void USTPMainWindow::initConnect()
{	
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPMdFrontConnected()), this, SLOT(doUSTPMdFrontConnected()));
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPMdFrontDisconnected(int)), this, SLOT(doUSTPMdFrontDisconnected(int)));
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

void USTPMainWindow::createToolBar()
{	
	mToolBar = new QToolBar(this);
	mToolBar->setAllowedAreas(Qt::TopToolBarArea);
	mToolBar->setOrientation(Qt::Horizontal);
	mToolBar->setMovable(false);
	mToolBar->addAction(mMarketAction);
	addToolBar(Qt::TopToolBarArea, mToolBar);
}	

void USTPMainWindow::createActions()
{
	mHelpAction = new QAction(QString(tr("关于(&A)")), this);
	mHelpAction->setShortcut(QKeySequence(QObject::tr("Ctrl+A")));
	
	mExitAction = new QAction(QIcon("../image/exit.png"), QString(tr("退出(&E)")), this);
	mExitAction->setShortcuts(QKeySequence::Quit);

	mMarketAction = new QAction(QIcon("../image/md.png"), QString(tr("注册行情(&N)")), this);
	mMarketAction->setShortcut(QKeySequence(QObject::tr("Ctrl+N")));

	connect(mExitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
	connect(mHelpAction, SIGNAL(triggered()), this, SLOT(about()));
	connect(mMarketAction, SIGNAL(triggered()), this, SLOT(doSubscribeMarket()));
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
	
	mMdLinkLabel = new QLabel;
	mMdLinkLabel->setFixedSize(LINK_LABEL_WIDTH, LINK_LABEL_WIDTH);
	mMdLinkLabel->setPixmap(mLinkPic);

	bar->addWidget(mMessageLabel, width() - LINK_LABEL_WIDTH);
	bar->addWidget(mMdLinkLabel);
}


void USTPMainWindow::about()
{
	QMessageBox::about(this, tr("关于 组合行情系统"),
		tr("<b>组合行情系统</b> 为一款基于飞马柜台的快速组合行情显示系统。"));
}


void USTPMainWindow::doSubscribeMarket()
{	
	QString mdItem;
	USTPProfile::readItem(tr("[MdItem]"), mdItem);
	USTPMdDialog dlg(mMarketWidget, mdItem, this);
	dlg.exec();
}

void USTPMainWindow::doUSTPMdFrontConnected()
{
	mMdLinkLabel->setPixmap(mLinkPic);
	USTPMdApi::reqUserLogin(mBrokerId, mUserId, mPsw);
}

void USTPMainWindow::doUSTPMdFrontDisconnected(int reason)
{
	mMdLinkLabel->setPixmap(mDisconnectPic);
}

#include "moc_USTPMainWindow.cpp"