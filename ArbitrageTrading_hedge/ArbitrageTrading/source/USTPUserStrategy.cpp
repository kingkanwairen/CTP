#include "USTPUserStrategy.h"
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <Windows.h>
#include "USTPConfig.h"
#include "USTPLogger.h"
#include "USTPCtpLoader.h"
#include "USTPMutexId.h"
#include "USTPTradeApi.h"
#include "USTPStrategyWidget.h"
#include "USTPSubmitWidget.h"
#include "USTPUnilateralWidget.h"

#define USTP_FTDC_OS_ORDER_SUBMIT 'O'
#define USTP_FTDC_OS_CANCEL_SUBMIT 'C'
#define USTP_FTDC_OS_ORDER_NO_ORDER 'N'
#define USTP_FTDC_OS_ORDER_ERROR 'E'

#define FIRST_INSTRUMENT tr("FirstInstrument")
#define SECOND_INSTRUMENT tr("SecondInstrument")
#define DEFINE_ORDER_TIME 10

USTPStrategyBase::USTPStrategyBase(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& qty, const char& bs,  const char& offset,
								   const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& firstSlipPoint, const int& secSlipPoint, bool isAutoFirstCancel,
								   bool isAutoSecCancel, bool isCycle)
{	
	
	mOrderLabel = orderLabel;
	mOrderPrice = orderPriceTick;
	mFirstCancelTime = cancelFirstTime;
	mSecondCancelTime = cancelSecTime;
	mFirstSlipPoint = firstSlipPoint;
	mSecSlipPoint = secSlipPoint;
	mIsAutoFirstCancel = isAutoFirstCancel;
	mIsAutoSecondCancel = isAutoSecCancel;
	mOrderQty = qty;
	mCycleStall = cycleStall;
	mFirstIns = firstIns;
	mSecIns = secIns;
	mReferenceIns = secIns;
	mOffsetFlag = offset;
	mHedgeFlag = hedge;
	mBS = bs;
	mIsCycle = isCycle;
	mIsDeleted = false;
	mInsComplex = mFirstIns + tr("|") + mSecIns;
	mFrontId = USTPMutexId::getFrontId();
	mSessionId = USTPMutexId::getSessionId();
	mExh = USTPMutexId::getInsExchangeId(secIns);
	mPriceTick = USTPMutexId::getInsPriceTick(secIns);
	mFirstInsUpperPrice = USTPMutexId::getUpperPrice(firstIns);
	mFirstInsLowerPrice = USTPMutexId::getLowerPrice(firstIns);
	mSecInsUpperPrice = USTPMutexId::getUpperPrice(secIns);
	mSecInsLowerPrice = USTPMutexId::getLowerPrice(secIns);
	mInsPrecision = getInsPrcision(mPriceTick);
}

USTPStrategyBase::~USTPStrategyBase()
{

}

bool USTPStrategyBase::isInMarket(const char& status)
{
	if (THOST_FTDC_OST_PartTradedQueueing == status || THOST_FTDC_OST_PartTradedNotQueueing == status ||
		THOST_FTDC_OST_NoTradeQueueing == status || THOST_FTDC_OST_NoTradeNotQueueing == status){
			return true;
	}
	return false;
}

int USTPStrategyBase::getInsPrcision(const double& value)
{
	if (value >= VALUE_1){
		return 0;
	}else if(value * 10 >= VALUE_1){
		return 1;
	}else if(value * 100 >= VALUE_1){
		return 2;
	}else if(value * 1000 >= VALUE_1){
		return 3;
	}
	return 0;
}


USTPStarePriceArbitrage::USTPStarePriceArbitrage(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& secPriceSlipPoint, const int& qty, const char& bs,  const char& offset, 
												 const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle, const double& firstMarketBidPrice,
												 const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, USTPStrategyWidget* pStrategyWidget)
												 :USTPStrategyBase(orderLabel, firstIns, secIns, orderPriceTick, qty, bs, offset, hedge, cancelFirstTime, cancelSecTime, cycleStall, 0, secPriceSlipPoint, isAutoFirstCancel, isAutoSecCancel, isCycle)
{		
	moveToThread(&mStrategyThread);
	mStrategyThread.start();
	mFirstTradeQty = 0;
	mFirstRemainQty = mOrderQty;
	mSecondTradeQty = 0;
	mOrderType = 0;
	mRequestId = -1;
	mActionReferNum = 0;
	mCurrentReferIndex = 0;
	mActionSuperNum = 0;
	mIsActionReferTick = false;
	if(THOST_FTDC_D_Buy == bs)
		mFirstMarketBasePrice = secMarketBidPrice;
	else
		mFirstMarketBasePrice = secMarketAskPrice;
	mFirstOrderBasePrice = 0.0;
	mFirstMarketOldBasePrice = 0.0;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mSecInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mSecondSlipPrice = secPriceSlipPoint * mPriceTick;
	mBrokerId = USTPCtpLoader::getBrokerId();
	mUserId = USTPMutexId::getUserId();
	mInvestorId = USTPMutexId::getInvestorId();
	mLimitSpread = USTPMutexId::getLimitSpread(mInsComplex) * mPriceTick;
	mIsCanMarket = (USTPMutexId::getInsMarketMaxVolume(secIns) > 0) ? true : false;
	initConnect(pStrategyWidget, pOrderWidget, pCancelWidget);	
	updateInitShow();
}

USTPStarePriceArbitrage::~USTPStarePriceArbitrage()
{
	mStrategyThread.quit();
	mStrategyThread.wait();
}

void USTPStarePriceArbitrage::initConnect(USTPStrategyWidget* pStrategyWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget)
{
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), 
		this, SLOT(doUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)),
		this, SLOT(doUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)),
		this, SLOT(doUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pOrderWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pCancelWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(pCancelWidget, SIGNAL(onDelOrder(const QString& )), this, SLOT(doDelOrder(const QString& )), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, 
		const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, const double&)), pStrategyWidget, SLOT(doOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, 
		const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&,
		const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, 
		const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, const double&)), pCancelWidget, SLOT(doOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, 
		const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, 
		const double&)), Qt::QueuedConnection);
}

void USTPStarePriceArbitrage::updateInitShow()
{
	mRequestId = USTPMutexId::getMutexId();
	QString szReqId = QString::number(mRequestId);
	emit onUpdateOrderShow(szReqId, mFirstIns, mOrderLabel, 'N', mBS, 0.0, mOrderQty, mOrderQty, 0, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
}

void USTPStarePriceArbitrage::doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
													   const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
													   const double& lowestPrice, const int& volume)
{	
	if (mSecIns != instrumentId)	//监听第二腿行情
		return;
	mFirstMarketOldBasePrice = mFirstMarketBasePrice;
	mSecondAskMarketPrice = askPrice;
	mSecondBidMarketPrice = bidPrice;
	if(THOST_FTDC_D_Buy == mBS)	//记录实时第二腿的行情，作为第一腿报单时的参考
		mFirstMarketBasePrice = bidPrice;
	else
		mFirstMarketBasePrice = askPrice;

	if(((USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus) || (THOST_FTDC_OST_Canceled == mFirstInsStatus))){	//第一腿初次或者设定时间撤单后报单
		double price = 0.0;
		if(THOST_FTDC_D_Buy == mBS){
			if(mFirstMarketBasePrice > mFirstMarketOldBasePrice + mLimitSpread)
				return;
			price = bidPrice + mOrderPrice;
			mFirstOrderBasePrice = bidPrice;
		}else{
			if(mFirstMarketOldBasePrice > mFirstMarketBasePrice + mLimitSpread)
				return;
			price = askPrice + mOrderPrice;
			mFirstOrderBasePrice = askPrice;
		}	
		if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus){
			if(mIsDeleted)
				return;
			orderInsert(mRequestId, FIRST_INSTRUMENT, mFirstIns, price, mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, true);
		}else
			submitOrder(FIRST_INSTRUMENT, mFirstIns, price, mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, true);

	}else if(USTPStrategyBase::isInMarket(mFirstInsStatus)){	//1.非设定时间撤单的情况，第一腿委托成功，第二腿行情发生变化，腿一撤单重发;2.设定时间撤单的情况，定时超时，根据行情触发。
		if((THOST_FTDC_D_Buy == mBS && ((bidPrice > mFirstOrderBasePrice) || (bidPrice < mFirstOrderBasePrice))) ||
		  (THOST_FTDC_D_Sell == mBS && ((askPrice > mFirstOrderBasePrice) || (askPrice < mFirstOrderBasePrice)))){
			mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
			submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
		}
	}
}

void USTPStarePriceArbitrage::submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	if(isFirstIns && mIsDeleted)	//撤掉报单，合约一禁止下新单
		return;
	orderInsert(USTPMutexId::getMutexId(), insLabel, instrument, orderPrice, direction, qty, priceType, timeCondition, isFirstIns);	
}

void USTPStarePriceArbitrage::orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	if(USTPMutexId::getActionNum(mInsComplex) > MAX_CANCEL_NUM)
		return;
	double adjustPrice = (priceType == THOST_FTDC_OPT_LimitPrice) ? orderPrice : 0.0;
	QString orderRef;
	if(isFirstIns){
		mFirstInsStatus = USTP_FTDC_OS_ORDER_SUBMIT;
		USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
		mFirstOrderRef = orderRef;
	}else{
		OrderStatus state;
		state.status = USTP_FTDC_OS_ORDER_SUBMIT;
		USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
		state.orderRef = orderRef;
		mSecOrderRefMap.insert(orderRef, state);
	}
	QString szReqId = QString::number(reqId);
	mReqMap.insert(orderRef, szReqId);
	emit onUpdateOrderShow(szReqId, instrument, mOrderLabel, 'N', direction, 0.0, qty, qty, 0, mOffsetFlag, priceType, mHedgeFlag, mOrderPrice);
	
	if(mIsAutoFirstCancel && isFirstIns)
		QTimer::singleShot(mFirstCancelTime, this, SLOT(doAutoCancelFirstIns()));
	else if(mIsAutoSecondCancel && isFirstIns == false){
		mSecActionList.append(orderRef);
		QTimer::singleShot(mSecondCancelTime, this, SLOT(doAutoCancelSecIns()));
	}
	//条件日志
#ifdef _DEBUG
	int nIsSecCancel = mIsAutoSecondCancel ? 1 : 0;
	int nIsSecIns = isFirstIns ? 0 : 1;
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderInsert]   Instrument: ")) + instrument +  QString(tr("  RequestId: ")) + szReqId + QString(tr("  OrderRef: ")) + orderRef + 
		QString(tr("  UserId: ")) + mUserId + QString(tr("  PriceType: ")) + QString(priceType) + QString(tr("  OrderPrice: ")) + QString::number(adjustPrice) + QString(tr("  OrderVolume: ")) + 
		QString::number(qty) + QString(tr("  Direction: ")) + QString(direction) + QString(tr("  SecAutoCancel: ")) + QString::number(nIsSecCancel) + QString(tr("  IsSecIns: ")) + QString::number(nIsSecIns);
	USTPLogger::saveData(data);
#endif	
}

void USTPStarePriceArbitrage::doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
											 const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
											 const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
	emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, orderStatus, direction, orderPrice, orderVolume, remainVolume, tradeVolume, offsetFlag, priceType, hedgeFlag, mOrderPrice);
	if(mFirstOrderRef == orderRef){
		mFirstInsStatus = orderStatus;
		mFirstRemainQty = remainVolume;

		if(THOST_FTDC_OST_Canceled == orderStatus){
			USTPMutexId::updateActionNum(mInsComplex);
			if(!mIsAutoFirstCancel){
				double price = mFirstMarketBasePrice + mOrderPrice;
				mFirstOrderBasePrice = mFirstMarketBasePrice;
				submitOrder(FIRST_INSTRUMENT, mFirstIns, price, mBS, remainVolume, priceType, THOST_FTDC_TC_GFD, true);
			}
			if(mIsDeleted)
				emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, false, mIsActionReferTick,
				mOrderType, 0.0, 0.0, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}else if(THOST_FTDC_OST_AllTraded == orderStatus){
			orderSecondIns(true, mOrderQty, 0.0, 0.0);
		}
	}else{
		mSecInsStatus = orderStatus;
		if(THOST_FTDC_OST_AllTraded == orderStatus){
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, true, mIsActionReferTick,
				mOrderType, 0.0, 0.0, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}
	}
}


void USTPStarePriceArbitrage::doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
											 const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
											 const QString& localId, const QString& orderRef, const QString& tradeTime)
{
	return;
}

void USTPStarePriceArbitrage::orderSecondIns(bool isInit, const int& qty, const double& bidPrice, const double& askPrice)
{	
	if(THOST_FTDC_D_Buy == mBS){
		if(isInit){
			double initPrice = mFirstOrderBasePrice - mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, initPrice, THOST_FTDC_D_Sell, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}else{
			double price = bidPrice - mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, price, THOST_FTDC_D_Sell, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}
	}else{
		if(isInit){
			double initPrice = mFirstOrderBasePrice + mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, initPrice, THOST_FTDC_D_Buy, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}else{
			double price = askPrice + mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, price, THOST_FTDC_D_Buy, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}
	}
}

void USTPStarePriceArbitrage::doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
													  const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
													  const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;

	if(mFirstIns == instrumentId)	//设置合约状态
		mFirstInsStatus = USTP_FTDC_OS_ORDER_ERROR;
	else if(mSecIns == instrumentId)
		mSecInsStatus = USTP_FTDC_OS_ORDER_ERROR;
	switch (errorId){
	case 22:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'D', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice); //重复的报单
		break;
	case 31:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'Z', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	资金不足
		break;
	case 42:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'S', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	结算结果未确认
		break;
	case 50:
	case 51:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'P', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//平仓位不足
		break;
	default:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'W', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);
		break;
	}
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderInsert] orderRef: ")) + orderRef + QString(tr("  RequestId: ")) + mReqMap[orderRef] + QString(tr("  InstrumentId: ")) + instrumentId + 
		QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPStarePriceArbitrage::doUSTPErrRtnOrderAction(const char& actionFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId,
													  const QString& orderSysId, const QString& userActionLocalId, const QString& orderActionRef, const double& orderPrice, 
													  const int& volumeChange, const int& errorId, const QString& errorMsg, const int& reqId)
{
#ifdef _DEBUG
	if(mReqMap.find(orderActionRef) == mReqMap.end())
		return;
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderAction] orderSysId: ")) + orderSysId + 
		QString(tr("  UserActionLocalId: ")) + userActionLocalId  + QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPStarePriceArbitrage::doAutoCancelFirstIns()
{	
	if(isInMarket(mFirstInsStatus)){
		mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
		submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
	}
}

void USTPStarePriceArbitrage::doAutoCancelSecIns()
{	
	if(mSecActionList.isEmpty()){
#ifdef _DEBUG
		QString data = mOrderLabel + QString(tr("  [WARN]  The list is empty when Auto cancel second instrument."));
		USTPLogger::saveData(data);
#endif
		return;
	}

	//QString orderRef = mSecActionList.takeFirst();
	//if (mSecOrderRefMap.find(orderRef) != mSecOrderRefMap.end()){
	//	if(isInMarket(mSecOrderRefMap[orderRef].status)){
	//		mSecOrderRefMap[orderRef].status = USTP_FTDC_OS_CANCEL_SUBMIT;
	//		submitAction(SECOND_INSTRUMENT, mSecOrderRefMap[orderRef].orderRef, mSecIns);
	//	}
	//}
}

void USTPStarePriceArbitrage::doDelOrder(const QString& orderStyle)
{
	if(orderStyle == mOrderLabel){
		mIsDeleted = true;
		if(isInMarket(mFirstInsStatus)){
			mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
			submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
		}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus || USTP_FTDC_OS_ORDER_ERROR == mFirstInsStatus || THOST_FTDC_OST_Canceled == mFirstInsStatus){
			emit onUpdateOrderShow(QString::number(mRequestId), mFirstIns, mOrderLabel, THOST_FTDC_OST_Canceled, mBS, 0.0, mOrderQty, mOrderQty, 0, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, false, mIsActionReferTick,
				mOrderType, 0.0, 0.0, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}
		QString data = mOrderLabel + QString(tr("  [DoDelOrder]   mFirstInsStatus: ")) + QString(mFirstInsStatus);
		USTPLogger::saveData(data);
	}
}

void USTPStarePriceArbitrage::submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument)
{	
	USTPTradeApi::reqOrderAction(USTPMutexId::getMutexId(), mBrokerId, mUserId, mInvestorId, instrument, orderLocalId, mFrontId, mSessionId);
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderAction]   OrderRef: ")) + orderLocalId + QString(tr("  InstrumentId: ")) + instrument;
	USTPLogger::saveData(data);
#endif	
}


