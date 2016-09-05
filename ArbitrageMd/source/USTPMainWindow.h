#ifndef USTP_MAIN_WINDOW_H
#define USTP_MAIN_WINDOW_H
#include <QtGui/QtGui>
#include "USTPMarketWidget.h"

QT_BEGIN_NAMESPACE
class QToolBar;
class QAction;
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
private slots:
	void about();
	void doSubscribeMarket();

private:
	void initConnect();

public slots:

	void doUSTPMdFrontConnected();

	void doUSTPMdFrontDisconnected(int reason);
	
private:
	
	USTPMarketWidget* mMarketWidget;
	QToolBar* mToolBar;
	QAction* mHelpAction;
	QAction* mExitAction;
	QAction* mMarketAction;

	QMenu* mFileMenu;
	QMenu* mHelpMenu;

	QPixmap mLinkPic;
	QPixmap mDisconnectPic;
	QLabel* mMdLinkLabel;
	QLabel* mMessageLabel;

	QString mBrokerId;
	QString mUserId;
	QString mPsw;
};

#endif
