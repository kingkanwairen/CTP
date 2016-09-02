#include <QtCore/QByteArray>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtCore/QTextStream>
#include "caMainWindow.h"
#include "caMonitorDialog.h"
#include "caEventDialog.h"
#include "caDeleteDialog.h"
#include "caLogManager.h"
#include "caPublicDefine.h"

caMainWindow::caMainWindow(QWidget * parent)
: QMainWindow(parent)
{	
	mCaiApi = NULL;
	mIsLink = false;
	createPic();
	createActions();
	createMenus();
	createToolBar();
	createStatusBar();

	if(readConfig(mIp, mPort, mPath, mStatus)){
		mParamDialog = new caConfigDialog(mIp, mPort, mPath, mStatus);
	}else{
		mParamDialog = new caConfigDialog();
	}
	mEngineRsp = new caEngineRsp;
	mLogWidget = new caLogWidget(this);
	mStatusWidget = new caStatusWidget();
	mOrderWidget = new caOrderWidget();
	mTradeWidget = new caTradeWidget();
	mPositionWidget = new caPositionWidget();

	mOrderDockWidget = createDockWidget(ORDER_DOCK_WIDGET, mOrderWidget);
	mTradeDockWidget = createDockWidget(TRADE_DOCK_WIDGET, mTradeWidget);
	mPositionDockWidget = createDockWidget(POSITION_DOCK_WIDGET, mPositionWidget);
	mStatusDockWidget = createDockWidget(STATUS_DOCK_WIDGET, mStatusWidget,Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea, Qt::RightDockWidgetArea);

	connect(mParamDialog, SIGNAL(onSetCofig(const QString&, const int&)), this, SLOT(doSetCofig(const QString&, const int&)));
	connect(mEngineRsp, SIGNAL(onUpdateLog(const QStringList&)), mLogWidget, SLOT(doUpdateLog(const QStringList&)));
	connect(mEngineRsp, SIGNAL(onUpdateOrder(const QStringList&)), mOrderWidget, SLOT(doUpdateOrder(const QStringList&)));
	connect(mEngineRsp, SIGNAL(onUpdateTrade(const QStringList&)), mTradeWidget, SLOT(doUpdateTrade(const QStringList&)));
	connect(mEngineRsp, SIGNAL(onUpdatePosition(const QStringList&)), mPositionWidget, SLOT(doUpdatePosition(const QStringList&)));
	connect(mEngineRsp, SIGNAL(onDisconnetEngine(const QString&)), mLogWidget, SLOT(doDisconnetEngine(const QString&)));
	connect(mEngineRsp, SIGNAL(onDisconnetEngine(const QString&)), this, SLOT(doDisconnetEngine(const QString&)));
	connect(this, SIGNAL(onSetCaiApi(CaiApi* )), mStatusWidget, SLOT(doSetCaiApi(CaiApi* )));
	connect(this, SIGNAL(onDisconnectEngine()), mStatusWidget, SLOT(doDisconnectEngine()));
	mTabWidget = new QTabWidget(this);
	mTabWidget->addTab(mLogWidget, MAIN_WINDOW_TAB_TITLE);
	setCentralWidget(mTabWidget);
	setWindowIcon(QIcon("../rc/title.png"));
	setWindowTitle(MAIN_WINDOW_TILTE);
	resize(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
}

caMainWindow::~caMainWindow()
{	
	if (mSetAction){
		delete mSetAction;
		mSetAction = NULL;
	}

	if (mHelpAction){
		delete mHelpAction;
		mHelpAction = NULL;
	}

	if (mExitAction){
		delete mExitAction;
		mExitAction = NULL;
	}

	if (mLinkAction){
		delete mLinkAction;
		mLinkAction = NULL;
	}

	if (mInsertAction){
		delete mInsertAction;
		mInsertAction = NULL;
	}

	if (mSendAction){
		delete mSendAction;
		mSendAction = NULL;
	}

	if (mDeleteAction){
		delete mDeleteAction;
		mDeleteAction = NULL;
	}

	if (mDisconnetAction){
		delete mDisconnetAction;
		mDisconnetAction = NULL;
	}

	if (mToolBar){
		delete mToolBar;
		mToolBar = NULL;
	}

	if (mEngineRsp){
		delete mEngineRsp;
		mEngineRsp = NULL;
	}

	if(mStatusWidget){
		delete mStatusWidget;
		mStatusWidget = NULL;
	}

	if (mStatusDockWidget){
		delete mStatusDockWidget;
		mStatusDockWidget = NULL;
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
	
	if (mLogWidget){	
		delete mLogWidget;
		mLogWidget = NULL;
	}

	if (mTabWidget){
		delete mTabWidget;
		mTabWidget = NULL;
	}

	if (mParamDialog){
		delete mParamDialog;
		mParamDialog = NULL;
	}
}

bool caMainWindow::readConfig(QString& ip, int& port, QString& path, int& status)
{
	//��ȡ�����ļ�
	int iPort = 0;
	QString strTemp;
	QFile file("../config/ca.ini");  
	if (!file.open(QFile::ReadOnly)) { 
		return false;
	}
	QTextStream stream(&file);
	QString strPort;
	strTemp = stream.readLine();
	int iLabelPos = strTemp.indexOf(":");
	if (iLabelPos == -1){
		return false;
	}
	if (strTemp.mid(0, iLabelPos) != QString(tr("Link"))){	
		return false;
	}
	QString strLink = strTemp.mid(iLabelPos + 1);
	QStringList linkParam = strLink.split(",");
	
	strTemp = stream.readLine();
	iLabelPos = strTemp.indexOf(":");
	if (iLabelPos == -1){
		return false;
	}
	if (strTemp.mid(0, iLabelPos) != QString(tr("Log"))){	
		return false;
	}
	QString strPath = strTemp.mid(iLabelPos + 1);
	QStringList pathParam = strPath.split(",");
	status = pathParam[0].toInt();
	port = linkParam[1].toInt();
	path = pathParam[1];
	ip = linkParam[0];
	file.close();
	return true;
}

void caMainWindow::createPic()
{	
	QImage image(LINK_LABEL_WIDTH, LINK_LABEL_HEIGHT, QImage::Format_RGB32);
	image.load(QString("../rc/red.png"));
	QImage scaleImage;
	if(image.width() > LINK_LABEL_WIDTH  || image.height() > LINK_LABEL_HEIGHT){
		scaleImage = image.scaled(QSize(LINK_LABEL_WIDTH, LINK_LABEL_HEIGHT));
	}
	mDisconnectPic = QPixmap::fromImage(scaleImage);
	
	image.load(QString("../rc/green.png"));
	if(image.width() > LINK_LABEL_WIDTH  || image.height() > LINK_LABEL_HEIGHT){
		scaleImage = image.scaled(QSize(LINK_LABEL_WIDTH, LINK_LABEL_HEIGHT));
	}
	mLinkPic = QPixmap::fromImage(scaleImage);
}

void caMainWindow::createToolBar()
{	
	mToolBar = new QToolBar(this);
	mToolBar->setAllowedAreas(Qt::TopToolBarArea);
	mToolBar->setOrientation(Qt::Horizontal);
	mToolBar->setMovable(false);
	mToolBar->addAction(mLinkAction);
	mToolBar->addAction(mInsertAction);
	mToolBar->addAction(mSendAction);
	mToolBar->addAction(mDeleteAction);
	mToolBar->addAction(mDisconnetAction);
	addToolBar(Qt::TopToolBarArea, mToolBar);
}	

void caMainWindow::createActions()
{
	mSetAction = new QAction(QIcon("../rc/set.gif"), QString(tr("����(&C)")), this);
	mSetAction->setShortcut(QKeySequence(QObject::tr("Ctrl+C")));
	//mSetAction->setStatusTip(tr("����״̬��������ز���"));
	
	mHelpAction = new QAction(QIcon("../rc/help.gif"), QString(tr("����(&A)")), this);
	mHelpAction->setShortcut(QKeySequence(QObject::tr("Ctrl+A")));
	//mHelpAction->setStatusTip(tr("��������Ĳ���"));
	
	mExitAction = new QAction(QIcon("../rc/exit.png"), QString(tr("�˳�(&E)")), this);
	mExitAction->setShortcuts(QKeySequence::Quit);
	//mExitAction->setStatusTip(tr("�˳�Ӧ�ó���"));

	mLinkAction = new QAction(QIcon("../rc/link.png"), QString(tr("����(&L)")), this);
	mLinkAction->setShortcut(QKeySequence(QObject::tr("Ctrl+L")));
	//mLinkAction->setStatusTip(tr("����Engine"));
	mInsertAction = new QAction(QIcon("../rc/monitor.png"), QString(tr("ע��Monitor(&I)")), this);
	mInsertAction->setShortcut(QKeySequence(QObject::tr("Ctrl+I")));
	//mInsertAction->setStatusTip(tr("ע��Monitorʵ��"));

	mSendAction = new QAction(QIcon("../rc/event.png"), QString(tr("����Event(&S)")), this);
	mSendAction->setShortcut(QKeySequence(QObject::tr("Ctrl+S")));
//	mSendAction->setStatusTip(tr("����event"));

	mDeleteAction = new QAction(QIcon("../rc/delete.png"), QString(tr("ɾ��(&D)")), this);
	mDeleteAction->setShortcut(QKeySequence(QObject::tr("Ctrl+D")));
	//mDeleteAction->setStatusTip(tr("ɾ�� monitor ���� event"));

	mDisconnetAction = new QAction(QIcon("../rc/diconnect.png"), QString(tr("�Ͽ�(&N)")), this);
	mDisconnetAction->setShortcut(QKeySequence(QObject::tr("Ctrl+N")));
	//mDisconnetAction->setStatusTip(tr("�Ͽ��� Engine ����"));

	connect(mExitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
	connect(mHelpAction, SIGNAL(triggered()), this, SLOT(about()));
	connect(mSetAction, SIGNAL(triggered()), this, SLOT(setParam()));
	connect(mLinkAction, SIGNAL(triggered()), this, SLOT(linkEngine()));
	connect(mDeleteAction, SIGNAL(triggered()), this, SLOT(deleteMonitor()));
	connect(mInsertAction, SIGNAL(triggered()), this, SLOT(insertMonitor()));
	connect(mSendAction, SIGNAL(triggered()), this, SLOT(sendEvent()));
	connect(mDisconnetAction, SIGNAL(triggered()), this, SLOT(disconnectEngine()));	
}

void caMainWindow::createMenus()
{
	mFileMenu = menuBar()->addMenu(tr("�ļ�(&F)"));
	mFileMenu->addAction(mExitAction);

	mOperMenu = menuBar()->addMenu(tr("����(&0)"));
	mOperMenu->addAction(mSetAction);
	menuBar()->addSeparator();

	mHelpMenu = menuBar()->addMenu(tr("����(&H)"));
	mHelpMenu->addAction(mHelpAction);
}

void caMainWindow::createStatusBar()
{
	QStatusBar* bar = statusBar();
	mMessageLabel = new QLabel;
	mLinkLabel = new QLabel;
	mLinkLabel->setFixedSize(LINK_LABEL_WIDTH, LINK_LABEL_WIDTH);
	mLinkLabel->setPixmap(mDisconnectPic);
	bar->addWidget(mMessageLabel, width() - LINK_LABEL_WIDTH);
	bar->addWidget(mLinkLabel);
}

QDockWidget* caMainWindow::createDockWidget(const QString& title, QWidget* pWidget, Qt::DockWidgetAreas allowAreas, Qt::DockWidgetArea initArea)
{
	QDockWidget *dock = new QDockWidget(title, this);
	dock->setAllowedAreas(allowAreas);
	dock->setWidget(pWidget);
	addDockWidget(initArea, dock);
	mOperMenu->addAction(dock->toggleViewAction());
	return dock;
}

void caMainWindow::about()
{
	QMessageBox::about(this, tr("���� �ϻ��ڻ�APAMA�ն˳���"),
		tr("<b>�ϻ��ڻ�APAMA�ն˳���</b> Ϊ�ϻ��ڻ�������һ�����Engine״̬��Ϣ���նˡ�����������£�"
		"1.�������ӵ�ַ���Ƿ񱣴淴����״̬��Ϣ."
		"2.���ӵ�Engine����."
		"3.������������event(��ѡ��."));
}


bool caMainWindow::setParam()
{
	return mParamDialog->exec();
}

void caMainWindow::linkEngine()
{	
	if (mCaiApi == NULL){
		string stdIP = mIp.toStdString();
		mCaiApi = CaiApi::CreateCaiApi(const_cast<char*>(stdIP.c_str()), mPort);
		mCaiApi->RegisterSpi(mEngineRsp);	
	}

	if(mCaiApi != NULL){
		char** pChannels = new char* [4];
		pChannels[0] = OMS_QUERY_CHANNEL;
		pChannels[1] = OMS_LOG_CHANNEL;
		pChannels[2] = OMS_EXTEND_CHANNEL;
		pChannels[3] = NULL;
		if(mCaiApi->Init(pChannels)){
			mLinkLabel->setPixmap(mLinkPic);	//������ʾ���ӱ�ʶ
			mIsLink = true;
			emit onSetCaiApi(mCaiApi);
		}
	}
}

void caMainWindow::deleteMonitor()
{
	if (mIsLink && mCaiApi != NULL){
		caDeleteDialog dlg(mCaiApi, this);
		dlg.exec();
	}	

}
void caMainWindow::disconnectEngine()
{

	if (mIsLink && mCaiApi != NULL){
		bool rtn = mCaiApi->Release();
		if(rtn){
			mLinkLabel->setPixmap(mDisconnectPic);
			mIsLink = false;
			emit onDisconnectEngine();
		}
	}
}

void caMainWindow::insertMonitor()
{
	if (mIsLink && mCaiApi != NULL){
		caMonitorDialog dlg(mCaiApi, this);
		dlg.exec();
	}	

}

void caMainWindow::sendEvent()
{
	if (mIsLink && mCaiApi != NULL){
		caEventDialog dlg(mCaiApi, this);
		dlg.exec();
	}	
}

void caMainWindow::doDisconnetEngine(const QString& reson)
{
	if (!reson.isEmpty()){
		mLinkLabel->setPixmap(mDisconnectPic);
		mIsLink = false;
		emit onDisconnectEngine();
	}

}

void caMainWindow::doSetCofig(const QString& ip, const int& port)
{
	mIp = ip;
	mPort = port;
}

#include "moc_caMainWindow.cpp"