USTPConditionArbitrage::USTPConditionArbitrage(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
											   const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, const int& actionSuperNum,
											   const double& worstSuperPrice, bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isReferTick, const double& firstMarketBidPrice, const double& firstMarketAskPrice,
											   const double& secMarketBidPrice, const double& secMarketAskPrice, USTPOrderWidget* pOrderWidget,USTPCancelWidget* pCancelWidget, 
											   USTPStrategyWidget* pStrategyWidget) : USTPStrategyBase(orderLabel, firstIns, secIns, orderPriceTick, qty, bs, offset, hedge, cancelFirstTime, cancelSecTime, cycleStall, firstPriceSlipPoint,
											   secPriceSlipPoint, isAutoFirstCancel, isAutoSecCancel, isCycle)
{	
	moveToThread(&mStrategyThread);
	mStrategyThread.start();
	mOrderType = 1;
	mRequestId = -1;
	mFirstRemainQty = mOrderQty;
	mSecondRemainQty = 0;
	mFirstBidMarketVolume = 0;
	mFirstAskMarketVolume = 0;
	mCurrentReferIndex = 0;
	mWorstSuperPrice = worstSuperPrice;
	mActionReferNum = actionReferNum;
	mActionSuperNum = actionSuperNum;
	mIsDefineOrder = isDefineOrder;
	mIsOppnentPrice = isOppentPrice;
	mIsActionReferTick = isReferTick;
	if(THOST_FTDC_D_Buy == bs){
		mFirstMarketBasePrice = firstMarketBidPrice;	
		mSecondMarketBasePrice = secMarketBidPrice;
	}else{
		mFirstMarketBasePrice = firstMarketAskPrice;	
		mSecondMarketBasePrice = secMarketAskPrice;
	}
	mFirstBidMarketPrice = firstMarketBidPrice;
	mFirstAskMarketPrice = firstMarketAskPrice;
	mSecondBidMarketPrice = secMarketBidPrice;
	mSecondAskMarketPrice = secMarketAskPrice;
	mFirstOrderPrice = 0.0;
	mSecondOrderBasePrice = 0.0;
	mFirstInsTradePrice = 0.0;
	
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mSecInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mFirstSlipPrice = firstPriceSlipPoint * mPriceTick;
	mSecondSlipPrice = secPriceSlipPoint * mPriceTick;
	mActionSuperSlipPrice = actionSuperNum * mPriceTick;
	mBrokerId = USTPCtpLoader::getBrokerId();
	mUserId = USTPMutexId::getUserId();
	mInvestorId = USTPMutexId::getInvestorId();
	mReferenceIns = USTPMutexId::getReferenceIns();
	mIsCanMarket = (USTPMutexId::getInsMarketMaxVolume(secIns) > 0) ? true : false;
	initConnect(pStrategyWidget, pOrderWidget, pCancelWidget);	
	updateInitShow();
}

USTPConditionArbitrage::~USTPConditionArbitrage()
{
	mStrategyThread.quit();
	mStrategyThread.wait();
}

void USTPConditionArbitrage::initConnect(USTPStrategyWidget* pStrategyWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget)
{
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), 
		this, SLOT(doUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)),
		this, SLOT(doUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)),
		this, SLOT(doUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pOrderWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pCancelWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(pCancelWidget, SIGNAL(onDelOrder(const QString& )), this, SLOT(doDelOrder(const QString& )), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, 
		const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, const double&)), pStrategyWidget, SLOT(doOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, 
		const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&,
		const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, 
		const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, const double&)), pCancelWidget, SLOT(doOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, 
		const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, 
		const double&)), Qt::QueuedConnection);
}

void USTPConditionArbitrage::updateInitShow()
{	
	if(mIsDeleted)	//如果手工撤单，则不更新
		return;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mRequestId = USTPMutexId::getMutexId();
	QString szReqId = QString::number(mRequestId);
	emit onUpdateOrderShow(szReqId, mFirstIns, mOrderLabel, 'N', mBS, 0.0, mFirstRemainQty, mFirstRemainQty, mFirstTradeQty, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
}

void USTPConditionArbitrage::doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
													  const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
													  const double& lowestPrice, const int& volume)
{	
	if(mIsDefineOrder && mReferenceIns == instrumentId)
		QTimer::singleShot(DEFINE_ORDER_TIME, this, SLOT(doDefineTimeOrderFirstIns()));

	if ((mFirstIns != instrumentId) &&(mSecIns != instrumentId))	//监听第一，二腿行情
		return;
	if(THOST_FTDC_D_Buy == mBS && mFirstIns == instrumentId)
		mFirstMarketBasePrice = bidPrice;
	else if(THOST_FTDC_D_Sell == mBS && mFirstIns == instrumentId)
		mFirstMarketBasePrice = askPrice;
	else if(THOST_FTDC_D_Buy == mBS && mSecIns == instrumentId)
		mSecondMarketBasePrice = bidPrice;
	else if(THOST_FTDC_D_Sell == mBS && mSecIns == instrumentId)
		mSecondMarketBasePrice = askPrice;

	if(mFirstIns == instrumentId){
		mFirstBidMarketPrice = bidPrice;
		mFirstAskMarketPrice = askPrice;
		mFirstBidMarketVolume = bidVolume;
		mFirstAskMarketVolume = askVolume;
	}else{
		mSecondBidMarketPrice = bidPrice;
		mSecondAskMarketPrice = askPrice;
	}
	if((mFirstMarketBasePrice < VALUE_ACCURACY) || (mSecondMarketBasePrice < VALUE_ACCURACY) || (mFirstBidMarketPrice < VALUE_ACCURACY) || (mFirstAskMarketPrice < VALUE_ACCURACY))	
		return;
	if((mFirstMarketBasePrice > INIT_VALUE) || (mSecondMarketBasePrice > INIT_VALUE) || (mFirstBidMarketPrice > INIT_VALUE) || (mFirstAskMarketPrice > INIT_VALUE))
		return;
	double fSecBasePrice = mSecondMarketBasePrice + mOrderPrice;
	if(mIsOppnentPrice)
		opponentPriceOrder(instrumentId, fSecBasePrice);
	else
		noOpponentPriceOrder(instrumentId, fSecBasePrice);
}

void USTPConditionArbitrage::opponentPriceOrder(const QString& instrument, const double& basePrice)
{
	if(((USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus) || (THOST_FTDC_OST_Canceled == mFirstInsStatus))){//第一腿没有下单或者已撤单
		if(THOST_FTDC_D_Buy == mBS && mFirstAskMarketPrice <= basePrice){	//买委托满足下单条件
			mFirstOrderPrice = mFirstAskMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else if(THOST_FTDC_D_Sell == mBS && mFirstBidMarketPrice >= basePrice){//卖委托满足下单条件
			mFirstOrderPrice = mFirstBidMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
		}else{
			if(THOST_FTDC_OST_Canceled == mFirstInsStatus){		
				updateInitShow();
			}
		}
	}else if(USTPStrategyBase::isInMarket(mFirstInsStatus)){
		double actionBidBasePrice = basePrice + mActionSuperSlipPrice;
		double actionAskBasePrice = basePrice - mActionSuperSlipPrice;

		if((mSecIns == instrument) && ((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice > actionBidBasePrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice < actionAskBasePrice)))){
			cancelFirstIns();
		}else if((mSecIns == instrument) && ((THOST_FTDC_D_Buy == mBS && (mFirstAskMarketPrice <= (basePrice + VALUE_ACCURACY))) || (THOST_FTDC_D_Sell == mBS && 
			(mFirstBidMarketPrice >= (basePrice - VALUE_ACCURACY))))){
				cancelFirstIns();
		}else if((THOST_FTDC_D_Buy == mBS && (mFirstAskMarketPrice > mFirstOrderPrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstBidMarketPrice < mFirstOrderPrice))){
			cancelFirstIns();
		}
	}else if((THOST_FTDC_OST_Canceled == mSecInsStatus) && (THOST_FTDC_OST_AllTraded == mFirstInsStatus)){
		orderSecondIns(false, mSecondRemainQty, 0.0, 0.0);
	}else if(USTPStrategyBase::isInMarket(mSecInsStatus) && (THOST_FTDC_OST_AllTraded == mFirstInsStatus)){
		double calAskValue = mSecondAskMarketPrice - mSecondSlipPrice + VALUE_ACCURACY;
		double calBidValue = mSecondBidMarketPrice + mSecondSlipPrice - VALUE_ACCURACY;
		if((THOST_FTDC_D_Buy == mBS && (mSecondOrderBasePrice > calAskValue)) || (THOST_FTDC_D_Sell == mBS && (mSecondOrderBasePrice < calBidValue))){
			cancelSecIns();
		}
	}
}

void USTPConditionArbitrage::noOpponentPriceOrder(const QString& instrument, const double& basePrice)
{
	if(((USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus) || (THOST_FTDC_OST_Canceled == mFirstInsStatus))){//第一腿没有下单或者已撤单
		
		if(THOST_FTDC_D_Buy == mBS && mFirstMarketBasePrice <= basePrice){	//条件单买委托满足下单条件
			mFirstOrderPrice = mFirstMarketBasePrice + mFirstSlipPrice;
			if(mFirstOrderPrice > basePrice)
				mFirstOrderPrice = basePrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else if(THOST_FTDC_D_Sell == mBS && mFirstMarketBasePrice >= basePrice){//条件单卖委托满足下单条件
			mFirstOrderPrice = mFirstMarketBasePrice - mFirstSlipPrice;
			if(mFirstOrderPrice < basePrice)
				mFirstOrderPrice = basePrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
			
		}else{
			if(THOST_FTDC_OST_Canceled == mFirstInsStatus){
				updateInitShow();
			}
		}
	}else if(USTPStrategyBase::isInMarket(mFirstInsStatus)){//1.非设定时间撤单的情况，第一腿委托成功，第一腿行情发生变化，腿一撤单重发;2.设定时间撤单的情况，定时超时，根据行情触发。
		if(mIsActionReferTick && (mFirstIns == instrument)){
			if(THOST_FTDC_D_Buy == mBS){
				if(mFirstMarketBasePrice > mFirstOrderPrice)
					mCurrentReferIndex++;
				else
					mCurrentReferIndex = 0;
			}else{
				if(mFirstMarketBasePrice < mFirstOrderPrice)
					mCurrentReferIndex++;
				else
					mCurrentReferIndex = 0;
			}
		}else if(mIsActionReferTick && (mSecIns == instrument)){
			if(mCurrentReferIndex > 0)
				mCurrentReferIndex++;
		}
		double actionBidBasePrice = basePrice + mActionSuperSlipPrice;
		double actionAskBasePrice = basePrice - mActionSuperSlipPrice;

#ifdef _DEBUG
		QString data = tr("[") + mOrderLabel + tr("-RealData]   Instrument: ") + instrument +  QString(tr("  FirstOrderPrice: ")) + QString::number(mFirstOrderPrice) + QString(tr("  ActionBidBasePrice: ")) +
			QString::number(actionBidBasePrice)  + QString(tr("  ActionAskBasePrice: ")) + QString::number(actionAskBasePrice) + QString(tr("  FirstMarketBasePrice: ")) + QString::number(mFirstMarketBasePrice) + 
			QString(tr("  BasePrice: ")) + QString::number(basePrice) + QString(tr("  CurrentReferIndex: ")) + QString::number(mCurrentReferIndex) + QString(tr("  ActionReferNum: ")) + QString::number(mActionReferNum);
		USTPLogger::saveData(data);
#endif

		if((mSecIns == instrument) && ((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice - VALUE_ACCURACY > actionBidBasePrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice + VALUE_ACCURACY < actionAskBasePrice)))){
			cancelFirstIns();
		}else if((THOST_FTDC_D_Buy == mBS && (mFirstMarketBasePrice > mFirstOrderPrice + VALUE_ACCURACY)) || (THOST_FTDC_D_Sell == mBS && (mFirstMarketBasePrice < mFirstOrderPrice - VALUE_ACCURACY))){
				cancelFirstIns();
		}else if((THOST_FTDC_D_Buy == mBS && (mIsActionReferTick && (mCurrentReferIndex >= mActionReferNum)) && (mFirstMarketBasePrice < basePrice)) || 
			(THOST_FTDC_D_Sell == mBS && (mIsActionReferTick && (mCurrentReferIndex >= mActionReferNum)) && (mFirstMarketBasePrice > basePrice))){
				cancelFirstIns();
		}else if((mFirstMarketBasePrice == mFirstOrderPrice) && ((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice < basePrice - VALUE_ACCURACY) && (mFirstRemainQty < mFirstBidMarketVolume)) || 
			(THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice > basePrice + VALUE_ACCURACY) && (mFirstRemainQty < mFirstAskMarketVolume)))){
				if (THOST_FTDC_D_Buy == mBS)
					mFirstMarketBasePrice += mPriceTick;
				else
					mFirstMarketBasePrice -= mPriceTick;	
				cancelFirstIns();
		}

	}else if((THOST_FTDC_OST_Canceled == mSecInsStatus) && (THOST_FTDC_OST_AllTraded == mFirstInsStatus)){
		orderSecondIns(false, mSecondRemainQty, 0.0, 0.0);
	}else if(USTPStrategyBase::isInMarket(mSecInsStatus) && (THOST_FTDC_OST_AllTraded == mFirstInsStatus)){
		double calAskValue = mSecondAskMarketPrice  + VALUE_ACCURACY;
		double calBidValue = mSecondBidMarketPrice - VALUE_ACCURACY;
		if((THOST_FTDC_D_Buy == mBS && (mSecondOrderBasePrice > calAskValue)) || (THOST_FTDC_D_Sell == mBS && (mSecondOrderBasePrice < calBidValue))){
			cancelSecIns();
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-@@Cancel]   Instrument: ") + instrument +  QString(tr("  mSecondOrderBasePrice: ")) + QString::number(mSecondOrderBasePrice) + QString(tr("  CalBidValue: ")) +
				QString::number(calBidValue)  + QString(tr("  CalAskValue: ")) + QString::number(calAskValue) + QString(tr("  BS: ")) + QString::number(mBS);
			USTPLogger::saveData(data);
#endif
		}
	}
}

void USTPConditionArbitrage::defineTimeOrder(const double& basePrice)
{
	if(((USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus) || (THOST_FTDC_OST_Canceled == mFirstInsStatus))){//第一腿没有下单或者已撤单
		if(mIsOppnentPrice){
			if(THOST_FTDC_D_Buy == mBS && mFirstAskMarketPrice <= basePrice){	//对价买委托满足下单条件
				mFirstOrderPrice = mFirstAskMarketPrice;
				mSecondOrderBasePrice = mSecondMarketBasePrice;
				switchFirstInsOrder(THOST_FTDC_TC_GFD);

			}else if(THOST_FTDC_D_Sell == mBS && mFirstBidMarketPrice >= basePrice){//对价卖委托满足下单条件
				mFirstOrderPrice = mFirstBidMarketPrice;
				mSecondOrderBasePrice = mSecondMarketBasePrice;
				switchFirstInsOrder(THOST_FTDC_TC_GFD);
			}else{
				if(THOST_FTDC_OST_Canceled == mFirstInsStatus){		
					updateInitShow();
				}
			}
		}else{
			if(THOST_FTDC_D_Buy == mBS && mFirstMarketBasePrice <= basePrice){	//条件单买委托满足下单条件
				mFirstOrderPrice = mFirstMarketBasePrice + mFirstSlipPrice;
				if(mFirstOrderPrice > basePrice)
					mFirstOrderPrice = basePrice;
				mSecondOrderBasePrice = mSecondMarketBasePrice;
				switchFirstInsOrder(THOST_FTDC_TC_GFD);

			}else if(THOST_FTDC_D_Sell == mBS && mFirstMarketBasePrice >= basePrice){//条件单卖委托满足下单条件
				mFirstOrderPrice = mFirstMarketBasePrice - mFirstSlipPrice;
				if(mFirstOrderPrice < basePrice)
					mFirstOrderPrice = basePrice;
				mSecondOrderBasePrice = mSecondMarketBasePrice;
				switchFirstInsOrder(THOST_FTDC_TC_GFD);
			}else{
				if(THOST_FTDC_OST_Canceled == mFirstInsStatus){		
					updateInitShow();
				}
			}
		}
	}
}

void USTPConditionArbitrage::switchFirstInsOrder(const char& tCondition)
{
	if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus){
		if(mIsDeleted)
			return;
		orderInsert(mRequestId, FIRST_INSTRUMENT, mFirstIns, mFirstOrderPrice,  mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, tCondition, true);
	}else
		submitOrder(FIRST_INSTRUMENT, mFirstIns, mFirstOrderPrice, mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, tCondition, true);
}

void USTPConditionArbitrage::submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType,
										 const char& timeCondition, bool isFirstIns)
{	
	if(isFirstIns && mIsDeleted)	//撤掉报单，合约一禁止下新单
		return;
	mRequestId = USTPMutexId::getMutexId();
	orderInsert(mRequestId, insLabel, instrument, orderPrice,  direction, qty, priceType, timeCondition, isFirstIns);	
}

void USTPConditionArbitrage::orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType,
										 const char& timeCondition, bool isFirstIns)
{	
	if(USTPMutexId::getActionNum(mInsComplex) > MAX_CANCEL_NUM)
		return;
	double adjustPrice = (priceType == THOST_FTDC_OPT_LimitPrice) ? orderPrice : 0.0;
	QString orderRef;
	if(isFirstIns){
		mCurrentReferIndex = 0;
		mFirstInsStatus = USTP_FTDC_OS_ORDER_SUBMIT;
		USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
		mFirstOrderRef = orderRef;
	}else{
		mSecInsStatus = USTP_FTDC_OS_ORDER_SUBMIT;
		USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
		mSecOrderRef = orderRef;
	}
	QString szReqId = QString::number(reqId);
	mReqMap.insert(orderRef, szReqId);
	emit onUpdateOrderShow(szReqId, instrument, mOrderLabel, 'N', direction, 0.0, qty, qty, 0, mOffsetFlag, priceType, mHedgeFlag, mOrderPrice);

	if(mIsAutoFirstCancel && isFirstIns)
		QTimer::singleShot(mFirstCancelTime, this, SLOT(doAutoCancelFirstIns()));
	else if(mIsAutoSecondCancel && !isFirstIns){
		mSecActionList.append(orderRef);
		QTimer::singleShot(mSecondCancelTime, this, SLOT(doAutoCancelSecIns()));
	}
	//条件日志
#ifdef _DEBUG
	int nIsSecCancel = mIsAutoSecondCancel ? 1 : 0;
	int nIsSecIns = isFirstIns ? 0 : 1;
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderInsert]   Instrument: ")) + instrument +  QString(tr("  RequestId: ")) + szReqId + QString(tr("  OrderRef: ")) + orderRef + 
		QString(tr("  UserId: ")) + mUserId + QString(tr("  PriceType: ")) + QString(priceType) + QString(tr("  OrderPrice: ")) + QString::number(adjustPrice) + QString(tr("  OrderVolume: ")) + 
		QString::number(qty) + QString(tr("  Direction: ")) + QString(direction) + QString(tr("  SecAutoCancel: ")) + QString::number(nIsSecCancel) + QString(tr("  IsSecIns: ")) + QString::number(nIsSecIns);
	USTPLogger::saveData(data);
