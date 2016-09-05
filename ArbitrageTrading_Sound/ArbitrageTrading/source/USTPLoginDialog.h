#ifndef USTP_LOGIN_DIALOG_H
#define USTP_LOGIN_DIALOG_H

#include <QtGui/QtGui>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
QT_END_NAMESPACE


class USTPLoginDialog : public QDialog
{
    Q_OBJECT

public:
	USTPLoginDialog(QWidget* parent = 0);

	~USTPLoginDialog();

public slots:
	
	void doUserLogin();
	void doUSTPTradeRspUserLogin(const QString& tradingDate, const QString& brokerId, const QString& userId, const int& maxLocalId, const int& frontId,
		const int& sessionId, const int& errorId, const QString& errorMsg, bool bIsLast);
	void doUSTPTradeSettlementInfoConfirm(const QString& brokerId, const QString& investorId, const int& errorId, const QString& errorMsg);
private:
	void initConnect();

protected:
	void paintEvent(QPaintEvent *event);

private:
	
	QLabel* mUserLabel;
	QLabel* mPasswordLabel;
	QLineEdit* mUserEdit;
	QLineEdit* mPasswordEdit;
	QPushButton* mLoginBtn;
	QPushButton* mCancelBtn;
	QGridLayout* mGridLayout;
	QHBoxLayout* mBtnLayout;
	QVBoxLayout* mViewLayout;
	QString mBrokerId;
	QString mUserId;
	QString mPassword;
};

#endif
