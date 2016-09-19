#ifndef USTP_USER_STRATEGY_H
#define USTP_USER_STRATEGY_H

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QStringList>
#include <QtCore/QMap>

class USTPOrderWidget;
class USTPCancelWidget;
class USTPSubmitWidget;


class USTPStrategyBase : public QObject
{
	Q_OBJECT
public:
	USTPStrategyBase(const QString& orderLabel, const QString& instrumentId, const double& orderPriceTick, const int& qty, const char& bs, const char& offset, const int& openSuperPoint,
		const int& stopProfitPoint, const int& stopLossPoint);
	virtual~USTPStrategyBase();

protected:

	virtual bool isInMarket(const char& status);

	virtual bool isInParkedMarket(const char& status);

	virtual void submitAction(const QString& orderRef, const QString& instrumentId){};

	virtual void submitOrder(const QString& instrumentId, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition){};

	virtual void orderInsert(const int& reqId, const QString& instrumentId, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition){};

protected:
	virtual int getInsPrcision(const double& value);
protected:

	QThread mStrategyThread;
	QString mInstrumentId;
	QString mOrderLabel;
	QString mBrokerId;
	QString mUserId;
	QString mInvestorId;
	QString mOrderSysId;
	QString mActionExh;
	
	QString mExh;
	
	double mPriceTick;
	double mOrderPrice;
	double mBidMarketPrice;
	double mAskMarketPrice;
	double mInsUpperPrice;
	double mInsLowerPrice;
	double mOpenTick;
	double mStopProfitTick;
	double mStopLossTick;
	int mRequestId;
	int mOrderQty;
	int mRemainQty;
	int mFrontId;
	int mSessionId;
	int mInsPrecision;
	int mFirstReqestId;
		
	char mBS;
	char mOffsetFlag;
	char mFirstInsStatus;
	char mFirstInsParkedStatus;
	QMap<int, QString> mReqMap;
	QMap<QString, QString> mParkedOrderMap;
private:
};


class USTPSpeculateOrder : public USTPStrategyBase
{
	Q_OBJECT
public:

	USTPSpeculateOrder(const QString& orderLabel, const QString& speOrderLabel, const QString& instrumentId, const double& orderPriceTick, const int& qty, const char& bs, const char& offset,
		const int& openSuperPoint, const int& stopProfitPoint, const int& stopLossPoint, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, USTPSubmitWidget* pSubmitWidget);

	virtual~USTPSpeculateOrder();

signals:

	void onOrderFinished(const QString& orderLabel, const QString& instrumentId);

	void onUpdateOrderShow(const QString& orderLabel, const QString& requestId, const QString& instrumentId, const char& status, const char& bs, const int&
		orderVolume, const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedge, const double& orderPrice);

	protected slots:
		void doSubmitOrder(const QString& orderLabel, const QString& speLabel, const QString& ins, const char& direction, const char& offsetFlag, const int& volume, const double& orderPrice);
		
		void doSubmitParkedOrder(const QString& orderLabel, const QString& speLabel, const QString& ins, const char& direction, const char& offsetFlag, const int& volume, const double& orderPrice);

		void doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
			const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
			const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId);

		void doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
			const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
			const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId);

		void doUSTPErrRtnOrderAction(const char& actionFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId,
			const QString& orderSysId, const QString& userActionLocalId, const QString& orderActionRef, const double& orderPrice, 
			const int& volumeChange, const int& errorId, const QString& errorMsg, const int& reqId);

		void doUSTPRemoveParkedOrder(const QString& brokerId, const QString& userId, const QString& parkedOrderId);

		void doUSTPParkedOrderInsert(const QString& parkedOrderId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
			const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
			const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& errorId,  const int& reqId);	

		void doUSTPParkedOrderAction(const QString& parkedOrderActionId, const QString& orderActionRef, const QString& instrumentId, const double& orderPrice, const char& orderStatus,
			const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderRef, const QString& statusMsg, const int& errorId,  const int& reqId);


		void doDelOrder(const QString& orderLabel);

		void doCancelParkedOrder();

protected:

	virtual void submitAction(const QString& orderRef, const QString& instrumentId);

	virtual void submitOrder(const QString& instrumentId, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition);

	virtual void orderInsert(const int& reqId, const QString& instrumentId, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition);

	virtual void submitParkedAction(const QString& orderRef, const QString& instrumentId);

	virtual void submitParkedOrder(const QString& instrumentId, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition);

	virtual void submitParkedCancel(const QString& parkedOrderId);
private:

	void initConnect(USTPSubmitWidget* pSubmitWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget);

	void parkedAction();

	void continueAction();

private:
	QString mTempOrderLabel;
	char mTempOffsetFlag;
	char mTempBS;
	QString mTempFirstIns;
	int mTempOrderQty;
	double mTempOrderPrice;
	QString mSpeOrderLabel;
	bool mIsParkedOrder;
};

class USTPUnilateralOrder : public USTPStrategyBase
{
	Q_OBJECT
public:

	USTPUnilateralOrder(const QString& orderLabel, const QString& instrumentId, const double& orderPriceTick, const int& qty, const char& bs, const char& offset, const int& openSuperPoint,
		const int& stopProfitPoint, const int& stopLossPoint, const double& bidPrice, const double& askPrice, const double& lastPrice, USTPOrderWidget* pOrderWidget,
		USTPCancelWidget* pCancelWidget, USTPSubmitWidget* pSubmitWidget);

	virtual~USTPUnilateralOrder();

signals:

	void onOrderFinished(const QString& orderLabel, const QString& instrumentId);

	void onUpdateOrderShow(const QString& orderLabel, const QString& requestId, const QString& instrumentId, const char& status, const char& bs, const int&
		orderVolume, const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedge, const double& orderPrice);

	protected slots:
		void doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
			const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
			const double& lowestPrice, const int& volume);

		void doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
			const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
			const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId);

		void doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
			const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
			const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId);

		void doUSTPErrRtnOrderAction(const char& actionFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId,
			const QString& orderSysId, const QString& userActionLocalId, const QString& orderActionRef, const double& orderPrice, 
			const int& volumeChange, const int& errorId, const QString& errorMsg, const int& reqId);

		void doDelOrder(const QString& orderLabel);


protected:

	virtual void submitAction(const QString& orderRef, const QString& instrumentId);

	virtual void submitOrder(const QString& instrumentId, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition);

	virtual void orderInsert(const int& reqId, const QString& instrumentId, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition);

private:
	void initConnect(USTPSubmitWidget* pSubmitWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget);
	void switchFirstInsOrder(const double& price, const char& tCondition);
	void cancelFirstIns();
private:
	double mOpenOrderPrice;
	double mStopProfitPrice;
	double mStopLossPrice;
	double mOrderBasePrice;
	bool mIsDeleted;
	char mSubmitStatus;
};

#endif