#endif	
}

void USTPConditionArbitrage::doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
											const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
											const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId)
{	
	QString data;
#ifdef _DEBUG
	data = tr("[") + mOrderLabel + tr("-@@USTPRtnOrder1 ]   Instrument: ") + instrumentId +  QString(tr("  OrderStatus: ")) + QString(orderStatus);
	USTPLogger::saveData(data);
#endif
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
#ifdef _DEBUG
	data = tr("[") + mOrderLabel + tr("-@@USTPRtnOrder2 ]   Instrument: ") + instrumentId +  QString(tr("  OrderStatus: ")) + QString(orderStatus);
	USTPLogger::saveData(data);
#endif
	emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, orderStatus, direction, orderPrice, orderVolume, remainVolume, tradeVolume, offsetFlag, priceType, hedgeFlag, mOrderPrice);
	if(mFirstOrderRef == orderRef){
#ifdef _DEBUG
		data = tr("[") + mOrderLabel + tr("-@@USTPRtnOrder3 ]   Instrument: ") + instrumentId +  QString(tr("  OrderStatus: ")) + QString(orderStatus);
		USTPLogger::saveData(data);
#endif
		mFirstInsStatus = orderStatus;
		mFirstRemainQty = remainVolume;

		if(THOST_FTDC_OST_Canceled == orderStatus){
			USTPMutexId::updateActionNum(mInsComplex);
			if(!mIsAutoFirstCancel){
				double fSecBasePrice = mSecondMarketBasePrice + mOrderPrice;
				defineTimeOrder(fSecBasePrice);
			}
			if(mIsDeleted)
				emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, mIsOppnentPrice, mIsDefineOrder, false,
				mIsActionReferTick, mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}else if(THOST_FTDC_OST_AllTraded == orderStatus){
			orderSecondIns(true, mOrderQty, 0.0, 0.0);
#ifdef _DEBUG
			data = tr("[") + mOrderLabel + tr("-@@USTPRtnOrder4 ]   Instrument: ") + instrumentId +  QString(tr("  OrderStatus: ")) + QString(orderStatus);
			USTPLogger::saveData(data);
#endif
		}
	}else{
		mSecInsStatus = orderStatus;
		if(THOST_FTDC_OST_AllTraded == orderStatus){
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, true, mIsActionReferTick,
				mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
#ifdef _DEBUG
			data = tr("[") + mOrderLabel + tr("-@@USTPRtnOrder5 ]   Instrument: ") + instrumentId +  QString(tr("  OrderStatus: ")) + QString(orderStatus);
			USTPLogger::saveData(data);
#endif
		}
	}
}


void USTPConditionArbitrage::doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
											const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
											const QString& localId, const QString& orderRef, const QString& tradeTime)
{
	return;
}

void USTPConditionArbitrage::orderSecondIns(bool isInit, const int& qty, const double& bidPrice, const double& askPrice)
{	
	if(THOST_FTDC_D_Buy == mBS){
		mSecondOrderBasePrice = mSecondAskMarketPrice - mSecondSlipPrice;
		submitOrder(SECOND_INSTRUMENT, mSecIns, mSecondOrderBasePrice, THOST_FTDC_D_Sell, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
#ifdef _DEBUG
		QString data = tr("[") + mOrderLabel + tr("-@@Ask ]   Instrument: ") + mSecIns +  QString(tr("  mSecondOrderBasePrice: ")) + QString::number(mSecondOrderBasePrice);
		USTPLogger::saveData(data);
#endif
	}else{
		mSecondOrderBasePrice = mSecondBidMarketPrice + mSecondSlipPrice;
		submitOrder(SECOND_INSTRUMENT, mSecIns, mSecondOrderBasePrice, THOST_FTDC_D_Buy, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
#ifdef _DEBUG
		QString data = tr("[") + mOrderLabel + tr("-@@ABid ]   Instrument: ") + mSecIns +  QString(tr("  mSecondOrderBasePrice: ")) + QString::number(mSecondOrderBasePrice);
		USTPLogger::saveData(data);
#endif
	}
}

void USTPConditionArbitrage::doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
													 const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
													 const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
	if(mFirstIns == instrumentId)	//设置合约状态
		mFirstInsStatus = USTP_FTDC_OS_ORDER_ERROR;
	else if(mSecIns == instrumentId)
		mSecInsStatus = USTP_FTDC_OS_ORDER_ERROR;
	switch (errorId){
	case 22:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'D', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice); //重复的报单
		break;
	case 31:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'Z', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	资金不足
		break;
	case 42:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'S', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	结算结果未确认
		break;
	case 50:
	case 51:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'P', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//平仓位不足
		break;
	default:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'W', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);
		break;
	}
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderInsert] orderRef: ")) + orderRef + QString(tr("  RequestId: ")) + mReqMap[orderRef] + QString(tr("  InstrumentId: ")) + instrumentId + 
		QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPConditionArbitrage::doUSTPErrRtnOrderAction(const char& actionFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId,
													 const QString& orderSysId, const QString& userActionLocalId, const QString& orderActionRef, const double& orderPrice, 
													 const int& volumeChange, const int& errorId, const QString& errorMsg, const int& reqId)
{
#ifdef _DEBUG
	if(mReqMap.find(orderActionRef) == mReqMap.end())
		return;
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderAction] orderSysId: ")) + orderSysId + 
		QString(tr("  UserActionLocalId: ")) + userActionLocalId  + QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPConditionArbitrage::doAutoCancelFirstIns()
{	
	if(isInMarket(mFirstInsStatus))
		cancelFirstIns();
}

void USTPConditionArbitrage::cancelFirstIns()
{
	mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
	submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
}

void USTPConditionArbitrage::doAutoCancelSecIns()
{	
	//if(isInMarket(mSecInsStatus))
	//	cancelSecIns();
	return;
}

void USTPConditionArbitrage::cancelSecIns()
{
	/*mSecInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
	submitAction(SECOND_INSTRUMENT, mSecOrderRef, mSecIns);*/
	return;
}

void USTPConditionArbitrage::doDefineTimeOrderFirstIns()
{
	if((mFirstMarketBasePrice < VALUE_ACCURACY) || (mSecondMarketBasePrice < VALUE_ACCURACY) || (mFirstBidMarketPrice < VALUE_ACCURACY) || (mFirstAskMarketPrice < VALUE_ACCURACY))	//保证两腿都收到行情
		return;
	if((mFirstMarketBasePrice > INIT_VALUE) || (mSecondMarketBasePrice > INIT_VALUE) || (mFirstBidMarketPrice > INIT_VALUE) || (mFirstAskMarketPrice > INIT_VALUE))
		return;
	double fSecBasePrice = mSecondMarketBasePrice + mOrderPrice;
	if(mIsOppnentPrice)
		opponentPriceOrder(mFirstIns, fSecBasePrice);
	else
		defineTimeOrder(fSecBasePrice);
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [DefineTimeOrder] InstrumentId: ")) + mFirstIns + QString(tr("  mSecondMarketBasePrice: ")) + QString::number(fSecBasePrice) +
		 QString(tr("  ReferenceIns: ")) + mReferenceIns;
	USTPLogger::saveData(data);
#endif
}

void USTPConditionArbitrage::doDelOrder(const QString& orderStyle)
{
	if(orderStyle == mOrderLabel){
		mIsDeleted = true;
		if(isInMarket(mFirstInsStatus)){
			mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
			submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
		}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus || USTP_FTDC_OS_ORDER_ERROR == mFirstInsStatus || THOST_FTDC_OST_Canceled == mFirstInsStatus){
			emit onUpdateOrderShow(QString::number(mRequestId), mFirstIns, mOrderLabel, THOST_FTDC_OST_Canceled, mBS, 0.0, mOrderQty, mOrderQty, 0, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, mWorstSuperPrice, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, false, mIsActionReferTick,
				mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}else if(isInMarket(mSecInsStatus)){
			mSecInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
			submitAction(SECOND_INSTRUMENT, mSecOrderRef, mSecIns);
		}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mSecInsStatus || USTP_FTDC_OS_ORDER_ERROR == mSecInsStatus || THOST_FTDC_OST_Canceled == mSecInsStatus){
			emit onUpdateOrderShow(QString::number(mRequestId), mSecIns, mOrderLabel, THOST_FTDC_OST_Canceled, mBS, 0.0, mOrderQty, mOrderQty, 0, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, mWorstSuperPrice, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, false, mIsActionReferTick,
				mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}
		QString data = mOrderLabel + tr("  [DoDelOrder]   mFirstInsStatus: ") + QString(mFirstInsStatus) + tr("   mSecInsStatus: ") + QString(mSecInsStatus);
		USTPLogger::saveData(data);
	}
}

void USTPConditionArbitrage::submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument)
{	
	USTPTradeApi::reqOrderAction(USTPMutexId::getMutexId(), mBrokerId, mUserId, mInvestorId, instrument, orderLocalId, mFrontId, mSessionId);
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderAction]   UserLocalOrderId: ")) + orderLocalId + QString(tr("  InstrumentId: ")) + instrument;
	USTPLogger::saveData(data);
#endif	
}


USTPOpponentArbitrage::USTPOpponentArbitrage(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
											 const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
											 const int& actionSuperNum, bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isReferTick, const double& firstMarketBidPrice,
											 const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget,
											 USTPStrategyWidget* pStrategyWidget) :USTPStrategyBase(orderLabel, firstIns, secIns, orderPriceTick, qty, bs, offset, hedge, cancelFirstTime, cancelSecTime, cycleStall, firstPriceSlipPoint,
											 secPriceSlipPoint, isAutoFirstCancel, isAutoSecCancel, isCycle)
{	
	moveToThread(&mStrategyThread);
	mStrategyThread.start();
	mOrderType = 2;
	mRequestId = -1;
	mFirstTradeQty = 0;
	mFirstRemainQty = mOrderQty;
	mSecondTradeQty = 0;
	mFirstBidMarketVolume = 0;
	mFirstAskMarketVolume = 0;
	mCurrentReferIndex = 0;
	mActionReferNum = actionReferNum;
	mActionSuperNum = actionSuperNum;
	mIsDefineOrder = isDefineOrder;
	mIsOppnentPrice = isOppentPrice;
	mIsActionReferTick = isReferTick;
	if(THOST_FTDC_D_Buy == bs){
		mFirstMarketBasePrice = firstMarketBidPrice;	
		mSecondMarketBasePrice = secMarketBidPrice;
	}else{
		mFirstMarketBasePrice = firstMarketAskPrice;	
		mSecondMarketBasePrice = secMarketAskPrice;
	}
	mFirstBidMarketPrice = firstMarketBidPrice;
	mFirstAskMarketPrice = firstMarketAskPrice;
	mSecondBidMarketPrice = secMarketBidPrice;
	mSecondAskMarketPrice = secMarketAskPrice;
	mFirstOrderPrice = 0.0;
	mSecondOrderBasePrice = 0.0;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mSecInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mFirstSlipPrice = firstPriceSlipPoint * mPriceTick;
	mSecondSlipPrice = secPriceSlipPoint * mPriceTick;
	mActionSuperSlipPrice = actionSuperNum * mPriceTick;
	mBrokerId = USTPCtpLoader::getBrokerId();
	mUserId = USTPMutexId::getUserId();
	mInvestorId = USTPMutexId::getInvestorId();
	mReferenceIns = USTPMutexId::getReferenceIns();
	mIsCanMarket = (USTPMutexId::getInsMarketMaxVolume(secIns) > 0) ? true : false;
	initConnect(pStrategyWidget, pOrderWidget, pCancelWidget);	
	updateInitShow();
}

USTPOpponentArbitrage::~USTPOpponentArbitrage()
{
	mStrategyThread.quit();
	mStrategyThread.wait();
}

void USTPOpponentArbitrage::initConnect(USTPStrategyWidget* pStrategyWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget)
{
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), 
		this, SLOT(doUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)),
		this, SLOT(doUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)),
		this, SLOT(doUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pOrderWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pCancelWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(pCancelWidget, SIGNAL(onDelOrder(const QString& )), this, SLOT(doDelOrder(const QString& )), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, 
		const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, const double&)), pStrategyWidget, SLOT(doOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, 
		const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&,
		const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, 
		const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, const double&)), pCancelWidget, SLOT(doOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, 
		const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, 
		const double&)), Qt::QueuedConnection);
}

void USTPOpponentArbitrage::updateInitShow()
{	
	if(mIsDeleted)	//如果手工撤单，则不更新
		return;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mRequestId = USTPMutexId::getMutexId();
	QString szReqId = QString::number(mRequestId);
	emit onUpdateOrderShow(szReqId, mFirstIns, mOrderLabel, 'N', mBS, 0.0, mFirstRemainQty, mFirstRemainQty, mFirstTradeQty, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
}

void USTPOpponentArbitrage::doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
													 const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
													 const double& lowestPrice, const int& volume)
{	
	if(mIsDefineOrder && mReferenceIns == instrumentId)
		QTimer::singleShot(DEFINE_ORDER_TIME, this, SLOT(doDefineTimeOrderFirstIns()));

	if ((mFirstIns != instrumentId) &&(mSecIns != instrumentId))	//监听第一，二腿行情
		return;
	if(THOST_FTDC_D_Buy == mBS && mFirstIns == instrumentId)
		mFirstMarketBasePrice = bidPrice;
	else if(THOST_FTDC_D_Sell == mBS && mFirstIns == instrumentId)
		mFirstMarketBasePrice = askPrice;
	else if(THOST_FTDC_D_Buy == mBS && mSecIns == instrumentId)
		mSecondMarketBasePrice = bidPrice;
	else if(THOST_FTDC_D_Sell == mBS && mSecIns == instrumentId)
		mSecondMarketBasePrice = askPrice;
	if(mFirstIns == instrumentId){
		mFirstBidMarketPrice = bidPrice;
		mFirstAskMarketPrice = askPrice;
		mFirstBidMarketVolume = bidVolume;
		mFirstAskMarketVolume = askVolume;
	}else{
		mSecondBidMarketPrice = bidPrice;
		mSecondAskMarketPrice = askPrice;
	}

	if((mFirstMarketBasePrice < VALUE_ACCURACY) || (mSecondMarketBasePrice < VALUE_ACCURACY) || (mFirstBidMarketPrice < VALUE_ACCURACY) || (mFirstAskMarketPrice < VALUE_ACCURACY))	//保证两腿都收到行情
		return;
	if((mFirstMarketBasePrice > INIT_VALUE) || (mSecondMarketBasePrice > INIT_VALUE) || (mFirstBidMarketPrice > INIT_VALUE) || (mFirstAskMarketPrice > INIT_VALUE))
		return;
	double fSecBasePrice = mSecondMarketBasePrice + mOrderPrice;
	if(mIsOppnentPrice)
		opponentPriceOrder(instrumentId, fSecBasePrice);
	else
		noOpponentPriceOrder(instrumentId, fSecBasePrice);
}

void USTPOpponentArbitrage::opponentPriceOrder(const QString& instrument, const double& basePrice)
{
	if(((USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus) || (THOST_FTDC_OST_Canceled == mFirstInsStatus))){//第一腿没有下单或者已撤单
		if(THOST_FTDC_D_Buy == mBS && mFirstAskMarketPrice <= (basePrice + VALUE_ACCURACY)){	//买委托满足下单条件
			mFirstOrderPrice = mFirstAskMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else if(THOST_FTDC_D_Sell == mBS && mFirstBidMarketPrice >= (basePrice - VALUE_ACCURACY)){//卖委托满足下单条件
			mFirstOrderPrice = mFirstBidMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
		}else{
			if(THOST_FTDC_OST_Canceled == mFirstInsStatus){
				updateInitShow();
			}
		}
	}else if(USTPStrategyBase::isInMarket(mFirstInsStatus)){
		double actionBidBasePrice = basePrice + mActionSuperSlipPrice;
		double actionAskBasePrice = basePrice - mActionSuperSlipPrice;
		if((mSecIns == instrument) && ((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice > actionBidBasePrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice < actionAskBasePrice)))){
			cancelFirstIns();
		}else if((mSecIns == instrument) && ((THOST_FTDC_D_Buy == mBS && (mFirstAskMarketPrice <= (basePrice + VALUE_ACCURACY))) || (THOST_FTDC_D_Sell == mBS && 
			(mFirstBidMarketPrice >= (basePrice - VALUE_ACCURACY))))){
			cancelFirstIns();
		}else if((THOST_FTDC_D_Buy == mBS && (mFirstAskMarketPrice > mFirstOrderPrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstBidMarketPrice < mFirstOrderPrice))){
			cancelFirstIns();
		}
	}
}

