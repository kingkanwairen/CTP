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
#include "USTPSubmitWidget.h"

#define USTP_FTDC_OS_ORDER_SUBMIT 'O'
#define USTP_FTDC_OS_CANCEL_SUBMIT 'C'
#define USTP_FTDC_OS_ORDER_NO_ORDER 'N'
#define USTP_FTDC_OS_ORDER_ERROR 'E'


USTPStrategyBase::USTPStrategyBase(const QString& orderLabel, const QString& instrumentId, const double& orderPriceTick, const int& qty, const char& bs, const char& offset, const int& openSuperPoint,
								   const int& stopProfitPoint, const int& stopLossPoint)
{	
	
	mOrderLabel = orderLabel;
	mOrderPrice = orderPriceTick;
	mOrderQty = qty;
	mRemainQty = qty;
	mOffsetFlag = offset;
	mBS = bs;
	mInstrumentId = instrumentId;
	mFrontId = USTPMutexId::getFrontId();
	mSessionId = USTPMutexId::getSessionId();
	mBrokerId = USTPCtpLoader::getBrokerId();
	mUserId = USTPMutexId::getUserId();
	mInvestorId = USTPMutexId::getInvestorId();
	mExh = USTPMutexId::getInsExchangeId(instrumentId);
	mPriceTick = USTPMutexId::getInsPriceTick(instrumentId);
	mInsUpperPrice = USTPMutexId::getUpperPrice(instrumentId);
	mInsLowerPrice = USTPMutexId::getLowerPrice(instrumentId);
	mInsPrecision = getInsPrcision(mPriceTick);
	mOpenTick = mPriceTick * openSuperPoint;
	mStopProfitTick = mPriceTick * stopProfitPoint;
	mStopLossTick = mPriceTick * stopLossPoint;
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



USTPSpeculateOrder::USTPSpeculateOrder(const QString& orderLabel, const QString& speOrderLabel, const QString& instrumentId, const double& orderPriceTick, const int& qty, const char& bs, const char& offset,
									   const int& openSuperPoint, const int& stopProfitPoint, const int& stopLossPoint, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, USTPSubmitWidget* pSubmitWidget)
												 :USTPStrategyBase(orderLabel, instrumentId, orderPriceTick, qty, bs, offset, openSuperPoint, stopProfitPoint, stopLossPoint)
{		
	moveToThread(&mStrategyThread);
	mStrategyThread.start();
	mTempOrderQty = 0;
	mTempOrderPrice = 0.0;
	mSpeOrderLabel = speOrderLabel;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
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

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pOrderWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pCancelWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(pCancelWidget, SIGNAL(onDelOrder(const QString& )), this, SLOT(doDelOrder(const QString& )), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&)), pCancelWidget, SLOT(doOrderFinished(const QString&, const QString&)), Qt::QueuedConnection);
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
		submitAction(mFirstOrderRef, mInstrumentId);

	}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus || THOST_FTDC_OST_Canceled == mFirstInsStatus){
		mOrderLabel = orderLabel;
		mOffsetFlag = offsetFlag;
		mBS = direction;
		mInstrumentId = ins;
		mOrderQty = volume;
		mOrderPrice = orderPrice;
		mRemainQty = volume;
		submitOrder(ins, orderPrice, direction, volume, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD);
	}
}

void USTPSpeculateOrder::submitOrder(const QString& instrumentId, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition)
{	
	orderInsert(USTPMutexId::getMutexId(), instrumentId, orderPrice, direction, qty, priceType, timeCondition);	

}

void USTPSpeculateOrder::orderInsert(const int& reqId, const QString& instrumentId, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition)
{	
	double adjustPrice = (priceType == THOST_FTDC_OPT_LimitPrice) ? orderPrice : 0.0;
	QString orderRef;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_SUBMIT;
	USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrumentId, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, THOST_FTDC_HF_Speculation, THOST_FTDC_VC_AV);
	mFirstOrderRef = orderRef;
	QString szReqId = QString::number(reqId);
	mReqMap.insert(orderRef, szReqId);
	emit onUpdateOrderShow(mOrderLabel, szReqId, instrumentId, 'N', direction, qty, qty, 0, mOffsetFlag, priceType, THOST_FTDC_HF_Speculation, mOrderPrice);
