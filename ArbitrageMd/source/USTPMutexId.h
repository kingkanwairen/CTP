#ifndef USTP_MUTEX_ID_H
#define USTP_MUTEX_ID_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QMap>

class USTPMutexId : public QObject
{
	Q_OBJECT
public:
	USTPMutexId();
	virtual ~USTPMutexId();

	static bool initialize();

	static bool setUserInfo(const QString& userId, const QString& psw, int maxId, int frontId, int sessionId);

	static QString getUserId();

	static QString getLoginPsw();

	static int getOrderRef();

	static int getMutexId();

	static bool setInsPriceTick(const QString& ins, const QString& exchangeId, const double& tick, const int& volumeMultiple);

	static double getInsPriceTick(const QString& ins);

	static QString getInsExchangeId(const QString& ins);

	static int getInsMultiple(const QString& ins);


	static bool finalize();
protected:
private:
	static USTPMutexId* mThis;
	int nOrderId;
	int mFrontId;
	int mSessionId;
	int nRequestIndex;

	QString mUserId;
	QString mPsw;
	QString mInvestorId;
	QMap<QString, double> mInsTicks;
	QMap<QString, QString> mInsExhs;
	QMap<QString, int> mInsMultiple;
	QMutex mRequestMutex;
	QMutex mOrderRefMutex;
};
#endif