void USTPOpponentArbitrage::noOpponentPriceOrder(const QString& instrument, const double& basePrice)
{
	if(((USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus) || (THOST_FTDC_OST_Canceled == mFirstInsStatus))){//第一腿没有下单或者已撤单

		if(THOST_FTDC_D_Buy == mBS && mFirstAskMarketPrice < (basePrice + VALUE_ACCURACY)){	//对价单买委托满足下单条件
			mFirstOrderPrice = mFirstAskMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-NoOpponentPriceOrder #1]   FirstAskMarketPrice: ") + QString::number(mFirstAskMarketPrice) + tr("  BasePrice: ") +
				QString::number(basePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif

		}else if(THOST_FTDC_D_Sell == mBS && mFirstBidMarketPrice > (basePrice - VALUE_ACCURACY)){//对价单卖委托满足下单条件
			mFirstOrderPrice = mFirstBidMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-NoOpponentPriceOrder #2]   FirstBidMarketPrice: ") + QString::number(mFirstBidMarketPrice) + tr("  BasePrice: ") +
				QString::number(basePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif

		}else if(THOST_FTDC_D_Buy == mBS && mFirstMarketBasePrice < basePrice - VALUE_ACCURACY){	//条件单买委托满足下单条件
			mFirstOrderPrice = mFirstMarketBasePrice + mFirstSlipPrice;
			if(mFirstOrderPrice > basePrice)
				mFirstOrderPrice = basePrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-NoOpponentPriceOrder #3]   FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + tr("  BasePrice: ") +
				QString::number(basePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif

		}else if(THOST_FTDC_D_Sell == mBS && mFirstMarketBasePrice > basePrice + VALUE_ACCURACY){//条件单卖委托满足下单条件
			mFirstOrderPrice = mFirstMarketBasePrice - mFirstSlipPrice;
			if(mFirstOrderPrice < basePrice)
				mFirstOrderPrice = basePrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-NoOpponentPriceOrder #4]   FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + tr("  BasePrice: ") +
				QString::number(basePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif

		}else{
			if(THOST_FTDC_OST_Canceled == mFirstInsStatus){
				updateInitShow();
			}
		}
	}else if(USTPStrategyBase::isInMarket(mFirstInsStatus)){//1.非设定时间撤单的情况，第一腿委托成功，第一腿行情发生变化，腿一撤单重发;2.设定时间撤单的情况，定时超时，根据行情触发。

		double actionBidBasePrice = basePrice + mActionSuperSlipPrice;
		double actionAskBasePrice = basePrice - mActionSuperSlipPrice;
#ifdef _DEBUG
		QString data = tr("[") + mOrderLabel + tr("-RealData]   Instrument: ") + instrument +  tr("  FirstOrderPrice: ") + QString::number(mFirstOrderPrice) + tr("  ActionBidBasePrice: ") +
			QString::number(actionBidBasePrice)  + tr("  ActionAskBasePrice: ") + QString::number(actionAskBasePrice) + tr("  FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + 
			tr("  BasePrice: ") + QString::number(basePrice) + tr("  FirstAskMarketPrice: ") + QString::number(mFirstAskMarketPrice) + tr("  FirstBidMarketPrice: ") + QString::number(mFirstBidMarketPrice) + 
			tr("  SecBidMarketPrice: ") + QString::number(mSecondBidMarketPrice) + tr("  SecondMarketBasePrice: ") + QString::number(mSecondMarketBasePrice) + tr("  CurrentReferIndex: ") +
			QString::number(mCurrentReferIndex) + tr("  ActionReferNum: ") + QString::number(mActionReferNum) + tr("  BS: ") + QString(mBS);
		USTPLogger::saveData(data);
#endif
		if((mSecIns == instrument) && ((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice > actionBidBasePrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice < actionAskBasePrice)))){
			cancelFirstIns();
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-Cancel#1]   Instrument: ") + instrument +  tr("  FirstOrderPrice: ") + QString::number(mFirstOrderPrice) + tr("  ActionBidBasePrice: ") +
				QString::number(actionBidBasePrice)  + tr("  ActionAskBasePrice: ") + QString::number(actionAskBasePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif
		}else if((THOST_FTDC_D_Buy == mBS && (mFirstAskMarketPrice <= (basePrice + VALUE_ACCURACY))) || (THOST_FTDC_D_Sell == mBS && (mFirstBidMarketPrice >= (basePrice - VALUE_ACCURACY)))){
			cancelFirstIns();
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-Cancel#2]   Instrument: ") + instrument +  tr("  BasePrice: ") + QString::number(basePrice) + tr("  FirstAskMarketPrice: ") +
				QString::number(mFirstAskMarketPrice)  + tr("  FirstBidMarketPrice: ") + QString::number(mFirstBidMarketPrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif
		}else if((THOST_FTDC_D_Buy == mBS && (mFirstMarketBasePrice - VALUE_ACCURACY > mFirstOrderPrice)&& (mFirstMarketBasePrice < basePrice)) || 
			(THOST_FTDC_D_Sell == mBS && (mFirstMarketBasePrice  + VALUE_ACCURACY < mFirstOrderPrice)&& (mFirstMarketBasePrice > basePrice))){
				cancelFirstIns();
#ifdef _DEBUG
				QString data = tr("[") + mOrderLabel + tr("-Cancel#3]   Instrument: ") + instrument +  tr("  BasePrice: ") + QString::number(basePrice) + tr("  FirstMarketBasePrice: ") +
					QString::number(mFirstMarketBasePrice)  + tr("  FirstOrderPrice: ") + QString::number(mFirstOrderPrice) + tr("  BS: ") + QString(mBS);
				USTPLogger::saveData(data);
#endif
		}
	}
}

void USTPOpponentArbitrage::defineTimeOrder(const double& basePrice)
{
	if(((USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus) || (THOST_FTDC_OST_Canceled == mFirstInsStatus))){//第一腿没有下单或者已撤单
		if(THOST_FTDC_D_Buy == mBS && mFirstAskMarketPrice <= (basePrice + VALUE_ACCURACY)){	//对价单买委托满足下单条件
			mFirstOrderPrice = mFirstAskMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-TimeOrder #1]   FirstAskMarketPrice: ") + QString::number(mFirstAskMarketPrice) + tr("  BasePrice: ") +
				QString::number(basePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif

		}else if(THOST_FTDC_D_Sell == mBS && mFirstBidMarketPrice >= (basePrice - VALUE_ACCURACY)){//对价单卖委托满足下单条件
			mFirstOrderPrice = mFirstBidMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-TimeOrder #2]   FirstBidMarketPrice: ") + QString::number(mFirstBidMarketPrice) + tr("  BasePrice: ") +
				QString::number(basePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif

		}else if(THOST_FTDC_D_Buy == mBS && mFirstMarketBasePrice < (basePrice - VALUE_ACCURACY)){	//条件单买委托满足下单条件
			mFirstOrderPrice = mFirstMarketBasePrice + mFirstSlipPrice;
			if(mFirstOrderPrice > basePrice)
				mFirstOrderPrice = basePrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-TimeOrder #3]   FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + tr("  BasePrice: ") +
				QString::number(basePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif

		}else if(THOST_FTDC_D_Sell == mBS && mFirstMarketBasePrice > (basePrice + VALUE_ACCURACY)){//条件单卖委托满足下单条件
			mFirstOrderPrice = mFirstMarketBasePrice - mFirstSlipPrice;
			if(mFirstOrderPrice < basePrice)
				mFirstOrderPrice = basePrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-TimeOrder #4]   FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + tr("  BasePrice: ") +
				QString::number(basePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif

		}else{
			if(THOST_FTDC_OST_Canceled == mFirstInsStatus){
				updateInitShow();
			}
		}
	}
}

void USTPOpponentArbitrage::switchFirstInsOrder(const char& tCondition)
{
	if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus){
		if(mIsDeleted)
			return;
		orderInsert(mRequestId, FIRST_INSTRUMENT, mFirstIns, mFirstOrderPrice, mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, tCondition, true);
	}
	else
		submitOrder(FIRST_INSTRUMENT, mFirstIns, mFirstOrderPrice, mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, tCondition, true);
}

void USTPOpponentArbitrage::submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	if(isFirstIns && mIsDeleted)	//撤掉报单，合约一禁止下新单
		return;
	orderInsert(USTPMutexId::getMutexId(), insLabel, instrument, orderPrice, direction, qty, priceType, timeCondition, isFirstIns);	
}

void USTPOpponentArbitrage::orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	if(USTPMutexId::getActionNum(mInsComplex) > MAX_CANCEL_NUM)
		return;
	double adjustPrice = (priceType == THOST_FTDC_OPT_LimitPrice) ? orderPrice : 0.0;
	QString orderRef;
	if(isFirstIns){
		mCurrentReferIndex = 0;
		mFirstInsStatus = USTP_FTDC_OS_ORDER_SUBMIT;
		USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
		mFirstOrderRef = orderRef;		
	}else{
		OrderStatus state;
		state.status = USTP_FTDC_OS_ORDER_SUBMIT;
		USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
		state.orderRef = orderRef;
		mSecOrderRefMap.insert(orderRef, state);
	}
	QString szReqId = QString::number(reqId);
	mReqMap.insert(orderRef, szReqId);
	emit onUpdateOrderShow(szReqId, instrument, mOrderLabel, 'N', direction, 0.0, qty, qty, 0, mOffsetFlag, priceType, mHedgeFlag, mOrderPrice);

	if(mIsAutoFirstCancel && isFirstIns)
		QTimer::singleShot(mFirstCancelTime, this, SLOT(doAutoCancelFirstIns()));
	else if(mIsAutoSecondCancel && isFirstIns == false){
		mSecActionList.append(orderRef);
		QTimer::singleShot(mSecondCancelTime, this, SLOT(doAutoCancelSecIns()));
	}
	//条件日志
#ifdef _DEBUG
	int nIsSecCancel = mIsAutoSecondCancel ? 1 : 0;
	int nIsSecIns = isFirstIns ? 0 : 1;
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderInsert]   Instrument: ")) + instrument +  QString(tr("  RequestId: ")) + szReqId + QString(tr("  OrderRef: ")) + orderRef + 
		QString(tr("  UserId: ")) + mUserId + QString(tr("  PriceType: ")) + QString(priceType) + QString(tr("  OrderPrice: ")) + QString::number(adjustPrice) + QString(tr("  OrderVolume: ")) + 
		QString::number(qty) + QString(tr("  Direction: ")) + QString(direction) + QString(tr("  SecAutoCancel: ")) + QString::number(nIsSecCancel) + QString(tr("  IsSecIns: ")) + QString::number(nIsSecIns) +
		QString(tr("  CurrentReferIndex: ")) + QString::number(mCurrentReferIndex) + QString(tr("  FirstInsUpperPrice: ")) + QString::number(mFirstInsUpperPrice) + QString(tr("  FirstInsLowerPrice: ")) + QString::number(mFirstInsLowerPrice);
	USTPLogger::saveData(data);
#endif	
}

void USTPOpponentArbitrage::doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
										   const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
										   const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
	emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, orderStatus, direction, orderPrice, orderVolume, remainVolume, tradeVolume, offsetFlag, priceType, hedgeFlag, mOrderPrice);
	if(mFirstOrderRef == orderRef){
		mFirstInsStatus = orderStatus;
		mFirstRemainQty = remainVolume;
		if(THOST_FTDC_OST_Canceled == orderStatus){
			USTPMutexId::updateActionNum(mInsComplex);
			if(!mIsAutoFirstCancel){
				double fSecBasePrice = mSecondMarketBasePrice + mOrderPrice;
				defineTimeOrder(fSecBasePrice);
			}
			if(mIsDeleted)
				emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, mIsOppnentPrice, mIsDefineOrder, false,
				mIsActionReferTick, mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}else if(THOST_FTDC_OST_AllTraded == orderStatus){
			orderSecondIns(true, mOrderQty, 0.0, 0.0);
		}
	}else{
		mSecInsStatus = orderStatus;
		if(THOST_FTDC_OST_AllTraded == orderStatus){
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, true, mIsActionReferTick,
				mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}
	}
}


void USTPOpponentArbitrage::doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
										   const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
										   const QString& localId, const QString& orderRef, const QString& tradeTime)
{
	return;
}

void USTPOpponentArbitrage::orderSecondIns(bool isInit, const int& qty, const double& bidPrice, const double& askPrice)
{	
	if(THOST_FTDC_D_Buy == mBS){
		if(isInit){
			double initPrice = mSecondOrderBasePrice - mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, initPrice, THOST_FTDC_D_Sell, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}else{
			double price = bidPrice - mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, price, THOST_FTDC_D_Sell, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}
	}else{
		if(isInit){
			double initPrice = mSecondOrderBasePrice + mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, initPrice, THOST_FTDC_D_Buy, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}else{
			double price = askPrice + mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, price, THOST_FTDC_D_Buy, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}
	}
}

void USTPOpponentArbitrage::doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
													const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
													const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
	if(mFirstIns == instrumentId)	//设置合约状态
		mFirstInsStatus = USTP_FTDC_OS_ORDER_ERROR;
	else if(mSecIns == instrumentId)
		mSecInsStatus = USTP_FTDC_OS_ORDER_ERROR;
	switch (errorId){
	case 22:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'D', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice); //重复的报单
		break;
	case 31:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'Z', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	资金不足
		break;
	case 42:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'S', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	结算结果未确认
		break;
	case 50:
	case 51:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'P', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//平仓位不足
		break;
	default:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'W', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);
		break;
	}
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderInsert] orderRef: ")) + orderRef + QString(tr("  RequestId: ")) + mReqMap[orderRef] + QString(tr("  InstrumentId: ")) + instrumentId + 
		QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPOpponentArbitrage::doUSTPErrRtnOrderAction(const char& actionFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId,
													const QString& orderSysId, const QString& userActionLocalId, const QString& orderActionRef, const double& orderPrice, 
													const int& volumeChange, const int& errorId, const QString& errorMsg, const int& reqId)
{
#ifdef _DEBUG
	if(mReqMap.find(orderActionRef) == mReqMap.end())
		return;
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderAction] orderSysId: ")) + orderSysId + 
		QString(tr("  UserActionLocalId: ")) + userActionLocalId  + QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPOpponentArbitrage::doAutoCancelFirstIns()
{	
	if(isInMarket(mFirstInsStatus))
		cancelFirstIns();
}

void USTPOpponentArbitrage::cancelFirstIns()
{
	//mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
	//submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
	return;
}

void USTPOpponentArbitrage::doAutoCancelSecIns()
{	
	if(mSecActionList.isEmpty()){
#ifdef _DEBUG
		QString data = mOrderLabel + QString(tr("  [WARN]  The list is empty when Auto cancel second instrument."));
		USTPLogger::saveData(data);
#endif
		return;
	}

	//QString orderRef = mSecActionList.takeFirst();
	//if (mSecOrderRefMap.find(orderRef) != mSecOrderRefMap.end()){
	//	if(isInMarket(mSecOrderRefMap[orderRef].status)){
	//		mSecOrderRefMap[orderRef].status = USTP_FTDC_OS_CANCEL_SUBMIT;
	//		submitAction(SECOND_INSTRUMENT, mSecOrderRefMap[orderRef].orderRef, mSecIns);
	//	}
	//}
	return;
}

void USTPOpponentArbitrage::doDefineTimeOrderFirstIns()
{
	if((mFirstMarketBasePrice < VALUE_ACCURACY) || (mSecondMarketBasePrice < VALUE_ACCURACY) || (mFirstBidMarketPrice < VALUE_ACCURACY) || (mFirstAskMarketPrice < VALUE_ACCURACY))	//保证两腿都收到行情
		return;
	if((mFirstMarketBasePrice > INIT_VALUE) || (mSecondMarketBasePrice > INIT_VALUE) || (mFirstBidMarketPrice > INIT_VALUE) || (mFirstAskMarketPrice > INIT_VALUE))
		return;
	double fSecBasePrice = mSecondMarketBasePrice + mOrderPrice;
	if(mIsOppnentPrice)
		opponentPriceOrder(mFirstIns, fSecBasePrice);
	else
		defineTimeOrder(fSecBasePrice);
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [DefineTimeOrder] InstrumentId: ")) + mFirstIns + QString(tr("  mSecondMarketBasePrice: ")) + QString::number(fSecBasePrice) +
		QString(tr("  ReferenceIns: ")) + mReferenceIns;
	USTPLogger::saveData(data);
#endif
}

void USTPOpponentArbitrage::doDelOrder(const QString& orderStyle)
{
	if(orderStyle == mOrderLabel){
		mIsDeleted = true;
		if(isInMarket(mFirstInsStatus)){
			mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
			submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
		}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus || USTP_FTDC_OS_ORDER_ERROR == mFirstInsStatus || THOST_FTDC_OST_Canceled == mFirstInsStatus){
			emit onUpdateOrderShow(QString::number(mRequestId), mFirstIns, mOrderLabel, THOST_FTDC_OST_Canceled, mBS, 0.0, mOrderQty, mOrderQty, 0, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, false, mIsActionReferTick,
				mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}
		QString data = mOrderLabel + QString(tr("  [DoDelOrder]   mFirstInsStatus: ")) + QString(mFirstInsStatus);
		USTPLogger::saveData(data);
	}
}

