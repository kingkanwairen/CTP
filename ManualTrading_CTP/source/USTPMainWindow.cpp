#include <QtCore/QByteArray>
#include "USTPConfig.h"
#include "USTPMainWindow.h"
#include "USTPMdDialog.h"
#include "USTPKeyDialog.h"
#include "USTPCtpLoader.h"
#include "USTPMutexId.h"
#include "USTPMdApi.h"
#include "USTPTradeApi.h"
#include "USTPProfile.h"


USTPMainWindow::USTPMainWindow(QWidget * parent)
: QMainWindow(parent)
{	
	mTabIndex = -1;
	mBrokerId = USTPCtpLoader::getBrokerId();
	mUserId = USTPMutexId::getUserId();
	mPsw = USTPMutexId::getLoginPsw();
	createPic();
	createActions();
	createMenus();
	createToolBar();
	createStatusBar();

	mTabIcon = QIcon("../image/tab.png");
	mRspKeyDown = new USTPRspKeyDown;
	mKeyDownApi = KeyDownHock::CreateKeyHock();
	mKeyDownApi->Init();
	mKeyDownApi->RegisterSpi(mRspKeyDown);
	mOrderWidget = new USTPOrderWidget(this);
	mTradeWidget = new USTPTradeWidget(this);
	mPositionWidget = new USTPositionWidget(this);
	mCancelWidget = new USTPCancelWidget(this);
	mMarketWidget = new USTPMarketWidget(this);
	
	QString group, order, key, actinNum;

	USTPProfile::readItem(tr("[ActionNum]"), actinNum);
	QStringList actionList = actinNum.split(",");
	for(int actionIndex = 0; actionIndex < actionList.size(); actionIndex++){
		QString itemAction = actionList.at(actionIndex);
		QStringList actionItem = itemAction.split(":");
		USTPMutexId::initActionNum(actionItem.at(0), actionItem.at(1).toInt());
	}
	USTPProfile::readItem(tr("[KeyItem]"), key);
	QStringList keyList = key.split("|");
	mBidKey = keyList.at(0).toInt();
	mAskKey = keyList.at(1).toInt();

	mSubmitWidget = new USTPSubmitWidget(mBidKey, mAskKey, mOrderWidget, mCancelWidget, mMarketWidget, this);
	QWidget* pWidget = new QWidget(this);
	QGridLayout* pLayOut = new QGridLayout;
	pLayOut->addWidget(mMarketWidget, 0, 0, 5, 1);
	pLayOut->addWidget(mSubmitWidget, 5, 0, 1, 1);	
	pWidget->setLayout(pLayOut);
	mMarketDockWidget = createDockWidget(CANCEL_DOCK_WIDGET, pWidget,Qt:: AllDockWidgetAreas, Qt::TopDockWidgetArea);
	mOrderDockWidget = createDockWidget(ORDER_DOCK_WIDGET, mOrderWidget,Qt:: AllDockWidgetAreas, Qt::RightDockWidgetArea);
	mPositionDockWidget = createDockWidget(POSITION_DOCK_WIDGET, mPositionWidget,Qt:: AllDockWidgetAreas, Qt::BottomDockWidgetArea);
	mTradeDockWidget = createDockWidget(TRADE_DOCK_WIDGET, mTradeWidget,Qt:: AllDockWidgetAreas, Qt::BottomDockWidgetArea);
	setCentralWidget(mCancelWidget);
	
	initConnect();
	setWindowIcon(QIcon("../image/title.png"));
	QString titleName = MAIN_WINDOW_TILTE + tr("     账号: ") + mUserId + tr("   席位: ") + mBrokerId;
	setWindowTitle(titleName);
	showMaximized(); 

	QDesktopWidget *pDesk = QApplication::desktop();
	resize(pDesk->width() / 2, pDesk->height());
	move(0, 0);
	doQryInvestorPosition();
}

