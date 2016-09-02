#ifndef USTP_MAIN_WINDOW_H
#define USTP_MAIN_WINDOW_H
#include <QtGui/QtGui>
#include "USTPOrderWidget.h"
#include "USTPTradeWidget.h"
#include "USTPositionWidget.h"
#include "USTPCancelWidget.h"
#include "USTPMarketWidget.h"
#include "USTPSubmitWidget.h"
#include "USTPTabWidget.h"
#include "USTPRspKeyDown.h"

QT_BEGIN_NAMESPACE
class QToolBar;
class QAction;
class QTabWidget;
class QDockWidget;
QT_END_NAMESPACE

class USTPMainWindow : public QMainWindow
{
	Q_OBJECT

public:

	USTPMainWindow(QWidget * parent = 0);

	~USTPMainWindow();

private:
	void createPic();
	void createToolBar();
	void createActions();
	void createMenus();
	void createStatusBar();
	QDockWidget * createDockWidget(const QString& title, QWidget* pWidget, 
		Qt::DockWidgetAreas allowAreas = Qt::AllDockWidgetAreas, Qt::DockWidgetArea initArea = Qt::TopDockWidgetArea);

signals:
	void onUpdateKey(const int& bidKey, const int& askKey);

private slots:
	void about();
	void doSubscribeMarket();
	void doNewKey();

private:
	void initConnect();
	void doQryInvestorPosition();

public slots:

	void doUSTPTradeFrontConnected();

	void doUSTPMdFrontConnected();

	void doUSTPTradeFrontDisconnected(int reason);

	void doUSTPMdFrontDisconnected(int reason);

	void doCreateNewKey(const int& bidKey, const int& askKey);

	void doUSTPMdRspUserLogin(const QString& brokerId, const QString& userId, const int& errorId, const QString& errorMsg, bool bIsLast);

	void doUSTPTradeRspUserLogin(const QString& tradingDate, const QString& brokerId, const QString& userId, const int& maxLocalId, const int& frontId,
		const int& sessionId, const int& errorId, const QString& errorMsg, bool bIsLast);
	
private:
	KeyDownHock* mKeyDownApi;
	USTPRspKeyDown* mRspKeyDown;
	USTPOrderWidget* mOrderWidget;
	USTPTradeWidget* mTradeWidget;
	USTPositionWidget* mPositionWidget;
	USTPCancelWidget* mCancelWidget;
	USTPSubmitWidget* mSubmitWidget;
	USTPMarketWidget* mMarketWidget;
	
	QDockWidget* mOrderDockWidget;
	QDockWidget* mTradeDockWidget;
	QDockWidget* mPositionDockWidget;
	QDockWidget* mMarketDockWidget;

	QToolBar* mToolBar;
	QAction* mHelpAction;
	QAction* mExitAction;
	QAction* mMarketAction;
	QAction* mKeyAction;

	QMenu* mFileMenu;
	QMenu* mOperMenu;
	QMenu* mHelpMenu;

	QPixmap mLinkPic;
	QPixmap mDisconnectPic;
	QLabel* mTradeLinkLabel;
	QLabel* mMdLinkLabel;
	QLabel* mMessageLabel;

	QString mBrokerId;
	QString mUserId;
	QString mPsw;
	QIcon mTabIcon;
	int mBidKey;
	int mAskKey;
	int mTabIndex;
	QMap<QString, QString> mInss;
};

#endif
