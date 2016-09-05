#ifndef USTP_USER_STRATEGY_H
#define USTP_USER_STRATEGY_H

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QStringList>
#include <QtCore/QMap>

class USTPStrategyWidget;
class USTPOrderWidget;
class USTPCancelWidget;
class USTPSubmitWidget;
class USTPUnilateralWidget;

typedef struct
{
	char status;
	QString orderRef;
}OrderStatus;

class USTPStrategyBase : public QObject
{
	Q_OBJECT
public:
	USTPStrategyBase(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& qty, const char& bs,  const char& offset,
		const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& firstSlipPoint, const int& secSlipPoint, bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle);
	virtual~USTPStrategyBase();

protected:
	virtual void createOrder(){};

	virtual void updateInitShow(){};

	virtual bool isInMarket(const char& status);

	virtual void orderSecondIns(bool isInit, const int& qty, const double& bidPrice, const double& askPrice){};

	virtual void submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument){};

	virtual void submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns){};

	virtual void orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns){};

protected:
	virtual int getInsPrcision(const double& value);
protected:

	QThread mStrategyThread;
	QString mFirstIns;
	QString mSecIns;
	QString mOrderLabel;
	QString mUserCustomLabel;
	QString mBrokerId;
	QString mUserId;
	QString mInvestorId;
	QString mStrategyLabel;
	QString mReferenceIns;

	QString mFirstOrderRef;
	QString mSecOrderRef;
	QString mInsComplex;
	QString mExh;

	double mOrderPrice;
	double mPriceTick;
	double mFirstSlipPrice;
	double mSecondSlipPrice;
	double mFirstMarketBasePrice;
	double mSecondMarketBasePrice;
	double mSecondOrderBasePrice;
	double mFirstOrderPrice;
	double mFirstBidMarketPrice;
	double mFirstAskMarketPrice;
	double mSecondBidMarketPrice;
	double mSecondAskMarketPrice;
	double mFirstOrderBasePrice;
	double mLimitSpread;
	double mFirstMarketOldBasePrice;
	double mSecondMarketOldBasePrice;
	double mActionSuperSlipPrice;

	double mFirstInsUpperPrice;
	double mFirstInsLowerPrice;
	double mSecInsUpperPrice;
	double mSecInsLowerPrice;
	
	int mRequestId;
	int mFirstCancelTime;
	int mSecondCancelTime;
	int mFirstSlipPoint;
	int mSecSlipPoint;
	int mInsPrecision;
	int mOrderQty;
	int mCycleStall;
	int mOrderType;
	int mActionReferNum;
	int mCurrentReferIndex;
	int mActionSuperNum;
	int mFirstRemainQty;
	int mFirstTradeQty;
	int mSecondRemainQty;
	int mSecondTradeQty;
	int mFrontId;
	int mSessionId;

	int mFirstBidMarketVolume;
	int mOldFirstBidMarketVolume;
	int mFirstAskMarketVolume;
	int mOldFirstAskMarketVolume;

	char mBS;
	char mOffsetFlag;
	char mHedgeFlag;
	char mFirstInsStatus;
	char mSecInsStatus;
	bool mIsAutoFirstCancel;
	bool mIsAutoSecondCancel;
	bool mIsDeleted;
	bool mIsCycle;
	bool mIsCanMarket;
	bool mIsOppnentPrice;
	bool mIsDefineOrder;
	bool mIsActionReferTick;
	QStringList mSecActionList;
	QMap<QString, OrderStatus> mSecOrderRefMap;
	QMap<QString, QString> mReqMap;
private:
};


class USTPStarePriceArbitrage : public USTPStrategyBase
{
	Q_OBJECT
public:

	USTPStarePriceArbitrage(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& secPriceSlipPoint, const int& qty, const char& bs,  const char& offset, 
		const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle,const double& firstMarketBidPrice, 
		const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, USTPStrategyWidget* pStrategyWidget);

	virtual~USTPStarePriceArbitrage();

signals:

	void onOrderFinished(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
		const int& actionSuperNum, bool isAutoFirstCancel, bool isAutoSecCancel,bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isAllTraded, bool isReferTick, const int& orderType,
		const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice);

	void onUpdateOrderShow(const QString& userLocalId, const QString& instrument, const QString& orderStyle, const char& status, const char& bs, const double& price, const int&
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

	void doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
		const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
		const QString& localId, const QString& orderRef, const QString& tradeTime);

	void doDelOrder(const QString& orderStyle);

	void doAutoCancelSecIns();

	void doAutoCancelFirstIns();

protected:
	virtual void updateInitShow();

	virtual void orderSecondIns(bool isInit, const int& qty, const double& bidPrice, const double& askPrice);

	virtual void submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument);

	virtual void submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

	virtual void orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

private:

	void initConnect(USTPStrategyWidget* pStrategyWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget);

};


class USTPConditionArbitrage : public USTPStrategyBase
{
	Q_OBJECT
public:

	USTPConditionArbitrage(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, const int& actionSuperNum,
		bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isReferTick, const double& firstMarketBidPrice, const double& firstMarketAskPrice,
		const double& secMarketBidPrice, const double& secMarketAskPrice, USTPOrderWidget* pOrderWidget,USTPCancelWidget* pCancelWidget, USTPStrategyWidget* pStrategyWidget);

	virtual~USTPConditionArbitrage();

signals:

	void onOrderFinished(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
		const int& actionSuperNum, bool isAutoFirstCancel, bool isAutoSecCancel,bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isAllTraded, bool isReferTick, const int& orderType,
		const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice);

	void onUpdateOrderShow(const QString& userLocalId, const QString& instrument, const QString& orderStyle, const char& status, const char& bs, const double& price, const int&
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

	void doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
		const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
		const QString& localId, const QString& orderRef, const QString& tradeTime);

	void doDelOrder(const QString& orderStyle);

	void doAutoCancelSecIns();

	void doAutoCancelFirstIns();

	void doDefineTimeOrderFirstIns();

protected:
	void updateInitShow();

	void orderSecondIns(bool isInit, const int& qty, const double& bidPrice, const double& askPrice);

	void submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument);

	void submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

	void orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

private:
	
	void cancelFirstIns();

	void initConnect(USTPStrategyWidget* pStrategyWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget);

	void opponentPriceOrder(const QString& instrument, const double& basePrice);

	void noOpponentPriceOrder(const QString& instrument, const double& basePrice);

	void switchFirstInsOrder(const char& tCondition);

	void defineTimeOrder(const double& basePrice);

private:
};

class USTPOpponentArbitrage : public USTPStrategyBase
{
	Q_OBJECT
public:

	USTPOpponentArbitrage(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, const int& actionSuperNum,
		bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isReferTick, const double& firstMarketBidPrice, const double& firstMarketAskPrice, 
		const double& secMarketBidPrice, const double& secMarketAskPrice, USTPOrderWidget* pOrderWidget,USTPCancelWidget* pCancelWidget, USTPStrategyWidget* pStrategyWidget);


	virtual~USTPOpponentArbitrage();

signals:

	void onOrderFinished(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
		const int& actionSuperNum, bool isAutoFirstCancel, bool isAutoSecCancel,bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isAllTraded, bool isReferTick, const int& orderType,
		const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice);

	void onUpdateOrderShow(const QString& userLocalId, const QString& instrument, const QString& orderStyle, const char& status, const char& bs, const double& price, const int&
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

		void doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
			const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
			const QString& localId, const QString& orderRef, const QString& tradeTime);

		void doDelOrder(const QString& orderStyle);

		void doAutoCancelSecIns();

		void doAutoCancelFirstIns();

		void doDefineTimeOrderFirstIns();

protected:
	void updateInitShow();

	void orderSecondIns(bool isInit, const int& qty, const double& bidPrice, const double& askPrice);

	void submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument);

	void submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

	void orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