USTPMainWindow::~USTPMainWindow()
{	
	QString actionList;
	QMap<QString, int> actionNms;
	if (USTPMutexId::getTotalActionNum(actionNms)){
		QMapIterator<QString, int> i(actionNms);
		int nIndex = 0;
		while (i.hasNext()){
			i.next();
			QString item = i.key() + tr(":") + QString::number(i.value());
			if (nIndex == 0)
				actionList = item;
			else{
				actionList += tr(",");
				actionList += item;
			}
			nIndex++;
		}
		USTPProfile::writeItem(tr("[ActionNum]"), actionList);
	}


	if(mOrderWidget){
		delete mOrderWidget;
		mOrderWidget = NULL;
	}

	if(mOrderDockWidget){
		delete mOrderDockWidget;
		mOrderDockWidget = NULL;
	}

	if(mTradeWidget){
		delete mTradeWidget;
		mTradeWidget = NULL;
	}

	if(mTradeDockWidget){
		delete mTradeDockWidget;
		mTradeDockWidget = NULL;
	}

	if(mPositionWidget){
		delete mPositionWidget;
		mPositionWidget = NULL;
	}

	if(mPositionDockWidget){
		delete mPositionDockWidget;
		mPositionDockWidget = NULL;
	}


	if(mCancelWidget){
		delete mCancelWidget;
		mCancelWidget = NULL;
	}

	if(mMarketWidget){
		delete mMarketWidget;
		mMarketWidget = NULL;
	}

	if(mSubmitWidget){
		delete mSubmitWidget;
		mSubmitWidget = NULL;
	}


	if(mMarketDockWidget){
		delete mMarketDockWidget;
		mMarketDockWidget = NULL;
	}

	if(mKeyDownApi){
		mKeyDownApi->Release();
		delete mKeyDownApi;
		mKeyDownApi = NULL;
	}
}

void USTPMainWindow::initConnect()
{	
	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPTradeFrontConnected()), this, SLOT(doUSTPTradeFrontConnected()));
	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPTradeFrontDisconnected(int)), this, SLOT(doUSTPTradeFrontDisconnected(int)));
	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPTradeRspUserLogin(const QString&, const QString&, const QString&, const int&, const int&, const int&, const int&, const QString&, bool)),
		this, SLOT(doUSTPTradeRspUserLogin(const QString&, const QString&, const QString&, const int&, const int&, const int&, const int&, const QString&, bool)));
	
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPMdFrontConnected()), this, SLOT(doUSTPMdFrontConnected()));
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPMdFrontDisconnected(int)), this, SLOT(doUSTPMdFrontDisconnected(int)));
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPMdRspUserLogin(const QString&, const QString&, const int&, const QString&, bool)),
		this, SLOT(doUSTPMdRspUserLogin(const QString&, const QString&, const int&, const QString&, bool)));
	
	connect(mRspKeyDown, SIGNAL(onKeyDownHock(const int&)), mSubmitWidget, SLOT(doKeyDownHock(const int&))); 
	connect(mRspKeyDown, SIGNAL(onKeyDownHock(const int&)), mCancelWidget, SLOT(doKeyDownHock(const int&))); 
	connect(mPositionWidget, SIGNAL(onUpdatePosition(const QString&, const int&, const int&)), mSubmitWidget,
		SLOT(doUpdatePosition(const QString&, const int&, const int&))); 
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
	mToolBar->addAction(mKeyAction);
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

	mKeyAction = new QAction(QIcon("../image/key.png"), QString(tr("快捷按键(&K)")), this);
	mKeyAction->setShortcut(QKeySequence(QObject::tr("Ctrl+K")));

	connect(mExitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
	connect(mHelpAction, SIGNAL(triggered()), this, SLOT(about()));
	connect(mMarketAction, SIGNAL(triggered()), this, SLOT(doSubscribeMarket()));
	connect(mKeyAction, SIGNAL(triggered()), this, SLOT(doNewKey()));	
}