void USTPOpponentArbitrage::submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument)
{	
	USTPTradeApi::reqOrderAction(USTPMutexId::getMutexId(), mBrokerId, mUserId, mInvestorId, instrument, orderLocalId, mFrontId, mSessionId);
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderAction]   UserLocalOrderId: ")) + orderLocalId + QString(tr("  InstrumentId: ")) + instrument;
	USTPLogger::saveData(data);
#endif	
}


USTPConditionStareArbitrage::USTPConditionStareArbitrage(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
														 const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, const int& actionSuperNum,
														 bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isReferTick, const double& firstMarketBidPrice, const double& firstMarketAskPrice,
														 const double& secMarketBidPrice, const double& secMarketAskPrice, USTPOrderWidget* pOrderWidget,USTPCancelWidget* pCancelWidget, USTPStrategyWidget* pStrategyWidget)
														 :USTPStrategyBase(orderLabel, firstIns, secIns, orderPriceTick, qty, bs, offset, hedge, cancelFirstTime, cancelSecTime, cycleStall, firstPriceSlipPoint,
														 secPriceSlipPoint, isAutoFirstCancel, isAutoSecCancel, isCycle)
{		
	moveToThread(&mStrategyThread);
	mStrategyThread.start();
	mFirstTradeQty = 0;
	mSecondTradeQty = 0;	
	mOrderType = 3;
	mRequestId = -1;
	mFirstRemainQty = mOrderQty;
	mIsActionReferTick = false;
	if(THOST_FTDC_D_Buy == bs){
		mFirstMarketBasePrice = firstMarketBidPrice;	
		mSecondMarketBasePrice = secMarketBidPrice;
	}else{
		mFirstMarketBasePrice = firstMarketAskPrice;	
		mSecondMarketBasePrice = secMarketAskPrice;
	}
	mFirstBidMarketPrice = firstMarketBidPrice;
	mFirstAskMarketPrice = firstMarketAskPrice;
	mSecondBidMarketPrice = secMarketBidPrice;
	mSecondAskMarketPrice = secMarketAskPrice;

	mFirstOrderPrice = 0.0;
	mSecondMarketOldBasePrice = 0.0;
	mFirstBidMarketVolume = 0;
	mFirstAskMarketVolume = 0;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mSecInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mSecondSlipPrice = secPriceSlipPoint * mPriceTick;
	mActionSuperSlipPrice = actionSuperNum * mPriceTick;
	mBrokerId = USTPCtpLoader::getBrokerId();
	mUserId = USTPMutexId::getUserId();
	mInvestorId = USTPMutexId::getInvestorId();
	mLimitSpread = USTPMutexId::getLimitSpread(mInsComplex) * mPriceTick;
	mIsCanMarket = (USTPMutexId::getInsMarketMaxVolume(secIns) > 0) ? true : false;
	initConnect(pStrategyWidget, pOrderWidget, pCancelWidget);	
	updateInitShow();
}

USTPConditionStareArbitrage::~USTPConditionStareArbitrage()
{
	mStrategyThread.quit();
	mStrategyThread.wait();
}

void USTPConditionStareArbitrage::initConnect(USTPStrategyWidget* pStrategyWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget)
{
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), 
		this, SLOT(doUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)),
		this, SLOT(doUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)),
		this, SLOT(doUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pOrderWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pCancelWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(pCancelWidget, SIGNAL(onDelOrder(const QString& )), this, SLOT(doDelOrder(const QString& )), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, 
		const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, const double&)), pStrategyWidget, SLOT(doOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, 
		const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&,
		const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, 
		const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, const double&)), pCancelWidget, SLOT(doOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, 
		const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, 
		const double&)), Qt::QueuedConnection);
}

void USTPConditionStareArbitrage::updateInitShow()
{
	mRequestId = USTPMutexId::getMutexId();
	QString szReqId = QString::number(mRequestId);
	emit onUpdateOrderShow(szReqId, mFirstIns, mOrderLabel, 'N', mBS, 0.0, mOrderQty, mOrderQty, 0, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
}

void USTPConditionStareArbitrage::doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
													   const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
													   const double& lowestPrice, const int& volume)
{	
	if ((mFirstIns != instrumentId) &&(mSecIns != instrumentId))
		return;
	if(THOST_FTDC_D_Buy == mBS && mFirstIns == instrumentId){
		mFirstMarketBasePrice = bidPrice;
	}else if(THOST_FTDC_D_Sell == mBS && mFirstIns == instrumentId){
		mFirstMarketBasePrice = askPrice;
	}else if(THOST_FTDC_D_Buy == mBS && mSecIns == instrumentId){
		mSecondMarketOldBasePrice = mSecondMarketBasePrice;
		mSecondMarketBasePrice = bidPrice;
	}else if(THOST_FTDC_D_Sell == mBS && mSecIns == instrumentId){
		mSecondMarketOldBasePrice = mSecondMarketBasePrice;
		mSecondMarketBasePrice = askPrice;
	}

	if(mFirstIns == instrumentId){
		mFirstBidMarketPrice = bidPrice;
		mFirstAskMarketPrice = askPrice;
		mFirstBidMarketVolume = bidVolume;
		mFirstAskMarketVolume = askVolume;
	}else{
		mSecondBidMarketPrice = bidPrice;
		mSecondAskMarketPrice = askPrice;
	}

	if((mFirstMarketBasePrice < VALUE_ACCURACY) || (mSecondMarketBasePrice < VALUE_ACCURACY) || (mSecondMarketBasePrice > mSecondMarketOldBasePrice + mLimitSpread))
		return;
	if((mFirstMarketBasePrice > INIT_VALUE) || (mSecondMarketBasePrice > INIT_VALUE))
		return;
	double fSecBasePrice = mSecondMarketBasePrice + mOrderPrice;
	conditionStarePriceOrder(instrumentId, fSecBasePrice);
}

void USTPConditionStareArbitrage::conditionStarePriceOrder(const QString& instrument, const double& basePrice)
{
	if(((USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus) || (THOST_FTDC_OST_Canceled == mFirstInsStatus))){
		if(THOST_FTDC_D_Buy == mBS && mFirstMarketBasePrice < basePrice - VALUE_ACCURACY){
			mFirstOrderPrice = basePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-NoOpponentPriceOrder #1]   FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + tr("  BasePrice: ") +
				QString::number(basePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif

		}else if(THOST_FTDC_D_Sell == mBS && mFirstMarketBasePrice > basePrice + VALUE_ACCURACY){
			mFirstOrderPrice = basePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-NoOpponentPriceOrder #2]   FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + tr("  BasePrice: ") +
				QString::number(basePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif
		}
	}else if(USTPStrategyBase::isInMarket(mFirstInsStatus)){
		double actionBidBasePrice = basePrice + mActionSuperSlipPrice;
		double actionAskBasePrice = basePrice - mActionSuperSlipPrice;
		if((mSecIns == instrument) && ((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice > actionBidBasePrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice < actionAskBasePrice)))){
			cancelFirstIns();
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-Cancel#1]   Instrument: ") + instrument +  tr("  FirstOrderPrice: ") + QString::number(mFirstOrderPrice) + tr("  ActionBidBasePrice: ") +
				QString::number(actionBidBasePrice)  + tr("  ActionAskBasePrice: ") + QString::number(actionAskBasePrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif
		}else if((THOST_FTDC_D_Buy == mBS && (mFirstAskMarketPrice < (basePrice + VALUE_ACCURACY))) || (THOST_FTDC_D_Sell == mBS && (mFirstBidMarketPrice > (basePrice - VALUE_ACCURACY)))){
			cancelFirstIns();
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-Cancel#2]   Instrument: ") + instrument +  tr("  FirstAskMarketPrice: ") + QString::number(mFirstAskMarketPrice) + tr("  BasePrice: ") +
				QString::number(basePrice)  + tr("  FirstBidMarketPrice: ") + QString::number(mFirstBidMarketPrice) + tr("  BS: ") + QString(mBS);
			USTPLogger::saveData(data);
#endif
		}else if((THOST_FTDC_D_Buy == mBS && (mFirstMarketBasePrice > mFirstOrderPrice) && (mFirstMarketBasePrice < basePrice)) || 
			(THOST_FTDC_D_Sell == mBS && (mFirstMarketBasePrice < mFirstOrderPrice  - VALUE_ACCURACY) && (mFirstMarketBasePrice > basePrice))){
				cancelFirstIns();
#ifdef _DEBUG
				QString data = tr("[") + mOrderLabel + tr("-Cancel#3]   Instrument: ") + instrument +  tr("  FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + tr("  BasePrice: ") +
					QString::number(basePrice)  + tr("  FirstOrderPrice: ") + QString::number(mFirstOrderPrice) + tr("  BS: ") + QString(mBS);
				USTPLogger::saveData(data);
#endif
		}
	}
}

void USTPConditionStareArbitrage::switchFirstInsOrder(const char& tCondition)
{
	if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus){
		if(mIsDeleted)
			return;
		orderInsert(mRequestId, FIRST_INSTRUMENT, mFirstIns, mFirstOrderPrice, mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, tCondition, true);
	}
	else
		submitOrder(FIRST_INSTRUMENT, mFirstIns, mFirstOrderPrice, mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, tCondition, true);
}

void USTPConditionStareArbitrage::submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	if(isFirstIns && mIsDeleted)	//撤掉报单，合约一禁止下新单
		return;
	orderInsert(USTPMutexId::getMutexId(), insLabel, instrument, orderPrice, direction, qty, priceType, timeCondition, isFirstIns);	
}

void USTPConditionStareArbitrage::orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	if(USTPMutexId::getActionNum(mInsComplex) > MAX_CANCEL_NUM)
		return;
	double adjustPrice = (priceType == THOST_FTDC_OPT_LimitPrice) ? orderPrice : 0.0;
	QString orderRef;
	if(isFirstIns){
		mFirstInsStatus = USTP_FTDC_OS_ORDER_SUBMIT;
		USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
		mFirstOrderRef = orderRef;
	}else{
		OrderStatus state;
		state.status = USTP_FTDC_OS_ORDER_SUBMIT;
		USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
		state.orderRef = orderRef;
		mSecOrderRefMap.insert(orderRef, state);
	}
	QString szReqId = QString::number(reqId);
	mReqMap.insert(orderRef, szReqId);
	emit onUpdateOrderShow(szReqId, instrument, mOrderLabel, 'N', direction, 0.0, qty, qty, 0, mOffsetFlag, priceType, mHedgeFlag, mOrderPrice);

	if(mIsAutoFirstCancel && isFirstIns)
		QTimer::singleShot(mFirstCancelTime, this, SLOT(doAutoCancelFirstIns()));
	else if(mIsAutoSecondCancel && isFirstIns == false){
		mSecActionList.append(orderRef);
		QTimer::singleShot(mSecondCancelTime, this, SLOT(doAutoCancelSecIns()));
	}
	//条件日志
#ifdef _DEBUG
	int nIsSecCancel = mIsAutoSecondCancel ? 1 : 0;
	int nIsSecIns = isFirstIns ? 0 : 1;
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderInsert]   Instrument: ")) + instrument +  QString(tr("  RequestId: ")) + szReqId + QString(tr("  OrderRef: ")) + orderRef + 
		QString(tr("  UserId: ")) + mUserId + QString(tr("  PriceType: ")) + QString(priceType) + QString(tr("  OrderPrice: ")) + QString::number(adjustPrice) + QString(tr("  OrderVolume: ")) + 
		QString::number(qty) + QString(tr("  Direction: ")) + QString(direction) + QString(tr("  SecAutoCancel: ")) + QString::number(nIsSecCancel) + QString(tr("  IsSecIns: ")) + QString::number(nIsSecIns);
	USTPLogger::saveData(data);
#endif	
}

void USTPConditionStareArbitrage::doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
											 const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
											 const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
	emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, orderStatus, direction, orderPrice, orderVolume, remainVolume, tradeVolume, offsetFlag, priceType, hedgeFlag, mOrderPrice);
	if(mFirstOrderRef == orderRef){
		mFirstInsStatus = orderStatus;
		mFirstRemainQty = remainVolume;

		if(THOST_FTDC_OST_Canceled == orderStatus){
			USTPMutexId::updateActionNum(mInsComplex);
			if (!mIsAutoFirstCancel){
				double fSecBasePrice = mSecondMarketBasePrice + mOrderPrice;
				if(THOST_FTDC_D_Buy == mBS && mFirstMarketBasePrice < fSecBasePrice - VALUE_ACCURACY){
					mFirstOrderPrice = fSecBasePrice;
					switchFirstInsOrder(THOST_FTDC_TC_GFD);

				}else if(THOST_FTDC_D_Sell == mBS && mFirstMarketBasePrice > fSecBasePrice + VALUE_ACCURACY){
					mFirstOrderPrice = fSecBasePrice;
					switchFirstInsOrder(THOST_FTDC_TC_GFD);

				}
			}
			if(mIsDeleted)
				emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, false, mIsActionReferTick,
				mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}else if(THOST_FTDC_OST_AllTraded == orderStatus){
			orderSecondIns(true, mOrderQty, 0.0, 0.0);
		}
	}else{
		mSecInsStatus = orderStatus;
		if(THOST_FTDC_OST_AllTraded == orderStatus){
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, true, mIsActionReferTick,
				mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}
	}
}


void USTPConditionStareArbitrage::doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
											 const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
											 const QString& localId, const QString& orderRef, const QString& tradeTime)
{
	return;
}

void USTPConditionStareArbitrage::orderSecondIns(bool isInit, const int& qty, const double& bidPrice, const double& askPrice)
{	
	if(THOST_FTDC_D_Buy == mBS){
		if(isInit){
			double initPrice = mFirstOrderPrice - mOrderPrice - mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, initPrice, THOST_FTDC_D_Sell, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}else{
			double price = bidPrice - mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, price, THOST_FTDC_D_Sell, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}
	}else{
		if(isInit){
			double initPrice = mFirstOrderPrice - mOrderPrice + mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, initPrice, THOST_FTDC_D_Buy, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}else{
			double price = askPrice + mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, price, THOST_FTDC_D_Buy, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}
	}
}

void USTPConditionStareArbitrage::doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
													  const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
													  const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;

	if(mFirstIns == instrumentId)	//设置合约状态
		mFirstInsStatus = USTP_FTDC_OS_ORDER_ERROR;
	else if(mSecIns == instrumentId)
		mSecInsStatus = USTP_FTDC_OS_ORDER_ERROR;
	switch (errorId){
	case 22:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'D', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice); //重复的报单
		break;
	case 31:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'Z', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	资金不足
		break;
	case 42:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'S', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	结算结果未确认
		break;
	case 50:
	case 51:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'P', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//平仓位不足
		break;
	default:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'W', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);
		break;
	}
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderInsert] orderRef: ")) + orderRef + QString(tr("  RequestId: ")) + mReqMap[orderRef] + QString(tr("  InstrumentId: ")) + instrumentId + 
		QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPConditionStareArbitrage::doUSTPErrRtnOrderAction(const char& actionFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId,
													  const QString& orderSysId, const QString& userActionLocalId, const QString& orderActionRef, const double& orderPrice, 
													  const int& volumeChange, const int& errorId, const QString& errorMsg, const int& reqId)
{
#ifdef _DEBUG
	if(mReqMap.find(orderActionRef) == mReqMap.end())
		return;
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderAction] orderSysId: ")) + orderSysId + 
		QString(tr("  UserActionLocalId: ")) + userActionLocalId  + QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPConditionStareArbitrage::doAutoCancelFirstIns()
{	
	if(isInMarket(mFirstInsStatus))
		cancelFirstIns();
}

void USTPConditionStareArbitrage::cancelFirstIns()
{
	mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
	submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
}

void USTPConditionStareArbitrage::doAutoCancelSecIns()
{	
	if(mSecActionList.isEmpty()){
#ifdef _DEBUG
		QString data = mOrderLabel + QString(tr("  [WARN]  The list is empty when Auto cancel second instrument."));
		USTPLogger::saveData(data);
#endif
		return;
	}
	return;
	//QString orderRef = mSecActionList.takeFirst();
	//if (mSecOrderRefMap.find(orderRef) != mSecOrderRefMap.end()){
	//	if(isInMarket(mSecOrderRefMap[orderRef].status)){
	//		mSecOrderRefMap[orderRef].status = USTP_FTDC_OS_CANCEL_SUBMIT;
	//		submitAction(SECOND_INSTRUMENT, mSecOrderRefMap[orderRef].orderRef, mSecIns);
	//	}
	//}
}

void USTPConditionStareArbitrage::doDelOrder(const QString& orderStyle)
{
	if(orderStyle == mOrderLabel){
		mIsDeleted = true;
		if(isInMarket(mFirstInsStatus)){
			mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
			submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
		}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus || USTP_FTDC_OS_ORDER_ERROR == mFirstInsStatus || THOST_FTDC_OST_Canceled == mFirstInsStatus){
			emit onUpdateOrderShow(QString::number(mRequestId), mFirstIns, mOrderLabel, THOST_FTDC_OST_Canceled, mBS, 0.0, mOrderQty, mOrderQty, 0, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, false, mIsActionReferTick,
				mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}
		QString data = mOrderLabel + QString(tr("  [DoDelOrder]   mFirstInsStatus: ")) + QString(mFirstInsStatus);
		USTPLogger::saveData(data);
	}
}

void USTPConditionStareArbitrage::submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument)
{	
	USTPTradeApi::reqOrderAction(USTPMutexId::getMutexId(), mBrokerId, mUserId, mInvestorId, instrument, orderLocalId, mFrontId, mSessionId);
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderAction]   OrderRef: ")) + orderLocalId + QString(tr("  InstrumentId: ")) + instrument;
	USTPLogger::saveData(data);