private:

	void cancelFirstIns();

	void initConnect(USTPStrategyWidget* pStrategyWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget);

	void opponentPriceOrder(const QString& instrument, const double& basePrice);

	void noOpponentPriceOrder(const QString& instrument, const double& basePrice);

	void switchFirstInsOrder(const char& tCondition);

	void defineTimeOrder(const double& basePrice);
};


class USTPConditionStareArbitrage : public USTPStrategyBase
{
	Q_OBJECT
public:

	USTPConditionStareArbitrage(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, const int& actionSuperNum,
		bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isReferTick, const double& firstMarketBidPrice, const double& firstMarketAskPrice,
		const double& secMarketBidPrice, const double& secMarketAskPrice, USTPOrderWidget* pOrderWidget,USTPCancelWidget* pCancelWidget, USTPStrategyWidget* pStrategyWidget);

	virtual~USTPConditionStareArbitrage();

signals:

	void onOrderFinished(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
		const int& actionSuperNum, bool isAutoFirstCancel, bool isAutoSecCancel,bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isAllTraded, bool isReferTick, const int& orderType,
		const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice);

	void onUpdateOrderShow(const QString& userLocalId, const QString& instrument, const QString& orderStyle, const char& status, const char& bs, const double& price, const int&
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

		void doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
			const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
			const QString& localId, const QString& orderRef, const QString& tradeTime);

		void doDelOrder(const QString& orderStyle);

		void doAutoCancelSecIns();

		void doAutoCancelFirstIns();

protected:
	virtual void updateInitShow();

	virtual void orderSecondIns(bool isInit, const int& qty, const double& bidPrice, const double& askPrice);

	virtual void submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument);

	virtual void submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

	virtual void orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

private:

	void cancelFirstIns();

	void switchFirstInsOrder(const char& tCondition);

	void initConnect(USTPStrategyWidget* pStrategyWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget);

	void conditionStarePriceOrder(const QString& instrument, const double& basePrice);

};


class USTPOpponentStareArbitrage : public USTPStrategyBase
{
	Q_OBJECT
public:

	USTPOpponentStareArbitrage(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, const int& actionSuperNum,
		bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isReferTick, const double& firstMarketBidPrice, const double& firstMarketAskPrice, 
		const double& secMarketBidPrice, const double& secMarketAskPrice, USTPOrderWidget* pOrderWidget,USTPCancelWidget* pCancelWidget, USTPStrategyWidget* pStrategyWidget);


	virtual~USTPOpponentStareArbitrage();

signals:

	void onOrderFinished(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
		const int& actionSuperNum, bool isAutoFirstCancel, bool isAutoSecCancel,bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isAllTraded, bool isReferTick, const int& orderType,
		const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice);

	void onUpdateOrderShow(const QString& userLocalId, const QString& instrument, const QString& orderStyle, const char& status, const char& bs, const double& price, const int&
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

		void doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
			const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
			const QString& localId, const QString& orderRef, const QString& tradeTime);

		void doDelOrder(const QString& orderStyle);

		void doAutoCancelSecIns();

		void doAutoCancelFirstIns();

		void doDefineTimeOrderFirstIns();

protected:
	void updateInitShow();

	void orderSecondIns(bool isInit, const int& qty, const double& bidPrice, const double& askPrice);

	void submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument);

	void submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

	void orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

private:

	void cancelFirstIns();

	void initConnect(USTPStrategyWidget* pStrategyWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget);

	void opponentPriceOrder(const QString& instrument, const double& basePrice);

	void noOpponentPriceOrder(const QString& instrument, const double& basePrice);

	void switchFirstInsOrder(const char& tCondition);

	void defineTimeOrder(const double& basePrice);
};


class USTPSpeculateOrder : public USTPStrategyBase
{
	Q_OBJECT
public:

	USTPSpeculateOrder(const QString& orderLabel, const QString& speLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& qty, const char& bs,  const char& offset,
		const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& firstSlipPoint, const int& secSlipPoint, bool isAutoFirstCancel, bool isAutoSecCancel,
		bool isCycle, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, USTPSubmitWidget* pSubmitWidget);

	virtual~USTPSpeculateOrder();

signals:

	void onOrderFinished(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
		const int& actionSuperNum, bool isAutoFirstCancel, bool isAutoSecCancel,bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isAllTraded, bool isReferTick, const int& orderType,
		const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice);

	void onUpdateOrderShow(const QString& userLocalId, const QString& instrument, const QString& orderStyle, const char& status, const char& bs, const double& price, const int&
		orderVolume, const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedge, const double& orderPrice);

	protected slots:
		void doSubmitOrder(const QString& orderLabel, const QString& speLabel, const QString& ins, const char& direction, const char& offsetFlag, const int& volume, const double& orderPrice);

		void doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
			const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
			const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId);

		void doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
			const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
			const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId);

		void doUSTPErrRtnOrderAction(const char& actionFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId,
			const QString& orderSysId, const QString& userActionLocalId, const QString& orderActionRef, const double& orderPrice, 
			const int& volumeChange, const int& errorId, const QString& errorMsg, const int& reqId);

		void doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
			const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
			const QString& localId, const QString& orderRef, const QString& tradeTime);

		void doDelOrder(const QString& orderStyle);

protected:
	virtual void submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument);

	virtual void submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

	virtual void orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

private:

	void initConnect(USTPSubmitWidget* pSubmitWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget);

private:
	QString mTempOrderLabel;
	char mTempOffsetFlag;
	char mTempBS;
	QString mTempFirstIns;
	int mTempOrderQty;
	double mTempOrderPrice;
	QString mSpeOrderLabel;
};

class USTPUnilateralOrder : public USTPStrategyBase
{
	Q_OBJECT
public:

	USTPUnilateralOrder(const QString& orderLabel, const QString& insName, const QString& referIns, const int& orderType, const char& direction, const char& offsetFlag, 
		const double& openPrice, const double& closePrice, const double& opponentPrice, const double& referPrice, const double& orderBasePrice, const double& insBidPrice,
		const double insAskPrice, const double& referBidPrice, const double& referAskPrice, const int& qty, const int& cancelTime, const int& superPriceTick, bool isAutoCancel,
		bool isCycle, bool isFirstIns, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, USTPUnilateralWidget* pUnilateralWidget);

	virtual~USTPUnilateralOrder();

signals:

	void onUnilateralFinished(const QString& orderLabel, const QString& insName, const QString& referIns, const int& orderType, const char& direction, const char& offsetFlag, 
		const double& openPrice, const double& closePrice, const double& opponentPrice, const double& referPrice, const double& orderBasePrice, const double& insBidPrice,
		const double insAskPrice, const double& referBidPrice, const double& referAskPrice, const int& qty, const int& cancelTime, const int& superPriceTick, bool isAutoCancel,
		bool isCycle, bool isFirstIns, bool isAllTraded);

	void onUpdateOrderShow(const QString& userLocalId, const QString& instrument, const QString& orderStyle, const char& status, const char& bs, const double& price, const int&
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

		void doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
			const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
			const QString& localId, const QString& orderRef, const QString& tradeTime);

		void doDelOrder(const QString& orderStyle);

		void doAutoCancelFirstIns();

protected:
	virtual void updateInitShow();

	virtual void submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument);

	virtual void submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

	virtual void orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns);

private:
	void initConnect(USTPUnilateralWidget* pUnilateralWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget);
	void switchFirstInsOrder(const char& tCondition);
	void cancelFirstIns();
	void firstInsAutoOrder();
	void secInsAutoOrder();
	void firstInsAutoCancel(const QString& insName);
	void secInsAutoCancel(const QString& insName);
private:
	double mReferBidMarketPrice;
	double mReferAskMarketPrice;
	double mOpponentPriceSpread;
	double mReferPriceSpread;	
	double mClosePriceSpread;
	double mTradeAvgPrice;
	double mBidOpponentBasePrice;
	double mAskOpponentBasePrice;
	double mBidOrderBasePrice;
	double mAskOrderBasePrice;
	int mOffsetType;
	bool mIsFirstIns;
	char mOffsetLabel;
};

#endif