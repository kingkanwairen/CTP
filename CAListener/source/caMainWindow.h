#ifndef svrMainWindow_h
#define svrMainWindow_h

#include <QtGui/QMainWindow>
#include <QtGui/QAction>
#include <QtGui/QToolBar>
#include "caEngineRsp.h"
#include "caLogWidget.h"
#include "caConfigDialog.h"
#include "caStatusWidget.h"
#include "caOrderWidget.h"
#include "caTradeWidget.h"
#include "caPositionWidget.h"


/************************************************************************
					主窗口类

				Function: 主要操作窗口
************************************************************************/

QT_BEGIN_NAMESPACE
class QToolBar;
class QAction;
class QTabWidget;
class QDockWidget;
QT_END_NAMESPACE

class caMainWindow : public QMainWindow
{
	Q_OBJECT

public:

	caMainWindow(QWidget * parent = 0);

	~caMainWindow();

private:
	void createPic();
	void createToolBar();
	void createActions();
	void createMenus();
	void createStatusBar();
	QDockWidget * createDockWidget(const QString& title, QWidget* pWidget, 
		Qt::DockWidgetAreas allowAreas = Qt::AllDockWidgetAreas, Qt::DockWidgetArea initArea = Qt::TopDockWidgetArea);
	bool readConfig(QString& ip, int& port, QString& path, int& status);

signals:
	void onSetCaiApi(CaiApi*  pCaiApi);
	void onDisconnectEngine();

private slots:
	bool setParam();
	void about();
	void linkEngine();
	void insertMonitor();
	void sendEvent();
	void deleteMonitor();
	void disconnectEngine();
	void doSetCofig(const QString& ip, const int& port);

public slots:
	void doDisconnetEngine(const QString& reson);
	
private:
	
	CaiApi* mCaiApi;
	caEngineRsp* mEngineRsp;
	caLogWidget* mLogWidget;
	caStatusWidget* mStatusWidget;
	caOrderWidget* mOrderWidget;
	caTradeWidget* mTradeWidget;
	caPositionWidget* mPositionWidget;

	QDockWidget* mStatusDockWidget;
	QDockWidget* mOrderDockWidget;
	QDockWidget* mTradeDockWidget;
	QDockWidget* mPositionDockWidget;

	caConfigDialog* mParamDialog;
	QTabWidget* mTabWidget;
	QToolBar* mToolBar;
	QAction* mSetAction;
	QAction* mHelpAction;
	QAction* mExitAction;
	QAction* mLinkAction;
	QAction* mInsertAction;
	QAction* mSendAction;
	QAction* mDeleteAction;
	QAction* mDisconnetAction;

	QMenu* mFileMenu;
	QMenu* mOperMenu;
	QMenu* mHelpMenu;

	QPixmap mLinkPic;
	QPixmap mDisconnectPic;
	QLabel* mLinkLabel;
	QLabel* mMessageLabel;

	bool mIsLink;
	QString mIp;
	QString mPath;
	int mPort;
	int mStatus;
};

#endif