#endif	
}



USTPOpponentStareArbitrage::USTPOpponentStareArbitrage(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
											 const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
											 const int& actionSuperNum, bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isReferTick, const double& firstMarketBidPrice,
											 const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget,
											 USTPStrategyWidget* pStrategyWidget) :USTPStrategyBase(orderLabel, firstIns, secIns, orderPriceTick, qty, bs, offset, hedge, cancelFirstTime, cancelSecTime, cycleStall, firstPriceSlipPoint,
											 secPriceSlipPoint, isAutoFirstCancel, isAutoSecCancel, isCycle)
{	
	moveToThread(&mStrategyThread);
	mStrategyThread.start();
	mOrderType = 4;
	mRequestId = -1;
	mFirstTradeQty = 0;
	mFirstRemainQty = mOrderQty;
	mSecondTradeQty = 0;
	mFirstBidMarketVolume = 0;
	mFirstAskMarketVolume = 0;
	mCurrentReferIndex = 0;
	mActionReferNum = actionReferNum;
	mActionSuperNum = actionSuperNum;
	mIsDefineOrder = isDefineOrder;
	mIsOppnentPrice = isOppentPrice;
	mIsActionReferTick = isReferTick;
	if(THOST_FTDC_D_Buy == bs){
		mFirstMarketBasePrice = firstMarketBidPrice;	
		mSecondMarketBasePrice = secMarketBidPrice;
	}else{
		mFirstMarketBasePrice = firstMarketAskPrice;	
		mSecondMarketBasePrice = secMarketAskPrice;
	}
	mFirstBidMarketPrice = firstMarketBidPrice;
	mFirstAskMarketPrice = firstMarketAskPrice;
	mSecondBidMarketPrice = secMarketBidPrice;
	mSecondAskMarketPrice = secMarketAskPrice;
	mFirstOrderPrice = 0.0;
	mSecondOrderBasePrice = 0.0;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mSecInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mFirstSlipPrice = firstPriceSlipPoint * mPriceTick;
	mSecondSlipPrice = secPriceSlipPoint * mPriceTick;
	mActionSuperSlipPrice = actionSuperNum * mPriceTick;
	mBrokerId = USTPCtpLoader::getBrokerId();
	mUserId = USTPMutexId::getUserId();
	mInvestorId = USTPMutexId::getInvestorId();
	mReferenceIns = USTPMutexId::getReferenceIns();
	mIsCanMarket = (USTPMutexId::getInsMarketMaxVolume(secIns) > 0) ? true : false;
	initConnect(pStrategyWidget, pOrderWidget, pCancelWidget);	
	updateInitShow();
}

USTPOpponentStareArbitrage::~USTPOpponentStareArbitrage()
{
	mStrategyThread.quit();
	mStrategyThread.wait();
}

void USTPOpponentStareArbitrage::initConnect(USTPStrategyWidget* pStrategyWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget)
{
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), 
		this, SLOT(doUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)),
		this, SLOT(doUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)),
		this, SLOT(doUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pOrderWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pCancelWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(pCancelWidget, SIGNAL(onDelOrder(const QString& )), this, SLOT(doDelOrder(const QString& )), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, 
		const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, const double&)), pStrategyWidget, SLOT(doOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, 
		const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&,
		const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, 
		const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, const double&)), pCancelWidget, SLOT(doOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, 
		const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, 
		const double&)), Qt::QueuedConnection);
}

void USTPOpponentStareArbitrage::updateInitShow()
{	
	if(mIsDeleted)	//如果手工撤单，则不更新
		return;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mRequestId = USTPMutexId::getMutexId();
	QString szReqId = QString::number(mRequestId);
	emit onUpdateOrderShow(szReqId, mFirstIns, mOrderLabel, 'N', mBS, 0.0, mFirstRemainQty, mFirstRemainQty, mFirstTradeQty, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
}

void USTPOpponentStareArbitrage::doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
													 const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
													 const double& lowestPrice, const int& volume)
{	
	if(mIsDefineOrder && mReferenceIns == instrumentId)
		QTimer::singleShot(DEFINE_ORDER_TIME, this, SLOT(doDefineTimeOrderFirstIns()));

	if ((mFirstIns != instrumentId) &&(mSecIns != instrumentId))	//监听第一，二腿行情
		return;
	if(THOST_FTDC_D_Buy == mBS && mFirstIns == instrumentId)
		mFirstMarketBasePrice = bidPrice;
	else if(THOST_FTDC_D_Sell == mBS && mFirstIns == instrumentId)
		mFirstMarketBasePrice = askPrice;
	else if(THOST_FTDC_D_Buy == mBS && mSecIns == instrumentId)
		mSecondMarketBasePrice = bidPrice;
	else if(THOST_FTDC_D_Sell == mBS && mSecIns == instrumentId)
		mSecondMarketBasePrice = askPrice;
	if(mFirstIns == instrumentId){
		mFirstBidMarketPrice = bidPrice;
		mFirstAskMarketPrice = askPrice;
		mFirstBidMarketVolume = bidVolume;
		mFirstAskMarketVolume = askVolume;
	}else{
		mSecondBidMarketPrice = bidPrice;
		mSecondAskMarketPrice = askPrice;
	}

	if((mFirstMarketBasePrice < VALUE_ACCURACY) || (mSecondMarketBasePrice < VALUE_ACCURACY) || (mFirstBidMarketPrice < VALUE_ACCURACY) || (mFirstAskMarketPrice < VALUE_ACCURACY))	//保证两腿都收到行情
		return;
	if((mFirstMarketBasePrice > INIT_VALUE) || (mSecondMarketBasePrice > INIT_VALUE) || (mFirstBidMarketPrice > INIT_VALUE) || (mFirstAskMarketPrice > INIT_VALUE))
		return;
	double fSecBasePrice = mSecondMarketBasePrice + mOrderPrice;
	if(mIsOppnentPrice)
		opponentPriceOrder(instrumentId, fSecBasePrice);
	else
		noOpponentPriceOrder(instrumentId, fSecBasePrice);
}

void USTPOpponentStareArbitrage::opponentPriceOrder(const QString& instrument, const double& basePrice)
{
	if(((USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus) || (THOST_FTDC_OST_Canceled == mFirstInsStatus))){//第一腿没有下单或者已撤单
		if(THOST_FTDC_D_Buy == mBS && mFirstAskMarketPrice <= (basePrice + VALUE_ACCURACY)){	//买委托满足下单条件
			mFirstOrderPrice = mFirstAskMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else if(THOST_FTDC_D_Sell == mBS && mFirstBidMarketPrice >= (basePrice - VALUE_ACCURACY)){//卖委托满足下单条件
			mFirstOrderPrice = mFirstBidMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);
		}else{
			if(THOST_FTDC_OST_Canceled == mFirstInsStatus){
				updateInitShow();
			}
		}
	}else if(USTPStrategyBase::isInMarket(mFirstInsStatus)){
		double actionBidBasePrice = basePrice + mActionSuperSlipPrice;
		double actionAskBasePrice = basePrice - mActionSuperSlipPrice;
		if((mSecIns == instrument) && ((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice > actionBidBasePrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice < actionAskBasePrice)))){
			cancelFirstIns();
		}else if((mSecIns == instrument) && ((THOST_FTDC_D_Buy == mBS && (mFirstAskMarketPrice <= (basePrice + VALUE_ACCURACY))) || (THOST_FTDC_D_Sell == mBS && 
			(mFirstBidMarketPrice >= (basePrice - VALUE_ACCURACY))))){
				cancelFirstIns();
		}else if((THOST_FTDC_D_Buy == mBS && (mFirstAskMarketPrice > mFirstOrderPrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstBidMarketPrice < mFirstOrderPrice))){
			cancelFirstIns();
		}
	}
}

void USTPOpponentStareArbitrage::noOpponentPriceOrder(const QString& instrument, const double& basePrice)
{
	if(((USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus) || (THOST_FTDC_OST_Canceled == mFirstInsStatus))){//第一腿没有下单或者已撤单

		if(THOST_FTDC_D_Buy == mBS && mFirstAskMarketPrice <= (basePrice + VALUE_ACCURACY)){	//对价单买委托满足下单条件
			mFirstOrderPrice = mFirstAskMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else if(THOST_FTDC_D_Sell == mBS && mFirstBidMarketPrice >= (basePrice - VALUE_ACCURACY)){//对价单卖委托满足下单条件
			mFirstOrderPrice = mFirstBidMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else if(THOST_FTDC_D_Buy == mBS && mFirstMarketBasePrice <= (basePrice + VALUE_ACCURACY)){	//条件单买委托满足下单条件
			mFirstOrderPrice = mFirstMarketBasePrice + mFirstSlipPrice;
			if(mFirstOrderPrice > basePrice)
				mFirstOrderPrice = basePrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else if(THOST_FTDC_D_Sell == mBS && mFirstMarketBasePrice >= (basePrice - VALUE_ACCURACY)){//条件单卖委托满足下单条件
			mFirstOrderPrice = mFirstMarketBasePrice - mFirstSlipPrice;
			if(mFirstOrderPrice < basePrice)
				mFirstOrderPrice = basePrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else{
			if(THOST_FTDC_OST_Canceled == mFirstInsStatus){
				updateInitShow();
			}
		}
	}else if(USTPStrategyBase::isInMarket(mFirstInsStatus)){//1.非设定时间撤单的情况，第一腿委托成功，第一腿行情发生变化，腿一撤单重发;2.设定时间撤单的情况，定时超时，根据行情触发。
		if(mIsActionReferTick && (mFirstIns == instrument)){
			if(THOST_FTDC_D_Buy == mBS){
				if((mFirstMarketBasePrice > mFirstOrderPrice + VALUE_ACCURACY) && (mFirstMarketBasePrice < basePrice)){
					mCurrentReferIndex++;
#ifdef _DEBUG
					QString data = tr("[") + mOrderLabel + tr("-Add1###]   +1: ") + tr("  FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + 
						tr("  FirstOrderPrice: ") + QString::number(mFirstOrderPrice);
					USTPLogger::saveData(data);
#endif
				}else{
					mCurrentReferIndex = 0;
#ifdef _DEBUG
					QString data = tr("[") + mOrderLabel + tr("-Add1###]   +2: ") + tr("  FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + 
						tr("  FirstOrderPrice: ") + QString::number(mFirstOrderPrice);
					USTPLogger::saveData(data);
#endif
				}
			}else{
				if((mFirstMarketBasePrice < mFirstOrderPrice - VALUE_ACCURACY)&& (mFirstMarketBasePrice > basePrice)){
					mCurrentReferIndex++;
#ifdef _DEBUG
					QString data = tr("[") + mOrderLabel + tr("-Add1###]   -1: ") + tr("  FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + 
						tr("  FirstOrderPrice: ") + QString::number(mFirstOrderPrice);
					USTPLogger::saveData(data);
#endif
				}else{
					mCurrentReferIndex = 0;
#ifdef _DEBUG
					QString data = tr("[") + mOrderLabel + tr("-Add1###]   -2: ") + tr("  FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + 
						tr("  FirstOrderPrice: ") + QString::number(mFirstOrderPrice);
					USTPLogger::saveData(data);
#endif
				}
			}
		}else if(mIsActionReferTick && (mSecIns == instrument)){
			if(mCurrentReferIndex > 0){
				mCurrentReferIndex++;
#ifdef _DEBUG
				QString data = tr("[") + mOrderLabel + tr("-Add2###]   +1: ");
				USTPLogger::saveData(data);
#endif
			}
		}

		double actionBidBasePrice = basePrice + mActionSuperSlipPrice;
		double actionAskBasePrice = basePrice - mActionSuperSlipPrice;
#ifdef _DEBUG
		QString data = tr("[") + mOrderLabel + tr("-RealData]   Instrument: ") + instrument +  tr("  FirstOrderPrice: ") + QString::number(mFirstOrderPrice) + tr("  ActionBidBasePrice: ") +
			QString::number(actionBidBasePrice)  + tr("  ActionAskBasePrice: ") + QString::number(actionAskBasePrice) + tr("  FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) + 
			tr("  BasePrice: ") + QString::number(basePrice) + tr("  FirstAskMarketPrice: ") + QString::number(mFirstAskMarketPrice) + tr("  FirstBidMarketPrice: ") + QString::number(mFirstBidMarketPrice) + 
			tr("  SecBidMarketPrice: ") + QString::number(mSecondBidMarketPrice) + tr("  SecondMarketBasePrice: ") + QString::number(mSecondMarketBasePrice) + tr("  CurrentReferIndex: ") +
			QString::number(mCurrentReferIndex) + tr("  ActionReferNum: ") + QString::number(mActionReferNum) + tr("  BS: ") + QString(mBS);
		USTPLogger::saveData(data);
#endif
		if((mSecIns == instrument) && ((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice > actionBidBasePrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice < actionAskBasePrice)))){
			cancelFirstIns();
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-Cancel###]   1: ");
			USTPLogger::saveData(data);
#endif
		}else if((THOST_FTDC_D_Buy == mBS && (mFirstAskMarketPrice <= (basePrice + VALUE_ACCURACY))) || (THOST_FTDC_D_Sell == mBS && (mFirstBidMarketPrice >= (basePrice - VALUE_ACCURACY)))){
			cancelFirstIns();
#ifdef _DEBUG
			QString data = tr("[") + mOrderLabel + tr("-Cancel###]   2: ");
			USTPLogger::saveData(data);
#endif
		}else if((THOST_FTDC_D_Buy == mBS && (mIsActionReferTick && (mCurrentReferIndex >= mActionReferNum)) && (mFirstMarketBasePrice < basePrice)) || 
			(THOST_FTDC_D_Sell == mBS && (mIsActionReferTick && (mCurrentReferIndex >= mActionReferNum)) && (mFirstMarketBasePrice > basePrice))){
				cancelFirstIns();
#ifdef _DEBUG
				QString data = tr("[") + mOrderLabel + tr("-Cancel]   3: ") + instrument +  tr("  CurrentReferIndex: ") + QString::number(mCurrentReferIndex) +
					tr("  ActionReferNum: ") + QString::number(mActionReferNum) + tr("  FirstMarketBasePrice: ") + QString::number(mFirstMarketBasePrice) +
					tr("  BasePrice: ") + QString::number(basePrice) + tr("  BS: ") + QString(mBS);
				USTPLogger::saveData(data);
#endif
		}
	}
}

void USTPOpponentStareArbitrage::defineTimeOrder(const double& basePrice)
{
	if(((USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus) || (THOST_FTDC_OST_Canceled == mFirstInsStatus))){//第一腿没有下单或者已撤单
		if(THOST_FTDC_D_Buy == mBS && mFirstAskMarketPrice <= (basePrice + VALUE_ACCURACY)){	//对价单买委托满足下单条件
			mFirstOrderPrice = mFirstAskMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else if(THOST_FTDC_D_Sell == mBS && mFirstBidMarketPrice >= (basePrice - VALUE_ACCURACY)){//对价单卖委托满足下单条件
			mFirstOrderPrice = mFirstBidMarketPrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else if(THOST_FTDC_D_Buy == mBS && mFirstMarketBasePrice <= (basePrice + VALUE_ACCURACY)){	//条件单买委托满足下单条件
			mFirstOrderPrice = mFirstMarketBasePrice + mFirstSlipPrice;
			if(mFirstOrderPrice > basePrice)
				mFirstOrderPrice = basePrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else if(THOST_FTDC_D_Sell == mBS && mFirstMarketBasePrice >= (basePrice - VALUE_ACCURACY)){//条件单卖委托满足下单条件
			mFirstOrderPrice = mFirstMarketBasePrice - mFirstSlipPrice;
			if(mFirstOrderPrice < basePrice)
				mFirstOrderPrice = basePrice;
			mSecondOrderBasePrice = mSecondMarketBasePrice;
			switchFirstInsOrder(THOST_FTDC_TC_GFD);

		}else{
			if(THOST_FTDC_OST_Canceled == mFirstInsStatus){
				updateInitShow();
			}
		}
	}
}

void USTPOpponentStareArbitrage::switchFirstInsOrder(const char& tCondition)
{
	if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus){
		if(mIsDeleted)
			return;
		orderInsert(mRequestId, FIRST_INSTRUMENT, mFirstIns, mFirstOrderPrice, mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, tCondition, true);
	}
	else
		submitOrder(FIRST_INSTRUMENT, mFirstIns, mFirstOrderPrice, mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, tCondition, true);
}

void USTPOpponentStareArbitrage::submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	if(isFirstIns && mIsDeleted)	//撤掉报单，合约一禁止下新单
		return;
	orderInsert(USTPMutexId::getMutexId(), insLabel, instrument, orderPrice, direction, qty, priceType, timeCondition, isFirstIns);	
}

void USTPOpponentStareArbitrage::orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	if(USTPMutexId::getActionNum(mInsComplex) > MAX_CANCEL_NUM)
		return;
	double adjustPrice = (priceType == THOST_FTDC_OPT_LimitPrice) ? orderPrice : 0.0;
	QString orderRef;
	if(isFirstIns){
		mCurrentReferIndex = 0;
		mFirstInsStatus = USTP_FTDC_OS_ORDER_SUBMIT;
		USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
		mFirstOrderRef = orderRef;		
	}else{
		OrderStatus state;
		state.status = USTP_FTDC_OS_ORDER_SUBMIT;
		USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
		state.orderRef = orderRef;
		mSecOrderRefMap.insert(orderRef, state);
	}
	QString szReqId = QString::number(reqId);
	mReqMap.insert(orderRef, szReqId);
	emit onUpdateOrderShow(szReqId, instrument, mOrderLabel, 'N', direction, 0.0, qty, qty, 0, mOffsetFlag, priceType, mHedgeFlag, mOrderPrice);

	if(mIsAutoFirstCancel && isFirstIns)
		QTimer::singleShot(mFirstCancelTime, this, SLOT(doAutoCancelFirstIns()));
	else if(mIsAutoSecondCancel && isFirstIns == false){
		mSecActionList.append(orderRef);
		QTimer::singleShot(mSecondCancelTime, this, SLOT(doAutoCancelSecIns()));
	}
	//条件日志
#ifdef _DEBUG
	int nIsSecCancel = mIsAutoSecondCancel ? 1 : 0;
	int nIsSecIns = isFirstIns ? 0 : 1;
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderInsert]   Instrument: ")) + instrument +  QString(tr("  RequestId: ")) + szReqId + QString(tr("  OrderRef: ")) + orderRef + 
		QString(tr("  UserId: ")) + mUserId + QString(tr("  PriceType: ")) + QString(priceType) + QString(tr("  OrderPrice: ")) + QString::number(adjustPrice) + QString(tr("  OrderVolume: ")) + 
		QString::number(qty) + QString(tr("  Direction: ")) + QString(direction) + QString(tr("  SecAutoCancel: ")) + QString::number(nIsSecCancel) + QString(tr("  IsSecIns: ")) + QString::number(nIsSecIns) +
		QString(tr("  CurrentReferIndex: ")) + QString::number(mCurrentReferIndex);
	USTPLogger::saveData(data);
#endif	
}

void USTPOpponentStareArbitrage::doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
										   const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
										   const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
	emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, orderStatus, direction, orderPrice, orderVolume, remainVolume, tradeVolume, offsetFlag, priceType, hedgeFlag, mOrderPrice);
	if(mFirstOrderRef == orderRef){
		mFirstInsStatus = orderStatus;
		mFirstRemainQty = remainVolume;

		if(THOST_FTDC_OST_Canceled == orderStatus){
			USTPMutexId::updateActionNum(mInsComplex);
			if(!mIsAutoFirstCancel){
				double fSecBasePrice = mSecondMarketBasePrice + mOrderPrice;
				defineTimeOrder(fSecBasePrice);
			}
			if(mIsDeleted)
				emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, mIsOppnentPrice, mIsDefineOrder, false,
				mIsActionReferTick, mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}else if(THOST_FTDC_OST_AllTraded == orderStatus){
			orderSecondIns(true, mOrderQty, 0.0, 0.0);
		}
	}else{
		mSecInsStatus = orderStatus;
		if(THOST_FTDC_OST_AllTraded == orderStatus){
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, true, mIsActionReferTick,
				mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}
	}
}


void USTPOpponentStareArbitrage::doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
										   const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
										   const QString& localId, const QString& orderRef, const QString& tradeTime)
{
	return;
}

void USTPOpponentStareArbitrage::orderSecondIns(bool isInit, const int& qty, const double& bidPrice, const double& askPrice)
{	
	if(THOST_FTDC_D_Buy == mBS){
		if(isInit){
			double initPrice = mSecondOrderBasePrice - mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, initPrice, THOST_FTDC_D_Sell, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}else{
			double price = bidPrice - mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, price, THOST_FTDC_D_Sell, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}
	}else{
		if(isInit){
			double initPrice = mSecondOrderBasePrice + mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, initPrice, THOST_FTDC_D_Buy, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}else{
			double price = askPrice + mSecondSlipPrice;
			submitOrder(SECOND_INSTRUMENT, mSecIns, price, THOST_FTDC_D_Buy, qty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, false);
		}
	}
}

void USTPOpponentStareArbitrage::doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
													const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
													const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
	if(mFirstIns == instrumentId)	//设置合约状态
		mFirstInsStatus = USTP_FTDC_OS_ORDER_ERROR;
	else if(mSecIns == instrumentId)
		mSecInsStatus = USTP_FTDC_OS_ORDER_ERROR;
	switch (errorId){
	case 22:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'D', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice); //重复的报单
		break;
	case 31:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'Z', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	资金不足
		break;
	case 42:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'S', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	结算结果未确认
		break;
	case 50:
	case 51:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'P', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//平仓位不足
		break;
	default:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'W', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);
		break;
	}
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderInsert] orderRef: ")) + orderRef + QString(tr("  RequestId: ")) + mReqMap[orderRef] + QString(tr("  InstrumentId: ")) + instrumentId + 
		QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPOpponentStareArbitrage::doUSTPErrRtnOrderAction(const char& actionFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId,
													const QString& orderSysId, const QString& userActionLocalId, const QString& orderActionRef, const double& orderPrice, 
													const int& volumeChange, const int& errorId, const QString& errorMsg, const int& reqId)
{
#ifdef _DEBUG
	if(mReqMap.find(orderActionRef) == mReqMap.end())
		return;
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderAction] orderSysId: ")) + orderSysId + 
		QString(tr("  UserActionLocalId: ")) + userActionLocalId  + QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPOpponentStareArbitrage::doAutoCancelFirstIns()
{	
	if(isInMarket(mFirstInsStatus))
		cancelFirstIns();
}

void USTPOpponentStareArbitrage::cancelFirstIns()
{
	mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
	submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
}

void USTPOpponentStareArbitrage::doAutoCancelSecIns()
{	
	if(mSecActionList.isEmpty()){
#ifdef _DEBUG
		QString data = mOrderLabel + QString(tr("  [WARN]  The list is empty when Auto cancel second instrument."));
		USTPLogger::saveData(data);
#endif
		return;
	}

	//QString orderRef = mSecActionList.takeFirst();
	//if (mSecOrderRefMap.find(orderRef) != mSecOrderRefMap.end()){
	//	if(isInMarket(mSecOrderRefMap[orderRef].status)){
	//		mSecOrderRefMap[orderRef].status = USTP_FTDC_OS_CANCEL_SUBMIT;
	//		submitAction(SECOND_INSTRUMENT, mSecOrderRefMap[orderRef].orderRef, mSecIns);
	//	}
	//}
	return;
}

void USTPOpponentStareArbitrage::doDefineTimeOrderFirstIns()
{
	if((mFirstMarketBasePrice < VALUE_ACCURACY) || (mSecondMarketBasePrice < VALUE_ACCURACY) || (mFirstBidMarketPrice < VALUE_ACCURACY) || (mFirstAskMarketPrice < VALUE_ACCURACY))	//保证两腿都收到行情
		return;
	if((mFirstMarketBasePrice > INIT_VALUE) || (mSecondMarketBasePrice > INIT_VALUE) || (mFirstBidMarketPrice > INIT_VALUE) || (mFirstAskMarketPrice > INIT_VALUE))
		return;
	double fSecBasePrice = mSecondMarketBasePrice + mOrderPrice;
	if(mIsOppnentPrice)
		opponentPriceOrder(mFirstIns, fSecBasePrice);
	else
		defineTimeOrder(fSecBasePrice);
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [DefineTimeOrder] InstrumentId: ")) + mFirstIns + QString(tr("  mSecondMarketBasePrice: ")) + QString::number(fSecBasePrice) +
		QString(tr("  ReferenceIns: ")) + mReferenceIns;
	USTPLogger::saveData(data);
#endif
}

void USTPOpponentStareArbitrage::doDelOrder(const QString& orderStyle)
{
	if(orderStyle == mOrderLabel){
		mIsDeleted = true;
		if(isInMarket(mFirstInsStatus)){
			mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
			submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
		}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus || USTP_FTDC_OS_ORDER_ERROR == mFirstInsStatus || THOST_FTDC_OST_Canceled == mFirstInsStatus){
			emit onUpdateOrderShow(QString::number(mRequestId), mFirstIns, mOrderLabel, THOST_FTDC_OST_Canceled, mBS, 0.0, mOrderQty, mOrderQty, 0, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, false, mIsActionReferTick,
				mOrderType, mFirstBidMarketPrice, mFirstAskMarketPrice, mSecondBidMarketPrice, mSecondAskMarketPrice);
		}
		QString data = mOrderLabel + QString(tr("  [DoDelOrder]   mFirstInsStatus: ")) + QString(mFirstInsStatus);
		USTPLogger::saveData(data);
	}
}

