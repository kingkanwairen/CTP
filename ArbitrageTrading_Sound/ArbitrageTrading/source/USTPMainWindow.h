#ifndef USTP_MAIN_WINDOW_H
#define USTP_MAIN_WINDOW_H
#include <QtGui/QtGui>
#include "USTPTradeWidget.h"


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
	void createActions();
	void createMenus();
	void createStatusBar();

signals:
	void onUpdateKey(const int& bidKey, const int& askKey);

private slots:
	void about();

private:
	void initConnect();

public slots:

	void doUSTPTradeFrontConnected();

	void doUSTPTradeFrontDisconnected(int reason);

	void doUSTPTradeRspUserLogin(const QString& tradingDate, const QString& brokerId, const QString& userId, const int& maxLocalId, const int& frontId,
		const int& sessionId, const int& errorId, const QString& errorMsg, bool bIsLast);
	
private:
	
	USTPTradeWidget* mTradeWidget;

	QAction* mHelpAction;
	QAction* mExitAction;

	QMenu* mFileMenu;
	QMenu* mHelpMenu;

	QPixmap mLinkPic;
	QPixmap mDisconnectPic;
	QLabel* mTradeLinkLabel;
	QLabel* mMessageLabel;

	QString mBrokerId;
	QString mUserId;
	QString mPsw;
	QIcon mTabIcon;
};

#endif