#ifdef _DEBUG
	QString data = mOrderLabel + tr("  [OrderInsert] Instrument: ") + instrumentId +  tr("  RequestId: ") + szReqId + tr("  OrderRef: ") + orderRef + 
		tr("  UserId: ") + mUserId + tr("  PriceType: ") + QString(priceType) + tr("  OrderPrice: ") + QString::number(adjustPrice) + tr("  OrderVolume: ") + 
		QString::number(qty) + tr("  Direction: ") + QString(direction) + tr("  OffsetFlag: ") + QString(mOffsetFlag);
	USTPLogger::saveData(data);
#endif	
}

void USTPSpeculateOrder::doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
											 const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
											 const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
	emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, orderStatus, direction, orderVolume, remainVolume, tradeVolume, offsetFlag, priceType, hedgeFlag, orderPrice);
	if(mFirstOrderRef == orderRef){
		mFirstInsStatus = orderStatus;
		mRemainQty = remainVolume;
		if (THOST_FTDC_OST_Canceled == orderStatus){
			USTPMutexId::updateActionNum(instrumentId);
			emit onOrderFinished(mOrderLabel, mInstrumentId);
			if(mTempOrderLabel == "")
				return;
			mOrderLabel = mTempOrderLabel;
			mInstrumentId = mTempFirstIns;
			mOrderPrice = mTempOrderPrice;
			mBS = mTempBS;
			mOrderQty = mTempOrderQty;
			mOffsetFlag = mTempOffsetFlag;
			submitOrder(mInstrumentId, mOrderPrice, mBS, mRemainQty, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD);
		}else if(THOST_FTDC_OST_AllTraded == orderStatus){
			emit onOrderFinished(mOrderLabel, mInstrumentId);
			mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
		}
	}
}


void USTPSpeculateOrder::doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
													  const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
													  const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;

	if(mInstrumentId == instrumentId)
		mFirstInsStatus = USTP_FTDC_OS_ORDER_ERROR;

	switch (errorId){
	case 22:
		emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, 'D', direction, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice); //重复的报单
		break;
	case 31:
		emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, 'Z', direction, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	资金不足
		break;
	case 42:
		emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, 'S', direction, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	结算结果未确认
		break;
	case 50:
	case 51:
		emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, 'P', direction, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//平仓位不足
		break;
	default:
		emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, 'W', direction, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);
		break;
	}
#ifdef _DEBUG
	QString data = mOrderLabel + tr("  [ErrRtnOrderInsert] orderRef: ") + orderRef + tr("  RequestId: ") + mReqMap[orderRef] + tr("  InstrumentId: ") + instrumentId + 
		tr("  ErrorId: ") + QString::number(errorId) + tr("  ErrorMsg: ") + errorMsg;
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
	QString data = mOrderLabel + tr("  [ErrRtnOrderAction] orderSysId: ") + orderSysId + tr("  UserActionLocalId: ") + userActionLocalId  + tr("  ErrorId: ") + 
		QString::number(errorId) + tr("  ErrorMsg: ") + errorMsg;
	USTPLogger::saveData(data);
#endif
}

void USTPSpeculateOrder::doDelOrder(const QString& orderLabel)
{
	if(orderLabel == mOrderLabel){
		if(isInMarket(mFirstInsStatus)){
			mTempOrderLabel = "";
			mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
			submitAction(mFirstOrderRef, mInstrumentId);
		}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus || USTP_FTDC_OS_ORDER_ERROR == mFirstInsStatus || THOST_FTDC_OST_Canceled == mFirstInsStatus){
			emit onUpdateOrderShow(mOrderLabel, mReqMap[mFirstOrderRef], mInstrumentId, THOST_FTDC_OST_Canceled, mBS, mOrderQty, mOrderQty, 0, mOffsetFlag, 
				THOST_FTDC_OPT_LimitPrice, THOST_FTDC_HF_Speculation, mOrderPrice);
			emit onOrderFinished(mOrderLabel, mInstrumentId);
			mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
		}
		QString data = mOrderLabel + tr("  [DoDelOrder] mFirstInsStatus: ") + QString(mFirstInsStatus);
		USTPLogger::saveData(data);
	}
}

