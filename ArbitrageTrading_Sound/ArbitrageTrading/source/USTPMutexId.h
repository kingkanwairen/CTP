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

	static int getOrderRef();

	static int getMutexId();

	static int getNewOrderIndex();

	static QString getUserId();

	static QString getLoginPsw();

	static int getFrontId();
	
	static int getSessionId();

	static void setInvestorId(const QString& investorId);

	static QString getInvestorId();

	static bool setInsBidPosition(const QString& ins, const int& qty);

	static bool setInsAskPosition(const QString& ins, const int& qty);

	static bool addBidPosition(const QString& ins, const int& qty);

	static bool addAskPosition(const QString& ins, const int& qty);

	static bool setReferenceIns(const QString& ins);

	static int getInsBidPosition(const QString& ins);

	static int getInsAskPosition(const QString& ins);

	static bool setInsPriceTick(const QString& ins, const QString& exchangeId, const double& tick, const int& volumeMultiple);

	static QString getInsExchangeId(const QString& ins);

	static int getInsMultiple(const QString& ins);

	static bool setInsMarketMaxVolume(const QString& ins, const int& volume);

	static double getInsPriceTick(const QString& ins);

	static bool getTotalBidPosition(QMap<QString, int>& bidPostions);

	static bool getTotalAskPosition(QMap<QString, int>& askPostions);

	static bool initActionNum(const QString& ins, const int& num);

	static bool setLimitSpread(const QString& ins, const int& spread);

	static int getLimitSpread(const QString& ins);

	static bool updateActionNum(const QString& ins);

	static int getActionNum(const QString& ins);

	static bool getTotalActionNum(QMap<QString, int>& actionNums);
	
	static int getInsMarketMaxVolume(const QString& ins);

	static QString getReferenceIns();

	static int getMarketIndex();

	static bool setUpperLowerPrice(const QString& ins, const double& upperPrice, const double& lowerPrice);
	
	static double getUpperPrice(const QString& ins);

	static double getLowerPrice(const QString& ins);

	static bool finalize();
protected:
private:
	static USTPMutexId* mThis;
	bool mIsBaseTime;
	int nOrderId;
	int mFrontId;
	int mSessionId;
	int nRequestIndex;
	int nNewOrderIndex;
	int nMarketIndex;
	QString mUserId;
	QString mPsw;
	QString mInvestorId;
	QString mReferenceIns;
	QMap<QString, int> mBidQtys;
	QMap<QString, int> mAskQtys;
	QMap<QString, double> mInsTicks;
	QMap<QString, QString> mInsExhs;
	QMap<QString, int> mMaxMarketQtys;
	QMap<QString, int> mInsMultiple;
	QMap<QString, int> mInsActionNums;
	QMap<QString, int> mLimitSpreads;
	QMap<QString, double> mUpperPrices;
	QMap<QString, double> mLowerPrices;
	QMutex mRequestMutex;
	QMutex mOrderRefMutex;
	QMutex mNewOrderMutex;
	QMutex mMarketMutex;
	QMutex mInsActionMutex;
};
#endif