void USTPMainWindow::createMenus()
{
	mFileMenu = menuBar()->addMenu(tr("文件(&F)"));
	mFileMenu->addAction(mExitAction);

	mOperMenu = menuBar()->addMenu(tr("操作(&0)"));
	menuBar()->addSeparator();

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

	mMdLinkLabel = new QLabel;
	mMdLinkLabel->setFixedSize(LINK_LABEL_WIDTH, LINK_LABEL_WIDTH);
	mMdLinkLabel->setPixmap(mLinkPic);

	bar->addWidget(mMessageLabel, width() - 2 * LINK_LABEL_WIDTH);
	bar->addWidget(mMdLinkLabel);
	bar->addWidget(mTradeLinkLabel);
}

QDockWidget* USTPMainWindow::createDockWidget(const QString& title, QWidget* pWidget, Qt::DockWidgetAreas allowAreas, Qt::DockWidgetArea initArea)
{
	QDockWidget *dock = new QDockWidget(title, this);
	dock->setAllowedAreas(allowAreas);
	dock->setWidget(pWidget);
	addDockWidget(initArea, dock);
	mOperMenu->addAction(dock->toggleViewAction());
	return dock;
}

void USTPMainWindow::about()
{
	QMessageBox::about(this, tr("关于 套利交易系统"),
		tr("<h4>期货K系统</h4>版权所有(C) King  2015-2016"));
}


void USTPMainWindow::doSubscribeMarket()
{	
	QString mdItem;
	USTPProfile::readItem(tr("[MdItem]"), mdItem);
	USTPMdDialog dlg(mMarketWidget, mSubmitWidget, mdItem, this);
	dlg.exec();
}


void  USTPMainWindow::doNewKey()
{
	USTPKeyDialog dlg(mBidKey, mAskKey, this);
	dlg.exec();
}

void USTPMainWindow::doUSTPTradeFrontConnected()
{
	USTPTradeApi::reqUserLogin(USTPMutexId::getMutexId(), mBrokerId, mUserId, mPsw);
}

void USTPMainWindow::doUSTPTradeFrontDisconnected(int reason)
{
	mTradeLinkLabel->setPixmap(mDisconnectPic);
}

void USTPMainWindow::doUSTPMdFrontConnected()
{
	USTPMdApi::reqUserLogin(mBrokerId, mUserId, mPsw);
}

void USTPMainWindow::doUSTPMdFrontDisconnected(int reason)
{
	mMdLinkLabel->setPixmap(mDisconnectPic);
}


void USTPMainWindow::doCreateNewKey(const int& bidKey, const int& askKey)
{	
	mBidKey = bidKey;
	mAskKey = askKey;
	emit onUpdateKey(bidKey, askKey);
}

void USTPMainWindow::doUSTPMdRspUserLogin(const QString& brokerId, const QString& userId, const int& errorId, const QString& errorMsg, bool bIsLast)
{	
	if (errorId >= 0){
		mMdLinkLabel->setPixmap(mLinkPic);
		QString mdItem;
		USTPProfile::readItem(tr("[MdItem]"), mdItem);
		QStringList insCombo = mdItem.split(",");
		
		mInss.clear();
		foreach(QString ins, insCombo){
			USTPMdApi::subMarketData(ins);
			mInss.insert(ins, ins);
		}
	}	
}

void USTPMainWindow::doUSTPTradeRspUserLogin(const QString& tradingDate, const QString& brokerId, const QString& userId, const int& maxLocalId, const int& frontId,
							 const int& sessionId, const int& errorId, const QString& errorMsg, bool bIsLast)
{	
	if (errorId == 0)
		mTradeLinkLabel->setPixmap(mLinkPic); //交易登录，显示连接成功信号
	
}

void USTPMainWindow::doQryInvestorPosition()
{	
	int nResult = USTPTradeApi::reqQryTrade(USTPMutexId::getMutexId(), mBrokerId, mUserId, "");
}

#include "moc_USTPMainWindow.cpp"