void USTPSpeculateOrder::submitAction(const QString& orderRef, const QString& instrumentId)
{	
	USTPTradeApi::reqOrderAction(USTPMutexId::getMutexId(), mBrokerId, mUserId, mInvestorId, instrumentId, orderRef, mFrontId, mSessionId);
#ifdef _DEBUG
	QString data = mOrderLabel + tr("  [OrderAction] OrderRef: ") + orderRef + tr("  InstrumentId: ") + instrumentId;
	USTPLogger::saveData(data);
#endif	
}


USTPUnilateralOrder::USTPUnilateralOrder(const QString& orderLabel, const QString& instrumentId, const double& orderPriceTick, const int& qty, const char& bs, const char& offset, const int& openSuperPoint,
										 const int& stopProfitPoint, const int& stopLossPoint, const double& bidPrice, const double& askPrice, const double& lastPrice, USTPOrderWidget* pOrderWidget,
										 USTPCancelWidget* pCancelWidget, USTPSubmitWidget* pSubmitWidget) : USTPStrategyBase(orderLabel, instrumentId, orderPriceTick, qty, bs, offset, openSuperPoint,
										 stopProfitPoint, stopLossPoint)
{	
	moveToThread(&mStrategyThread);
	mStrategyThread.start();
	mIsDeleted = false;
	if(THOST_FTDC_D_Buy == bs){
		mOpenOrderPrice = mOrderPrice - mOpenTick;
		mStopProfitPrice = mOpenOrderPrice + mStopProfitTick;
		mStopLossPrice = mOpenOrderPrice - mStopLossTick;
	}else{
		mOpenOrderPrice = mOrderPrice + mOpenTick;
		mStopProfitPrice = mOpenOrderPrice - mStopProfitTick;
		mStopLossPrice = mOpenOrderPrice + mStopLossTick;
	}
	mSubmitStatus = 'O';
	mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
	initConnect(pSubmitWidget, pOrderWidget, pCancelWidget);	
	switchFirstInsOrder(mOpenOrderPrice, THOST_FTDC_TC_GFD);
}

USTPUnilateralOrder::~USTPUnilateralOrder()
{
	mStrategyThread.quit();
	mStrategyThread.wait();
}

void USTPUnilateralOrder::initConnect(USTPSubmitWidget* pSubmitWidget, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget)
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
	
	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pOrderWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const int&, const int&, const int&, const char&, const char&, const char&, const double&)), 
		pCancelWidget, SLOT(doUpdateOrderShow(const QString&, const QString&, const QString&, const char&, const char&, const int&, const int&, const int&, const char&, const char&, const char&, const double&)), Qt::QueuedConnection);
	
	connect(pCancelWidget, SIGNAL(onDelOrder(const QString& )), this, SLOT(doDelOrder(const QString& )), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&)), pSubmitWidget, SLOT(doOrderFinished(const QString&, const QString&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onOrderFinished(const QString&, const QString&)), pCancelWidget, SLOT(doOrderFinished(const QString&, const QString&)), Qt::QueuedConnection);
}


void USTPUnilateralOrder::doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
													   const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
													   const double& lowestPrice, const int& volume)
{	//记录合约买卖行情
	if(mInstrumentId != instrumentId)
		return;
	mAskMarketPrice = askPrice;
	mBidMarketPrice = bidPrice;

	if((mAskMarketPrice < VALUE_ACCURACY) || (mBidMarketPrice < VALUE_ACCURACY))	//保证两腿都收到行情
		return;
	if(('P' == mSubmitStatus) && USTPStrategyBase::isInMarket(mFirstInsStatus)){	//报单存在队列中
		double bidStopLoss1 = mAskMarketPrice - mStopLossPrice;
		double bidStopLoss2 = mAskMarketPrice - mOrderBasePrice;
		double askStopLoss1 = mStopLossPrice - mBidMarketPrice;
		double askStopLoss2 = mOrderBasePrice - mBidMarketPrice;
#ifdef _DEBUG	
		QString data = tr("[MarketDepth]  Instrument: ") + instrumentId + tr("  BS: ") + QString(mBS) + tr("  BidMarketPrice: ") + QString::number(mBidMarketPrice) +
			tr("  AskMarketPrice: ") + QString::number(mAskMarketPrice) + tr("  BidStopLoss1: ") + QString::number(bidStopLoss1) + tr("  BidStopLoss2: ") +
			QString::number(bidStopLoss2) + tr("  AskStopLoss1: ") + QString::number(askStopLoss1) + tr("  AskStopLoss2: ") + QString::number(askStopLoss2);
		USTPLogger::saveData(data);
#endif
		if(((THOST_FTDC_D_Buy == mBS) && (bidStopLoss1 > VERSE_VALUE_ACCURACY) && (bidStopLoss2 > VALUE_ACCURACY)) ||
			((THOST_FTDC_D_Sell == mBS) && (askStopLoss1 > VERSE_VALUE_ACCURACY) && (askStopLoss2 > VALUE_ACCURACY)))
			cancelFirstIns();
	}
}