void USTPOpponentStareArbitrage::submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument)
{	
	USTPTradeApi::reqOrderAction(USTPMutexId::getMutexId(), mBrokerId, mUserId, mInvestorId, instrument, orderLocalId, mFrontId, mSessionId);
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderAction]   UserLocalOrderId: ")) + orderLocalId + QString(tr("  InstrumentId: ")) + instrument;
	USTPLogger::saveData(data);
#endif	
}



USTPSpeculateOrder::USTPSpeculateOrder(const QString& orderLabel, const QString& speLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& qty, const char& bs,  const char& offset,
												 const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& firstSlipPoint, const int& secSlipPoint, bool isAutoFirstCancel, 
												 bool isAutoSecCancel, bool isCycle, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, USTPSubmitWidget* pSubmitWidget)
												 :USTPStrategyBase(orderLabel, firstIns, secIns, orderPriceTick, qty, bs, offset, hedge, 0, 0, 0, 0, 0, false, false, false)
{		
	moveToThread(&mStrategyThread);
	mStrategyThread.start();
	mOrderType = 5;
	mRequestId = -1;
	mTempOrderQty = 0;
	mTempOrderPrice = 0.0;
	mSpeOrderLabel = speLabel;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mBrokerId = USTPCtpLoader::getBrokerId();
	mUserId = USTPMutexId::getUserId();
	mInvestorId = USTPMutexId::getInvestorId();
	initConnect(pSubmitWidget, pOrderWidget, pCancelWidget);
}

USTPSpeculateOrder::~USTPSpeculateOrder()
{
	mStrategyThread.quit();
	mStrategyThread.wait();
}

void USTPSpeculateOrder::initConnect(USTPSubmitWidget* pSubmitWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget)
{
	connect(pSubmitWidget, SIGNAL(onSubmitOrder(const QString&, const QString&, const QString&, const char&, const char&, const int&, const double&)), 
		this, SLOT(doSubmitOrder(const QString&, const QString&, const QString&, const char&, const char&, const int&, const double&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)),
		this, SLOT(doUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)),
		this, SLOT(doUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pOrderWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pCancelWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(pCancelWidget, SIGNAL(onDelOrder(const QString& )), this, SLOT(doDelOrder(const QString& )), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, 
		const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, const double&)), pCancelWidget, SLOT(doOrderFinished(const QString&, const QString&, const QString&, const double&, const int&, 
		const int&, const int&, const char&,  const char&, const char&, const int&, const int&, const int&, const int&, const int&, const double&, bool, bool, bool, bool, bool, bool, bool, const int&, const double&, const double&, const double&, 
		const double&)), Qt::QueuedConnection);
}

void USTPSpeculateOrder::doSubmitOrder(const QString& orderLabel, const QString& speLabel, const QString& ins, const char& direction, const char& offsetFlag, const int& volume, const double& orderPrice)
{	
	if(mSpeOrderLabel != speLabel)
		return;
	if(USTPStrategyBase::isInMarket(mFirstInsStatus)){
		mTempOrderLabel = orderLabel;
		mTempOffsetFlag = offsetFlag;
		mTempBS = direction;
		mTempFirstIns = ins;
		mTempOrderQty = volume;
		mTempOrderPrice = orderPrice;
		mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
		submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);

	}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus || THOST_FTDC_OST_Canceled == mFirstInsStatus){
		mOrderLabel = orderLabel;
		mOffsetFlag = offsetFlag;
		mBS = direction;
		mFirstIns = ins;
		mOrderQty = volume;
		mOrderPrice = orderPrice;
		mFirstRemainQty = volume;
		submitOrder(FIRST_INSTRUMENT, ins, orderPrice, direction, volume, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, true);
	}
}

void USTPSpeculateOrder::submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	orderInsert(USTPMutexId::getMutexId(), insLabel, instrument, orderPrice, direction, qty, priceType, timeCondition, isFirstIns);	

}

void USTPSpeculateOrder::orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	double adjustPrice = (priceType == THOST_FTDC_OPT_LimitPrice) ? orderPrice : 0.0;
	QString orderRef;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_SUBMIT;
	USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
	mFirstOrderRef = orderRef;
	QString szReqId = QString::number(reqId);
	mReqMap.insert(orderRef, szReqId);
	emit onUpdateOrderShow(szReqId, instrument, mOrderLabel, 'N', direction, 0.0, qty, qty, 0, mOffsetFlag, priceType, mHedgeFlag, mOrderPrice);
#ifdef _DEBUG
	int nIsSecCancel = mIsAutoSecondCancel ? 1 : 0;
	int nIsSecIns = isFirstIns ? 0 : 1;
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderInsert]   Instrument: ")) + instrument +  QString(tr("  RequestId: ")) + szReqId + QString(tr("  OrderRef: ")) + orderRef + 
		QString(tr("  UserId: ")) + mUserId + QString(tr("  PriceType: ")) + QString(priceType) + QString(tr("  OrderPrice: ")) + QString::number(adjustPrice) + QString(tr("  OrderVolume: ")) + 
		QString::number(qty) + QString(tr("  Direction: ")) + QString(direction) + QString(tr("  SecAutoCancel: ")) + QString::number(nIsSecCancel) + QString(tr("  IsSecIns: ")) + QString::number(nIsSecIns);
	USTPLogger::saveData(data);
#endif	
}

void USTPSpeculateOrder::doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
											 const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
											 const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
	emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, orderStatus, direction, orderPrice, orderVolume, remainVolume, tradeVolume, offsetFlag, priceType, hedgeFlag, orderPrice);
	if(mFirstOrderRef == orderRef){
		mFirstInsStatus = orderStatus;
		mFirstRemainQty = remainVolume;
		if (THOST_FTDC_OST_Canceled == orderStatus){
			USTPMutexId::updateActionNum(instrumentId);
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, false, mIsActionReferTick,
				mOrderType, 0.0, 0.0, 0.0, 0.0);
			if(mTempOrderLabel == "")
				return;
			mOrderLabel = mTempOrderLabel;
			mFirstIns = mTempFirstIns;
			mOrderPrice = mTempOrderPrice;
			mBS = mTempBS;
			mOrderQty = mTempOrderQty;
			mOffsetFlag = mTempOffsetFlag;
			submitOrder(FIRST_INSTRUMENT, mFirstIns, mOrderPrice, mBS, mOrderQty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, true);
		}else if(THOST_FTDC_OST_AllTraded == orderStatus){
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, true, mIsActionReferTick,
				mOrderType, 0.0, 0.0, 0.0, 0.0);
			mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
		}
	}
}


void USTPSpeculateOrder::doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
											 const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
											 const QString& localId, const QString& orderRef, const QString& tradeTime)
{
}


void USTPSpeculateOrder::doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
													  const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
													  const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;

	if(mFirstIns == instrumentId)
		mFirstInsStatus = USTP_FTDC_OS_ORDER_ERROR;

	switch (errorId){
	case 22:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'D', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice); //重复的报单
		break;
	case 31:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'Z', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	资金不足
		break;
	case 42:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'S', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	结算结果未确认
		break;
	case 50:
	case 51:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'P', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//平仓位不足
		break;
	default:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'W', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);
		break;
	}
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderInsert] orderRef: ")) + orderRef + QString(tr("  RequestId: ")) + mReqMap[orderRef] + QString(tr("  InstrumentId: ")) + instrumentId + 
		QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPSpeculateOrder::doUSTPErrRtnOrderAction(const char& actionFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId,
													  const QString& orderSysId, const QString& userActionLocalId, const QString& orderActionRef, const double& orderPrice, 
													  const int& volumeChange, const int& errorId, const QString& errorMsg, const int& reqId)
{
#ifdef _DEBUG
	if(mReqMap.find(orderActionRef) == mReqMap.end())
		return;
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderAction] orderSysId: ")) + orderSysId + 
		QString(tr("  UserActionLocalId: ")) + userActionLocalId  + QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPSpeculateOrder::doDelOrder(const QString& orderStyle)
{
	if(orderStyle == mOrderLabel){
		if(isInMarket(mFirstInsStatus)){
			mTempOrderLabel = "";
			mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
			submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
		}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus || USTP_FTDC_OS_ORDER_ERROR == mFirstInsStatus || THOST_FTDC_OST_Canceled == mFirstInsStatus){
			emit onUpdateOrderShow(QString::number(mRequestId), mFirstIns, mOrderLabel, THOST_FTDC_OST_Canceled, mBS, 0.0, mOrderQty, mOrderQty, 0, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
			emit onOrderFinished(mOrderLabel, mFirstIns, mSecIns, mOrderPrice, mFirstSlipPoint, mSecSlipPoint, mOrderQty, mBS, mOffsetFlag, mHedgeFlag, mFirstCancelTime,
				mSecondCancelTime, mCycleStall, mActionReferNum, mActionSuperNum, 0.0, mIsAutoFirstCancel, mIsAutoSecondCancel, mIsCycle, false, false, false, mIsActionReferTick,
				mOrderType, 0.0, 0.0, 0.0, 0.0);
			mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
		}
		QString data = mOrderLabel + QString(tr("  [DoDelOrder]   mFirstInsStatus: ")) + QString(mFirstInsStatus);
		USTPLogger::saveData(data);
	}
}