void USTPUnilateralOrder::switchFirstInsOrder(const double& price, const char& tCondition)
{
	submitOrder(mInstrumentId, price, mBS, mRemainQty, THOST_FTDC_OPT_LimitPrice, tCondition);
		
}

void USTPUnilateralOrder::submitOrder(const QString& instrumentId, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition)
{	
	if(mIsDeleted)
		return;
	orderInsert(USTPMutexId::getMutexId(), instrumentId, orderPrice, direction, qty, priceType, timeCondition);	
}

void USTPUnilateralOrder::orderInsert(const int& reqId, const QString& instrumentId, const double& orderPrice, const char& direction, const int& qty, const char& priceType, const char& timeCondition)
{	
	double adjustPrice = (priceType == THOST_FTDC_OPT_LimitPrice) ? orderPrice : 0.0;
	QString orderRef;
	mFirstInsStatus = USTP_FTDC_OS_ORDER_SUBMIT;
	USTPTradeApi::reqOrderInsert(reqId, orderRef, mBrokerId, mUserId, mInvestorId, instrumentId, priceType, timeCondition, adjustPrice, qty, direction, mOffsetFlag, THOST_FTDC_HF_Speculation, THOST_FTDC_VC_AV);
	mFirstOrderRef = orderRef;
	mOrderBasePrice = adjustPrice;
	QString szReqId = QString::number(reqId);
	mReqMap.insert(orderRef, szReqId);
	emit onUpdateOrderShow(mOrderLabel, szReqId, instrumentId, 'N', direction, qty, qty, 0, mOffsetFlag, priceType, THOST_FTDC_HF_Speculation, orderPrice);
#ifdef _DEBUG
	QString data = mOrderLabel + tr("  [OrderInsert] Instrument: ") + instrumentId +  tr("  RequestId: ") + szReqId + tr("  OrderRef: ") + orderRef + 
		tr("  UserId: ") + mUserId + tr("  PriceType: ") + QString(priceType) + tr("  OrderPrice: ") + QString::number(adjustPrice) + tr("  OrderVolume: ") + 
		QString::number(qty) + tr("  Direction: ") + QString(direction) + tr("  OffsetFlag: ") + QString(mOffsetFlag);
	USTPLogger::saveData(data);
#endif	
}

void USTPUnilateralOrder::doUSTPRtnOrder(const QString& localId, const QString& orderRef, const QString& instrumentId, const char& direction, const double& orderPrice, const int& orderVolume,
											 const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedgeFlag, const char& orderStatus,
											 const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId, const QString& statusMsg, const char& timeCondition, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;
	emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, orderStatus, direction, orderVolume, remainVolume, tradeVolume, offsetFlag, priceType, hedgeFlag, orderPrice);
	if(mFirstOrderRef == orderRef){
		mFirstInsStatus = orderStatus;
		mRemainQty = remainVolume;
		if (THOST_FTDC_OST_Canceled == orderStatus){
			USTPMutexId::updateActionNum(instrumentId);
			if(mIsDeleted){
				emit onOrderFinished(mOrderLabel, instrumentId);
			}else{
				if('P' == mSubmitStatus){
					if(THOST_FTDC_D_Buy == direction)
						switchFirstInsOrder(mAskMarketPrice, THOST_FTDC_TC_GFD);
					else
						switchFirstInsOrder(mBidMarketPrice, THOST_FTDC_TC_GFD);
				}
			}
		}else if (THOST_FTDC_OST_AllTraded == orderStatus){
			if('O' == mSubmitStatus){
				mSubmitStatus = 'P';
				mFirstInsStatus = USTP_FTDC_OS_ORDER_NO_ORDER;
				if(THOST_FTDC_D_Buy == mBS)
					mBS = THOST_FTDC_D_Sell;
				else
					mBS = THOST_FTDC_D_Buy;
				mOffsetFlag = THOST_FTDC_OF_CloseToday;
				mRemainQty = mOrderQty;
				switchFirstInsOrder(mStopProfitPrice, THOST_FTDC_TC_GFD);
			}else if('P' == mSubmitStatus){
				emit onOrderFinished(mOrderLabel, instrumentId);
			}
		}
	}
}