void USTPSpeculateOrder::submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument)
{	
	USTPTradeApi::reqOrderAction(USTPMutexId::getMutexId(), mBrokerId, mUserId, mInvestorId, instrument, orderLocalId, mFrontId, mSessionId);
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderAction]   OrderRef: ")) + orderLocalId + QString(tr("  InstrumentId: ")) + instrument;
	USTPLogger::saveData(data);
#endif	
}


USTPUnilateralOrder::USTPUnilateralOrder(const QString& orderLabel, const QString& insName, const char& direction, const char& offsetFlag, 
										 const double& orderPrice, const int& orderQty, const int& cyclePoint, const int& cycleNum, const int& totalCycleNum, const double& insBidPrice,
										 const double& insAskPrice, bool isFirstIns, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, USTPUnilateralWidget* pUnilateralWidget) :
USTPStrategyBase(orderLabel, insName, "", orderPrice, orderQty, direction, offsetFlag, THOST_FTDC_HF_Speculation, 0, 0, 0, 0, 0, false, false, true)
{	
	moveToThread(&mStrategyThread);
	mStrategyThread.start();
	mFirstTradeQty = 0;
	mFirstRemainQty = mOrderQty;
	mSecondTradeQty = 0;
	mOrderType = 6;
	mRequestId = -1;
	mCyclePoint = cyclePoint;
	mCycleNum = cycleNum;
	mTotalCycleNum = totalCycleNum;
	mFirstBidMarketPrice = insBidPrice;
	mFirstAskMarketPrice = insAskPrice;
	mIsFirstIns = isFirstIns;	
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mBrokerId = USTPCtpLoader::getBrokerId();
	mUserId = USTPMutexId::getUserId();
	mInvestorId = USTPMutexId::getInvestorId();
	initConnect(pUnilateralWidget, pOrderWidget, pCancelWidget);	
	updateInitShow();
}

USTPUnilateralOrder::~USTPUnilateralOrder()
{
	mStrategyThread.quit();
	mStrategyThread.wait();
}

void USTPUnilateralOrder::initConnect(USTPUnilateralWidget* pUnilateralWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget)
{
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), 
		this, SLOT(doUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)),
		this, SLOT(doUSTPRtnOrder(const QString&, const QString&, const QString&, const char&, const double&, const int&, const int&, const int&, const char&, const char&, const char&, const char&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const char&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderInsert(const QString&, const QString&, const char&, const QString&, const char&, const QString&, const QString&, const char&, const char&, const char&,
		const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)),
		this, SLOT(doUSTPErrRtnOrderAction(const char&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const double&, const int&, const int&, const QString&, const int&)), Qt::QueuedConnection);

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)),
		this, SLOT(doUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pOrderWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pCancelWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const double& , const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(pCancelWidget, SIGNAL(onDelOrder(const QString& )), this, SLOT(doDelOrder(const QString& )), Qt::QueuedConnection);

	connect(this, SIGNAL(onUnilateralFinished(const QString&, const QString&, const char&, const char&, const double&, const int&, const double&, const double, const int&, const int&, const int&, bool, bool)), 
		pUnilateralWidget, SLOT(doUnilateralFinished(const QString&, const QString&, const char&, const char&, const double&, const int&, const double&, const double, const int&, const int&, const int&, bool, bool)),  Qt::QueuedConnection);

	connect(this, SIGNAL(onUnilateralFinished(const QString&, const QString&, const char&, const char&, const double&, const int&, const double&, const double, const int&, const int&, const int&, bool, bool)), pCancelWidget,
		SLOT(doUnilateralFinished(const QString&, const QString&, const char&, const char&, const double&, const int&, const double&, const double, const int&, const int&, const int&, bool, bool)), Qt::QueuedConnection);
}

void USTPUnilateralOrder::updateInitShow()
{	
	if(mIsDeleted)
		return;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	mRequestId = USTPMutexId::getMutexId();
	QString szReqId = QString::number(mRequestId);
	emit onUpdateOrderShow(szReqId, mFirstIns, mOrderLabel, 'N', mBS, 0.0, mOrderQty, mOrderQty, 0, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
	switchFirstInsOrder(THOST_FTDC_TC_GFD);
}

void USTPUnilateralOrder::doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
													   const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
													   const double& lowestPrice, const int& volume)
{	//记录合约买卖行情
	return;
//	if(mFirstIns != instrumentId)
//		return;
//	if (mFirstIns == instrumentId){
//		mFirstAskMarketPrice = askPrice;
//		mFirstBidMarketPrice = bidPrice;
//	}
//
//#ifdef _DEBUG	
//	QString data = QString(tr("[MarketDepth]  Instrument: ")) + instrumentId + tr("  BS: ") + QString(mBS) + ("  FirstBidMarketPrice: ") + QString::number(mFirstBidMarketPrice) +
//		tr("  FirstAskMarketPrice: ") + QString::number(mFirstAskMarketPrice) + tr("  IsFirst: ") + QString::number(mIsFirstIns ? 1 : 0);
//	USTPLogger::saveData(data);
//#endif
//
//	if(mIsFirstIns &&((mFirstAskMarketPrice < VALUE_ACCURACY) || (mFirstBidMarketPrice < VALUE_ACCURACY)))	//保证两腿都收到行情
//		return;
//	if((mFirstBidMarketPrice > INIT_VALUE) || (mFirstAskMarketPrice > INIT_VALUE))
//		return;
//	if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus){	//第一腿初次或者设定时间撤单后报单
//		if(mIsFirstIns)
//			firstInsAutoOrder();
//		else
//			secInsAutoOrder();
//	}else if(USTPStrategyBase::isInMarket(mFirstInsStatus)){	//报单存在队列中
//		if(mIsFirstIns)
//			firstInsAutoCancel(instrumentId);
//		else
//			secInsAutoCancel(instrumentId);
//	}
}

//void USTPUnilateralOrder::firstInsAutoOrder()
//{
//	if(mFirstAskMarketPrice >= mFirstBidMarketPrice + mOrderPrice){	
//		double referBidBasePrice = mReferBidMarketPrice + mReferPriceSpread;
//		double referAskBasePrice = mReferAskMarketPrice + mReferPriceSpread;
//		if (THOST_FTDC_D_Buy == mBS && (mFirstBidMarketPrice <= referBidBasePrice)){
//			mFirstOrderPrice = mFirstBidMarketPrice + mFirstSlipPrice;
//			if(mFirstOrderPrice > referBidBasePrice)
//				mFirstOrderPrice = referBidBasePrice;
//			switchFirstInsOrder(THOST_FTDC_TC_GFD);
//
//		}else if(THOST_FTDC_D_Sell == mBS && (mFirstAskMarketPrice >= referAskBasePrice)){
//			mFirstOrderPrice = mFirstAskMarketPrice - mFirstSlipPrice;
//			if(mFirstOrderPrice < referAskBasePrice)
//				mFirstOrderPrice = referAskBasePrice;
//			switchFirstInsOrder(THOST_FTDC_TC_GFD);
//		}
//	}else{
//		if(THOST_FTDC_OST_Canceled == mFirstInsStatus){
//			updateInitShow();
//		}
//	}
//}
//
//void USTPUnilateralOrder::secInsAutoOrder()
//{	
//	if(THOST_FTDC_D_Buy == mBS && (mFirstAskMarketPrice <= mBidOpponentBasePrice)){
//		mFirstOrderPrice = mFirstAskMarketPrice;
//		switchFirstInsOrder(THOST_FTDC_TC_GFD);
//	}else if(THOST_FTDC_D_Buy == mBS && (mFirstBidMarketPrice <= mBidOrderBasePrice)){
//		mFirstOrderPrice = mFirstBidMarketPrice + mFirstSlipPrice;
//		if(mFirstOrderPrice > mBidOrderBasePrice)
//			mFirstOrderPrice = mBidOrderBasePrice;
//		switchFirstInsOrder(THOST_FTDC_TC_GFD);
//	}else if(THOST_FTDC_D_Sell == mBS && (mFirstBidMarketPrice >= mAskOpponentBasePrice)){
//		mFirstOrderPrice = mFirstBidMarketPrice;
//		switchFirstInsOrder(THOST_FTDC_TC_GFD);
//	}else if(THOST_FTDC_D_Sell == mBS && (mFirstAskMarketPrice >= mAskOrderBasePrice)){
//		mFirstOrderPrice = mFirstAskMarketPrice - mFirstSlipPrice;
//		if(mFirstOrderPrice < mAskOrderBasePrice)
//			mFirstOrderPrice = mAskOrderBasePrice;
//		switchFirstInsOrder(THOST_FTDC_TC_GFD);
//	}else{
//		if(THOST_FTDC_OST_Canceled == mFirstInsStatus){
//			updateInitShow();
//		}
//	}
//}
//
//void USTPUnilateralOrder::firstInsAutoCancel(const QString& insName)
//{	
//	double referBidBasePrice = mReferBidMarketPrice + mReferPriceSpread;
//	double referAskBasePrice = mReferAskMarketPrice + mReferPriceSpread;
//	if((mFirstIns == insName) && ((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice > mFirstAskMarketPrice - mOrderPrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice < mFirstBidMarketPrice + mOrderPrice)))){
//		cancelFirstIns();
//	}else if((mFirstIns == insName) && ((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice < mFirstBidMarketPrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice > mFirstAskMarketPrice)))){
//		cancelFirstIns();
//	}else if(((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice > referBidBasePrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice < referAskBasePrice)))){
//		cancelFirstIns();
//	}
//}
//
//void USTPUnilateralOrder::secInsAutoCancel(const QString& insName)
//{
//	if((mFirstIns == insName) && ((THOST_FTDC_D_Buy == mBS && (mFirstAskMarketPrice <= mBidOrderBasePrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstBidMarketPrice >=  mAskOrderBasePrice)))){
//		cancelFirstIns();
//	}else if((mFirstIns == insName) && ((THOST_FTDC_D_Buy == mBS && (mFirstOrderPrice < mFirstBidMarketPrice)) || (THOST_FTDC_D_Sell == mBS && (mFirstOrderPrice > mFirstAskMarketPrice)))){
//		cancelFirstIns();
//	}
//}

void USTPUnilateralOrder::switchFirstInsOrder(const char& tCondition)
{
	if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus){
		if(mIsDeleted)
			return;
		orderInsert(mRequestId, FIRST_INSTRUMENT, mFirstIns, mOrderPrice, mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, tCondition, true);
	}else
		submitOrder(FIRST_INSTRUMENT, mFirstIns, mOrderPrice, mBS, mFirstRemainQty, THOST_FTDC_OPT_LimitPrice, tCondition, true);
}

void USTPUnilateralOrder::submitOrder(const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	if(isFirstIns && mIsDeleted)	//撤掉报单，合约一禁止下新单
		return;
	orderInsert(USTPMutexId::getMutexId(), insLabel, instrument, orderPrice, direction, qty, priceType, timeCondition, isFirstIns);	
}

void USTPUnilateralOrder::orderInsert(const int& reqId, const QString& insLabel, const QString& instrument, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition, bool isFirstIns)
{	
	double adjustPrice = (priceType == THOST_FTDC_OPT_LimitPrice) ? orderPrice : 0.0;
	QString orderRef;
	if(isFirstIns){
		mFirstInsStatus = USTP_FTDC_OS_ORDER_SUBMIT;
		USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrument, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, mHedgeFlag, THOST_FTDC_VC_AV);
		mFirstOrderRef = orderRef;
	}
	QString szReqId = QString::number(reqId);
	mReqMap.insert(orderRef, szReqId);
	emit onUpdateOrderShow(szReqId, instrument, mOrderLabel, 'N', direction, 0.0, qty, qty, 0, mOffsetFlag, priceType, mHedgeFlag, mOrderPrice);
	//条件日志
#ifdef _DEBUG
	QString data = mOrderLabel + tr("  [") + insLabel + tr("-OrderInsert]   Instrument: ") + instrument +  tr("  RequestId: ") + szReqId + tr("  OrderRef: ") + orderRef + 
		tr("  UserId: ") + mUserId + tr("  PriceType: ") + QString(priceType) + tr("  OrderPrice: ") + QString::number(adjustPrice) + tr("  OrderVolume: ") + 
		QString::number(qty) + tr("  Direction: ") + QString(direction);
	USTPLogger::saveData(data);
#endif	
}

void USTPUnilateralOrder::doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
											 const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
											 const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
	emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, orderStatus, direction, orderPrice, orderVolume, remainVolume, tradeVolume, offsetFlag, priceType, hedgeFlag, mOrderPrice);
	if(mFirstOrderRef == orderRef){
		mFirstInsStatus = orderStatus;
		mFirstRemainQty = remainVolume;
		if (THOST_FTDC_OST_Canceled == orderStatus){
			USTPMutexId::updateActionNum(instrumentId);
			if(mIsDeleted){
				emit onUnilateralFinished(mOrderLabel, mFirstIns, mBS, mOffsetFlag, mOrderPrice, mOrderQty, mFirstBidMarketPrice,
					mFirstAskMarketPrice, mCyclePoint, mCycleNum, mTotalCycleNum, mIsFirstIns, false);
			}
		}
	}
}


void USTPUnilateralOrder::doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
											 const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
											 const QString& localId, const QString& orderRef, const QString& tradeTime)
{
	if(mFirstOrderRef == orderRef){	
		mFirstTradeQty += tradeVolume;
	}
	if( mFirstTradeQty >= mOrderQty){
		emit onUnilateralFinished(mOrderLabel, mFirstIns, mBS, mOffsetFlag, mOrderPrice, mOrderQty, mFirstBidMarketPrice,
			mFirstAskMarketPrice, mCyclePoint, mCycleNum, mTotalCycleNum, mIsFirstIns, true);
	}
}


void USTPUnilateralOrder::doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
													  const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
													  const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;

	if(mFirstIns == instrumentId)	//设置合约状态
		mFirstInsStatus = USTP_FTDC_OS_ORDER_ERROR;

	switch (errorId){
	case 22:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'D', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice); //重复的报单
		break;
	case 31:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'Z', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	资金不足
		break;
	case 42:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'S', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	结算结果未确认
		break;
	case 50:
	case 51:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'P', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//平仓位不足
		break;
	default:
		emit onUpdateOrderShow(mReqMap[orderRef], instrumentId, mOrderLabel, 'W', direction, orderPrice, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);
		break;
	}
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderInsert] orderRef: ")) + orderRef + QString(tr("  RequestId: ")) + mReqMap[orderRef] + QString(tr("  InstrumentId: ")) + instrumentId + 
		QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPUnilateralOrder::doUSTPErrRtnOrderAction(const char& actionFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId,
													  const QString& orderSysId, const QString& userActionLocalId, const QString& orderActionRef, const double& orderPrice, 
													  const int& volumeChange, const int& errorId, const QString& errorMsg, const int& reqId)
{
#ifdef _DEBUG
	if(mReqMap.find(orderActionRef) == mReqMap.end())
		return;
	QString data = mOrderLabel + QString(tr("  [ErrRtnOrderAction] orderSysId: ")) + orderSysId + 
		QString(tr("  UserActionLocalId: ")) + userActionLocalId  + QString(tr("  ErrorId: ")) + QString::number(errorId) + QString(tr("  ErrorMsg: ")) + errorMsg;
	USTPLogger::saveData(data);
#endif
}
//
//void USTPUnilateralOrder::doAutoCancelFirstIns()
//{	
//	if(isInMarket(mFirstInsStatus)){
//		mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
//		submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
//	}
//}


void USTPUnilateralOrder::doDelOrder(const QString& orderStyle)
{
	if(orderStyle == mOrderLabel){
		mIsDeleted = true;
		if(isInMarket(mFirstInsStatus)){
			mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
			submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
		}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus || USTP_FTDC_OS_ORDER_ERROR == mFirstInsStatus || THOST_FTDC_OST_Canceled == mFirstInsStatus){
			emit onUpdateOrderShow(QString::number(mRequestId), mFirstIns, mOrderLabel, THOST_FTDC_OST_Canceled, mBS, 0.0, mOrderQty, mOrderQty, 0, mOffsetFlag, THOST_FTDC_OPT_LimitPrice, mHedgeFlag, mOrderPrice);
			emit onUnilateralFinished(mOrderLabel, mFirstIns, mBS, mOffsetFlag, mOrderPrice, mOrderQty, mFirstBidMarketPrice,
				mFirstAskMarketPrice, mCyclePoint, mCycleNum, mTotalCycleNum, mIsFirstIns, false);
		}
		QString data = mOrderLabel + QString(tr("  [DoDelOrder]   mFirstInsStatus: ")) + QString(mFirstInsStatus);
		USTPLogger::saveData(data);
	}
}
//
//void USTPUnilateralOrder::cancelFirstIns()
//{
//	mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
//	submitAction(FIRST_INSTRUMENT, mFirstOrderRef, mFirstIns);
//}

void USTPUnilateralOrder::submitAction(const QString& insLabel, const QString& orderLocalId, const QString& instrument)
{	
	USTPTradeApi::reqOrderAction(USTPMutexId::getMutexId(), mBrokerId, mUserId, mInvestorId, instrument, orderLocalId, mFrontId, mSessionId);
#ifdef _DEBUG
	QString data = mOrderLabel + QString(tr("  [")) + insLabel + QString(tr("-OrderAction]   OrderRef: ")) + orderLocalId + QString(tr("  InstrumentId: ")) + instrument;
	USTPLogger::saveData(data);
#endif	
}


#include "moc_USTPUserStrategy.cpp"