void USTPUnilateralOrder::doUSTPErrRtnOrderInsert(const QString& userId, const QString& brokerId, const char& direction, const QString& gtdDate, const char& hedgeFlag,
													  const QString& instrumentId, const QString& investorId, const char& offsetFlag, const char& priceType, const char& timeCondition,
													  const QString& orderRef, const double& orderPrice, const int& volume, const int& errorId, const QString& errorMsg, const int& reqId)
{	
	if(mReqMap.find(orderRef) == mReqMap.end())
		return;

	if(mInstrumentId == instrumentId)
		mFirstInsStatus = USTP_FTDC_OS_ORDER_ERROR;

	switch (errorId){
	case 22:
		emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, 'D', direction, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice); //重复的报单
		break;
	case 31:
		emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, 'Z', direction, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	资金不足
		break;
	case 42:
		emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, 'S', direction, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//	结算结果未确认
		break;
	case 50:
	case 51:
		emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, 'P', direction, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);	//平仓位不足
		break;
	default:
		emit onUpdateOrderShow(mOrderLabel, mReqMap[orderRef], instrumentId, 'W', direction, volume, volume, 0, offsetFlag, priceType, hedgeFlag, mOrderPrice);
		break;
	}
#ifdef _DEBUG
	QString data = mOrderLabel + tr("  [ErrRtnOrderInsert] orderRef: ") + orderRef + tr("  RequestId: ") + mReqMap[orderRef] + tr("  InstrumentId: ") + instrumentId + 
		tr("  ErrorId: ") + QString::number(errorId) + tr("  ErrorMsg: ") + errorMsg;
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
	QString data = mOrderLabel + tr("  [ErrRtnOrderAction] orderSysId: ") + orderSysId + tr("  UserActionLocalId: ") + userActionLocalId  + tr("  ErrorId: ") + 
		QString::number(errorId) + tr("  ErrorMsg: ") + errorMsg;
	USTPLogger::saveData(data);
#endif
}


void USTPUnilateralOrder::doDelOrder(const QString& orderLabel)
{
	if(orderLabel == mOrderLabel){
		mIsDeleted = true;
		if(isInMarket(mFirstInsStatus)){
			mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
			submitAction(mFirstOrderRef, mInstrumentId);
		}else if(USTP_FTDC_OS_ORDER_NO_ORDER == mFirstInsStatus || USTP_FTDC_OS_ORDER_ERROR == mFirstInsStatus || THOST_FTDC_OST_Canceled == mFirstInsStatus){
			emit onUpdateOrderShow(mOrderLabel, mReqMap[mFirstOrderRef], mInstrumentId, THOST_FTDC_OST_Canceled, mBS, mOrderQty, mOrderQty, 0, mOffsetFlag, 
				THOST_FTDC_OPT_LimitPrice, THOST_FTDC_HF_Speculation, mOpenOrderPrice);
			emit onOrderFinished(mOrderLabel, mInstrumentId);
		}
		QString data = mOrderLabel + QString(tr("  [DoDelOrder]   mFirstInsStatus: ")) + QString(mFirstInsStatus);
		USTPLogger::saveData(data);
	}
}

void USTPUnilateralOrder::cancelFirstIns()
{
	mFirstInsStatus = USTP_FTDC_OS_CANCEL_SUBMIT;
	submitAction(mFirstOrderRef, mInstrumentId);
}

void USTPUnilateralOrder::submitAction(const QString& orderRef, const QString& instrumentId)
{	
	USTPTradeApi::reqOrderAction(USTPMutexId::getMutexId(), mBrokerId, mUserId, mInvestorId, instrumentId, orderRef, mFrontId, mSessionId);
#ifdef _DEBUG
	QString data = mOrderLabel + tr("  [OrderAction] OrderRef: ") + orderRef + tr("  InstrumentId: ") + instrumentId;
	USTPLogger::saveData(data);
#endif	
}


#include "moc_USTPUserStrategy.cpp"